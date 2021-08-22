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

///////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "sensor_manager.h"

////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "SensorManager";

////////////////////////////////////////////////////////////////////////////////////////

SensorManager g_SensorManager;

////////////////////////////////////////////////////////////////////////////////////////

void SensorManager::ProcessMeasurements(void)
{
   // --- now measure on all sensors

    for (int i = 0; i < CONFIG_TEMP_SENSOR_CNT; ++i)
    {
        if (!m_Sensors[i].PerformMeasurement())
        {
            ESP_LOGE(TAG, "Failed to perform a mesurement on sensor %d", i);
        }
        else
        {
            ESP_LOGI(TAG, "Sensor %d: pm1 %f, pm2.5 %f, pm10 %f",i,m_Sensors[i].GetPM1(),m_Sensors[i].GetPM2(),m_Sensors[i].GetPM10());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////

void SensorManager::InitSensors(void)
{
    // --- first get the #define config data into an array

    gpio_num_t l_DATA[CONFIG_TEMP_SENSOR_CNT];

#if CONFIG_TEMP_SENSOR_CNT >= 1
    l_DATA[0] = (gpio_num_t)CONFIG_TEMP_SENSOR1_DATA_GPIO;
#endif

#if CONFIG_TEMP_SENSOR_CNT >= 2
    l_DATA[1] = (gpio_num_t)CONFIG_TEMP_SENSOR2_DATA_GPIO;
#endif

#if CONFIG_TEMP_SENSOR_CNT >= 3
    l_DATA[2] = (gpio_num_t)CONFIG_TEMP_SENSOR3_DATA_GPIO;
#endif

#if CONFIG_TEMP_SENSOR_CNT >= 4
    #error You need to add additional lines here!
#endif

    // --- now configure all sensors

    for (int i = 0; i < CONFIG_TEMP_SENSOR_CNT; ++i)
    {
        ESP_LOGI(TAG, "Sensor %d GPIOs: DATA %d", i,l_DATA[i]);
        if (!m_Sensors[i].SetupSensor(l_DATA[i]))
        {
            ESP_LOGE(TAG, "Failed to initialize sensor %d", i);
        }
    }
}