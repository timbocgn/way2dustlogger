/*
	--------------------------------------------------------------------------------

    ESPDustLogger       
    
    ESP32 based IoT Device for air quality logging featuring an MQTT client and 
    REST API acess. Works in conjunction with a VINDRIKTNING air sensor from IKEA.
    

	This code is based on Bertrik Sikken PM1006 class available at
	https://github.com/bertrik/pm1006

    --------------------------------------------------------------------------------

    Copyright (c) 2021 Tim Hagemann / way2.net Services

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    --------------------------------------------------------------------------------
*/

////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "vindriktning.h"

////////////////////////////////////////////////////////////////////////////////////////

#define BUF_SIZE (1024)
#define STACK_SIZE (2048)
#define DATAGRAM_LEN 30

////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "vindriktning";

////////////////////////////////////////////////////////////////////////////////////////

// --- this is a debug stub "sending" a valid message for test purposes

uint8_t g_TestMessage[] = { 0x16, 0x11, 0x0b, 0x00,0x00,0x03,0xe8,0x00,0x00,0x07,0xd0,0x00,0x00,0x0B,0xb8,0x00,0x00,0x00,0x00,0x49};

int stub_uart_read_bytes(uart_port_t uart_num, void* buf, uint32_t length, TickType_t ticks_to_wait)
{
	assert(length >= sizeof(g_TestMessage));

	memcpy(buf,g_TestMessage,sizeof(g_TestMessage));

	return sizeof(g_TestMessage);
}

////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    PM1006_HEADER,
    PM1006_LENGTH,
    PM1006_DATA,
    PM1006_CHECK
} pm1006_state_t;

////////////////////////////////////////////////////////////////////////////////////////

class CReceiver
{

public:

	CReceiver(void)
	{
		_state 		= PM1006_HEADER;
		_rxlen 		= 0;
		_index 		= 0;
		_checksum 	= 0;		

		memset(_rxbuf, 0, sizeof(_rxbuf));
	}

	// --------------------------------------------

	void dump(void)
	{
		char l_s[100];

		strcpy(l_s,"Buffer:");

		for (int i=0;i<_rxlen;++i)
		{
			char l_buf[5];
			snprintf(l_buf,5,"%0x ",_rxbuf[i]);
			strcat(l_s,l_buf);
		}
		ESP_LOGI(TAG, "%s",l_s); 

	}

	// --------------------------------------------

	bool process_rx(uint8_t c)
	{
		//ESP_LOGI(TAG, "process_rx(%x): IN state %d checksum %d _rxlen %d index %d",c,_state,_checksum,_rxlen,_index);

		switch (_state) {
		case PM1006_HEADER:
			_checksum = c;
			if (c == 0x16) {
				_state = PM1006_LENGTH;
			}
			break;

		case PM1006_LENGTH:
			_checksum += c;
			if (c <= sizeof(_rxbuf)) {
				_rxlen = c;
				_index = 0;
				_state = (_rxlen > 0) ? PM1006_DATA : PM1006_CHECK;
			} 
			else 
			{
				ESP_LOGE(TAG, "process_rx(%x): message too long for buffer!",c);
				_state = PM1006_HEADER;
			}
			break;

		case PM1006_DATA:
			_checksum += c;
			_rxbuf[_index++] = c;
			if (_index == _rxlen) {
				_state = PM1006_CHECK;
			}
			break;

		case PM1006_CHECK:
			_checksum += c;
			_state = PM1006_HEADER;
	
			if (_checksum) ESP_LOGE(TAG, "process_rx(%x): checksum error. Expected 0, got %x",c,_checksum);

			//ESP_LOGI(TAG, "process_rx(%x): OUT state %d checksum %d _rxlen %d index %d",c,_state,_checksum,_rxlen,_index);

			return (_checksum == 0);

		default:
			_state = PM1006_HEADER;
			break;
		}

		//ESP_LOGI(TAG, "process_rx(%x): OUT state %d checksum %d _rxlen %d index %d",c,_state,_checksum,_rxlen,_index);

		return false;
	}

	// --------------------------------------------

	uint8_t GetByte(int f_idx)
	{
		assert(f_idx < DATAGRAM_LEN);
		
		return _rxbuf[f_idx];
	}

	// --------------------------------------------

    pm1006_state_t 	_state;
    size_t 			_rxlen;
    size_t 			_index;
    uint8_t 		_rxbuf[DATAGRAM_LEN];
    uint8_t 		_checksum;
 
};

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

CVindriktning::CVindriktning(void)
{
	m_Initialized		= false;

	m_pin_data			= (gpio_num_t)0;
	m_uart 				= (uart_port_t)0;
	
	m_pm1				= 0;
	m_pm2				= 0;
	m_pm10				= 0;
}

////////////////////////////////////////////////////////////////////////////////////////

static void uart_task(void *arg)
{
	CVindriktning 	*l_this = (CVindriktning *)arg;
	
	// ---- local instance of our shifter for the datagram

	CReceiver l_receiver;

	// ---- tell the monitor where we are 

	ESP_LOGI(TAG,"UART read task started for uart %d on GPIO pin %d", l_this->GetUart(), l_this->GetDataPin());

    // --- Configure a temporary buffer for the incoming data

    uint8_t *l_data = (uint8_t *) malloc(BUF_SIZE);
	assert(l_data);

	// --- never ending loop 

	while (1) 
	{
        // --- Read data from the UART. This might be one or more bytes in the middle of a datagram

        int len = uart_read_bytes(l_this->GetUart(), l_data, BUF_SIZE, 20 / portTICK_RATE_MS);

		// --- add them to the shifter

		for (int i=0;i < len;++i)
		{
			//ESP_LOGI(TAG, "received byte %x",l_data[i]);

			if (l_receiver.process_rx(l_data[i]))
			{
					// --- header and length are not stored in the receiver. We start with "0x0B" --> idx0, DF1 --> idx 1

					const uint16_t pm25 = (l_receiver.GetByte(3) << 8) | l_receiver.GetByte(4);
					const uint16_t pm1  = (l_receiver.GetByte(7) << 8) | l_receiver.GetByte(8);
					const uint16_t pm10 = (l_receiver.GetByte(11) << 8) | l_receiver.GetByte(12);

					ESP_LOGI(TAG, "Datagram valid pm25 %d pm1 %d pm10 %d",pm25,pm1,pm10);

					//l_receiver.dump();

					l_this->SetValues(pm25,pm1,pm10);
			}
		}

		vTaskDelay(3000 / portTICK_PERIOD_MS);

    }
}


////////////////////////////////////////////////////////////////////////////////////////

bool CVindriktning::SetupSensor(gpio_num_t f_data,uart_port_t f_uart)
{
	m_pin_data	= f_data;
	m_uart		= f_uart;

	ESP_LOGI(TAG,"Setting up sensor on uart %d on GPIO pin %d", GetUart(), GetDataPin());

    // --- Configure parameters of an UART driver, communication pins and install the driver 

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(m_uart, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(m_uart, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(m_uart, UART_PIN_NO_CHANGE, m_pin_data, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

	// --- now start a free rtos task to receive the sensor data

    xTaskCreate(uart_task, "CVindriktning__uart_task", STACK_SIZE, this, 10, NULL);

	m_Initialized = true;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

bool CVindriktning::PerformMeasurement(void)
{
	// ---- do nothing here. Values are populated asynchronously 

	return true;
}