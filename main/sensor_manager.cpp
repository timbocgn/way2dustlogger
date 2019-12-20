

////////////////////////////////////////////////////////////////////////////////////////

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
            ESP_LOGI(TAG, "Sensor %d: Temp %f, rh %f, dp %f",i,m_Sensors[i].GetTemp(),m_Sensors[i].GetRH(),m_Sensors[i].GetDP());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////

void SensorManager::InitSensors(void)
{
    // --- first get the #define config data into an array

    gpio_num_t l_SCLK[CONFIG_TEMP_SENSOR_CNT];
    gpio_num_t l_DATA[CONFIG_TEMP_SENSOR_CNT];

#if CONFIG_TEMP_SENSOR_CNT >= 1
    l_SCLK[0] = (gpio_num_t)CONFIG_TEMP_SENSOR1_SCLK_GPIO;
    l_DATA[0] = (gpio_num_t)CONFIG_TEMP_SENSOR1_DATA_GPIO;
#endif

#if CONFIG_TEMP_SENSOR_CNT >= 2
    l_SCLK[1] = (gpio_num_t)CONFIG_TEMP_SENSOR2_SCLK_GPIO;
    l_DATA[1] = (gpio_num_t)CONFIG_TEMP_SENSOR2_DATA_GPIO;
#endif

#if CONFIG_TEMP_SENSOR_CNT >= 3
    l_SCLK[2] = (gpio_num_t)CONFIG_TEMP_SENSOR3_SCLK_GPIO;
    l_DATA[2] = (gpio_num_t)CONFIG_TEMP_SENSOR3_DATA_GPIO;
#endif

#if CONFIG_TEMP_SENSOR_CNT >= 4
    #error You need to add additional lines here!
#endif

    // --- now configure all sensors

    for (int i = 0; i < CONFIG_TEMP_SENSOR_CNT; ++i)
    {
        ESP_LOGI(TAG, "Sensor %d GPIOs: SCLK %d DATA %d", i,l_SCLK[i],l_DATA[i]);
        if (!m_Sensors[i].SetupSensor(l_SCLK[i],l_DATA[i]))
        {
            ESP_LOGE(TAG, "Failed to initialize sensor %d", i);
        }
    }
}