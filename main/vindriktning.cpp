/*
	--------------------------------------------------------------------------------

    ESPDustLogger       
    
    ESP32 based IoT Device for air quality logging featuring an MQTT client and 
    REST API acess. Works in conjunction with a VINDRIKTNING air sensor from IKEA.
    
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
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "vindriktning.h"

/*

////////////////////////////////////////////////////////////////////////////////////////

// some code defines

// ---- HGT: chaged to 50 us (from 2) as our line length is too huge (5m STP)

#define SHT1x_DELAY ets_delay_us(50);

// ---- Macros to toggle port state of SCK line

#define SHT1x_SCK_LO	gpio_set_level(m_SHT1x_pin_sck, 0)

#define SHT1x_SCK_HI	gpio_set_level(m_SHT1x_pin_sck, 1)

#define SHT1x_DATA_LO 	gpio_set_level(m_SHT1x_pin_data, 0);\
						gpio_set_direction(m_SHT1x_pin_data,GPIO_MODE_OUTPUT)

#define	SHT1x_DATA_HI 	gpio_set_direction(m_SHT1x_pin_data,GPIO_MODE_INPUT)

#define SHT1x_GET_BIT 	gpio_get_level(m_SHT1x_pin_data)

*/
////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "ESP32_vindriktning";

////////////////////////////////////////////////////////////////////////////////////////

CVindriktning::CVindriktning(void)
{
	m_Initialized		= false;

	m_pin_data	= (gpio_num_t)0;
	
	m_pm1				= 0;
	m_pm2				= 0;
	m_pm10				= 0;
}

////////////////////////////////////////////////////////////////////////////////////////

/*
bool CVindriktning::SHT1x_InitPins(void) 
{
		
	esp_err_t l_err;
	// --- Reset gpios to default state (select gpio function, enable pullup and disable input and output).

	gpio_reset_pin(m_SHT1x_pin_sck);
	gpio_reset_pin(m_SHT1x_pin_data);

	// --- SCK line as output but set to low first

	l_err = gpio_set_level(m_SHT1x_pin_sck, 0);
	if (l_err != ESP_OK) { ESP_LOGE(TAG,"SHT1x_InitPins error on gpio_set_level for SCK"); return false; }

	l_err = gpio_set_direction(m_SHT1x_pin_sck,GPIO_MODE_OUTPUT);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "SHT1x_InitPins error on gpio_set_direction for SCK"); return false; }

	l_err = gpio_set_level(m_SHT1x_pin_sck, 0);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "SHT1x_InitPins error on gpio_set_level for SCK"); return false; }

	// --- how it works for SHT1x_pin_data: set PORT to 0 => pull data line low by setting port as output

	// --- floating means - no pullup and no pulldown - so we need the one on the breakout board 

	l_err = gpio_set_pull_mode(m_SHT1x_pin_data,GPIO_FLOATING);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "SHT1x_InitPins error on gpio_set_pull_mode for DATA"); return false; }

	// --- configure data output and set to low

	l_err = gpio_set_direction(m_SHT1x_pin_data,GPIO_MODE_OUTPUT);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "SHT1x_InitPins error on gpio_set_direction for DATA"); return false; }

	l_err = gpio_set_level(m_SHT1x_pin_sck, 0);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "SHT1x_InitPins error on gpio_set_level for DATA"); return false; }

	m_Initialized	= true;

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////

bool SHT1x::SHT1x_Reset(void) 
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

void SHT1x::SHT1x_Transmission_Start(void) 
{
	assert(m_Initialized);

}

////////////////////////////////////////////////////////////////////////////////////////

unsigned char SHT1x::SHT1x_Readbyte(bool send_ack) 
{
	unsigned char mask;
	unsigned char value = 0;

	// SCK is low here !
	for(mask=0x80; mask; mask >>= 1 )
	{
		SHT1x_SCK_HI;	SHT1x_DELAY;  	// SCK hi
		if( SHT1x_GET_BIT != 0 )  	// and read data
			value |= mask;

		SHT1x_SCK_LO;	SHT1x_DELAY; // SCK lo => sensor puts new data
	}

	// send ACK if required 
	if ( send_ack )
	{
		SHT1x_DATA_LO;	SHT1x_DELAY; // Get DATA line
	}
	
	SHT1x_SCK_HI;	SHT1x_DELAY;    // give a clock pulse
	SHT1x_SCK_LO;	SHT1x_DELAY; 
	
	if ( send_ack )
	{       // Release DATA line
		SHT1x_DATA_HI;	SHT1x_DELAY; 
	}

	return value;
}

////////////////////////////////////////////////////////////////////////////////////////

bool SHT1x::SHT1x_Sendbyte( unsigned char value) 
{
	unsigned char mask;
	bool ack;

	for(mask = 0x80; mask; mask>>=1)
	{
		SHT1x_SCK_LO;	SHT1x_DELAY;

		if( value & mask )
		{  
			SHT1x_DATA_HI; 	SHT1x_DELAY;
		}
		else
		{
			SHT1x_DATA_LO;	SHT1x_DELAY;
		}

		SHT1x_SCK_HI;	SHT1x_DELAY;  // SCK hi => sensor reads data
	}
	SHT1x_SCK_LO;	SHT1x_DELAY;

	// Release DATA line
	SHT1x_DATA_HI;	SHT1x_DELAY;
	SHT1x_SCK_HI;	SHT1x_DELAY;

	ack = false;

	if(!SHT1x_GET_BIT)
		ack = true;

	SHT1x_SCK_LO;	SHT1x_DELAY;

	SHT1x_Crc_Check(value);   // crc calculation

	return ack;
}

////////////////////////////////////////////////////////////////////////////////////////

bool SHT1x::SHT1x_Measure_Start(SHT1xMeasureType type) 
{
	assert(m_Initialized);

	// send a transmission start and reset crc calculation
	SHT1x_Transmission_Start();
	// send command. Crc gets updated!
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

bool SHT1x::SHT1x_Get_Measure_Value(unsigned short int * value ) 
{
	unsigned char * chPtr = (unsigned char*) value;
	unsigned char checksum;
	unsigned char delay_count=62;  // delay is 62 * 5ms 

	assert(m_Initialized);

	// Wait for measurement to complete (DATA pin gets LOW) 
	// Raise an error after we waited 250ms without success (210ms + 15%) 
	while( SHT1x_GET_BIT )
	{
		ets_delay_us(5000);			// $$$$$$$$$$$$$$$$$$ 1 ms not working $$$$$$$$$$$$$$$$$$$$$$$$
	
		delay_count--;
		if (delay_count == 0)
		{
			ESP_LOGE(TAG, "Timeout in SHT1x_Get_Measure_Value for Sensor with SCLK %d and DATA %d", (int)m_SHT1x_pin_sck,(int)m_SHT1x_pin_data);
			return false;
		}
			
	}

	*(chPtr + 1) = SHT1x_Readbyte(true);  // read hi byte
	SHT1x_Crc_Check(*(chPtr + 1));  		// crc calculation
	*chPtr = SHT1x_Readbyte(true);    	// read lo byte
	SHT1x_Crc_Check(*chPtr);    			// crc calculation

	checksum = SHT1x_Readbyte(false);   // crc calculation

	if (SHT1x_Mirrorbyte( checksum ) == m_SHT1x_crc)
	{
		return true;
	}
	else
	{
		ESP_LOGE(TAG, "CRC error in SHT1x_Get_Measure_Value for Sensor with SCLK %d and DATA %d", (int)m_SHT1x_pin_sck,(int)m_SHT1x_pin_data);
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void SHT1x::SHT1x_Calc(unsigned short int p_humidity ,unsigned short int p_temperature)
{
	const float C1=-2.0468;            		// for 12 Bit
	const float C2=+0.0367;           		// for 12 Bit
	const float C3=-0.0000015955;      		// for 12 Bit
	const float T1=+0.01;             		// for 12 Bit
	const float T2=+0.00008;           		// for 12 Bit

	const float	D1 = -39.66;			// For 3.3 Volt power supply, Centigrade
	const float	D2 = 0.01;			// For 14 Bit temperature, Centigrade

	float rh = p_humidity;             		// rh:      Humidity [Ticks] 12 Bit 
	float t = p_temperature;          		// t:       Temperature [Ticks] 14 Bit
	float rh_lin;                     		// rh_lin:  Humidity linear
	float rh_true;                    		// rh_true: Temperature compensated humidity
	float t_C;                        		// t_C   :  Temperature [C]

	t_C = D1 + (D2 * t);                  		// calc. temperature from ticks to [C]
	rh_lin = C1 + (C2 * rh) + (C3 * rh * rh);	// calc. humidity from ticks to [%RH]
	rh_true = (t_C - 25) *(T1 + (T2 * rh)) + rh_lin;// calc. temperature compensated humidity [%RH]
	if(rh_true>100) rh_true=100;       		// cut if the value is outside of
	if(rh_true<0.1)	rh_true=0.1;       		// the physical possible range

	m_temp 	= t_C;               			// return temperature [C]
	m_rh 	= rh_true;              		// return humidity[%RH]
}

////////////////////////////////////////////////////////////////////////////////////////

float SHT1x::SHT1x_CalcDewpoint(float fRH ,float fTemp)
{
	// Set some constants for the temperature range
	float Tn = 243.12;
	float m = 17.62;
	if (fTemp < 0)
	{
		Tn = 272.62;
		m = 22.46;
	}
	float lnRH = log(fRH/100);
	float mTTnT = (m * fTemp)/(Tn+fTemp);
	
	return (float)(Tn * ((lnRH + mTTnT)/(m - lnRH - mTTnT)));
}
*/

/*
////////////////////////////////////////////////////////////////////////////////////////

float SHT1x::SHT1x_CalcAbsHumidity(float r ,float T)
{
	// --- this function returns the absolute humidity in g/m3 for r in % and T in °C
*/
	/*
	
	Bezeichnungen:
	
	r = relative Luftfeuchte
	T = Temperatur in °C
	TK = Temperatur in Kelvin (TK = T + 273.15)
	TD = Taupunkttemperatur in °C
	DD = Dampfdruck in hPa
	SDD = Sättigungsdampfdruck in hPa

	Parameter:
	a = 7.5, b = 237.3 für T >= 0
	a = 7.6, b = 240.7 für T < 0 über Wasser (Taupunkt)
	a = 9.5, b = 265.5 für T < 0 über Eis (Frostpunkt)

	R* = 8314.3 J/(kmol*K) (universelle Gaskonstante)
	mw = 18.016 kg/kmol (Molekulargewicht des Wasserdampfes)
	AF = absolute Feuchte in g Wasserdampf pro m3 Luft

	Formeln:

	SDD(T) = 6.1078 * 10^((a*T)/(b+T))

	DD(r,T) = r/100 * SDD(T)

	r(T,TD) = 100 * SDD(TD) / SDD(T)

	TD(r,T) = b*v/(a-v) mit v(r,T) = log10(DD(r,T)/6.1078)

	AF(r,TK) = 10^5 * mw/R* * DD(r,T)/TK; AF(TD,TK) = 10^5 * mw/R* * SDD(TD)/TK

	*/
	/*
	float a,b;
	if (T < 0)
	{
		a = 7.6;
		b = 240.7;
	}
	else
	{
		a = 7.5;
		b = 237.3;
	}

	float SDD 	= 6.1078 * pow(10,((a*T)/(b+T)));
	float DD 	= r/100 * SDD;
	float TK 	= T + 273.15;

	float AF = 100000 * 18.016/8314.3 * DD/TK; 
	
	return AF;
}
*/
////////////////////////////////////////////////////////////////////////////////////////

bool CVindriktning::SetupSensor(gpio_num_t f_data)
{
	m_pin_data	= f_data;

	// --- set hardware pins

	//SHT1x_InitPins();
	
	// --- Reset the SHT1x

	//SHT1x_Reset();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

bool CVindriktning::PerformMeasurement(void)
{
	m_pm2 = rand() % 1000;
	m_pm1 = rand() % 1000;
	m_pm10 = rand() % 1000;

	ESP_LOGE(TAG, "SCVindriktning stub implementation generating random values");

	return true;
}