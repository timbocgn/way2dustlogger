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
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "esp_event.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "mqtt_client.h"

#include "config_manager.h"
#include "config_manager_defines.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"

/*
#define CFMGR_MQTT_SERVER       "mqtt_server"
#define CFMGR_MQTT_PORT         "mqtt_port"
#define CFMGR_MQTT_TOPIC        "mqtt_topic"
#define CFMGR_MQTT_TIME         "mqtt_time"
#define CFMGR_MQTT_ENABLE       "mqtt_enable"
*/

////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "MqttManager";

////////////////////////////////////////////////////////////////////////////////////////

static void prvMqttTimerCallback( TimerHandle_t xExpiredTimer )
{
    MqttManager *l_mqttmgr;

    // --- Obtain the address of the info manager

    l_mqttmgr = (MqttManager *) pvTimerGetTimerID( xExpiredTimer );

    l_mqttmgr->ProcessCallback();
}

////////////////////////////////////////////////////////////////////////////////////////

void MqttManager::ProcessCallback(void)
{
    // ---- mqtt is off, do nothing

    if (!m_mqtt_enabled) return;

    // ---- decrease the counter and send message, when zero

    --m_delay_current;

    if (!m_delay_current)
    {
        // --- reload our counter

        m_delay_current = m_mqtt_delay;

        std::string l_topic = g_ConfigManager.GetStringValue(CFMGR_MQTT_TOPIC);

        // --- now loop over all sensors and send a message

        for (int l_senidx = 0; l_senidx < g_SensorManager.GetSensorCount(); ++l_senidx)
        {

            // ---- now ask the sensor for the values and create a JSON from that    

            cJSON *root = cJSON_CreateObject();
            
            cJSON_AddNumberToObject(root, "pm1", g_SensorManager.GetSensor(l_senidx).GetPM1());
            cJSON_AddNumberToObject(root, "pm2", g_SensorManager.GetSensor(l_senidx).GetPM2());
            cJSON_AddNumberToObject(root, "pm10", g_SensorManager.GetSensor(l_senidx).GetPM10());
            
            const char *sys_info = cJSON_Print(root);
            
            char l_snum[5];

            std::string l_fulltopic = l_topic;
            l_fulltopic += "/sensor";
            l_fulltopic += itoa(l_senidx+1,l_snum,10);

            int l_err = esp_mqtt_client_publish(m_mqtt_hdl, l_fulltopic.c_str(), sys_info,0, 0,0);
            if (l_err == -1)
            {
                ESP_LOGE(TAG, "Error sending mqtt message '%s' to topic %s", sys_info,l_fulltopic.c_str());
            }
            else
            {
                ESP_LOGI(TAG, "Successfully send mqtt message.");
            }

            free((void *)sys_info);
            cJSON_Delete(root);
        }


    }
}

////////////////////////////////////////////////////////////////////////////////////////

esp_err_t MqttManager::InitManager(void)
{
    ESP_LOGE(TAG, "initmgr");

    std::string l_server = g_ConfigManager.GetStringValue(CFMGR_MQTT_SERVER);

    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg,0,sizeof(esp_mqtt_client_config_t));

    mqtt_cfg.uri = l_server.c_str();

    ESP_LOGE(TAG, "before esp_mqtt_client_init");
    m_mqtt_hdl = esp_mqtt_client_init(&mqtt_cfg);
    if (!m_mqtt_hdl)
    {
        ESP_LOGE(TAG, "Error on esp_mqtt_client_init (%s)", l_server.c_str());
        return ESP_FAIL;
    }
    ESP_LOGE(TAG, "after esp_mqtt_client_init");

    esp_err_t l_ee = esp_mqtt_client_start(m_mqtt_hdl);
    if (l_ee != ESP_OK)
    {
        ESP_LOGE(TAG, "Error on esp_mqtt_client_start (%s): %d", l_server.c_str(),l_ee);
        return l_ee;
    }
   ESP_LOGE(TAG, "after esp_mqtt_client_start");

    // ---- get all config values to the manager

    UpdateConfig();

    // ---- timer stuff

    m_timer = xTimerCreate( "T1", 1000 / portTICK_PERIOD_MS, pdTRUE, (void *)this, prvMqttTimerCallback);
    xTimerStart( m_timer, 0 );



    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

void MqttManager::UpdateConfig(void)
{
    // ---- get some flag since we use them frequently

    m_mqtt_enabled = g_ConfigManager.GetIntValue(CFMGR_MQTT_ENABLE) == 1;
    m_mqtt_delay = g_ConfigManager.GetIntValue(CFMGR_MQTT_TIME);

    m_delay_current = m_mqtt_delay;

    // ---- the broker might have changed, so close the existing connection

    if (m_mqtt_hdl)
    {
        std::string l_server = g_ConfigManager.GetStringValue(CFMGR_MQTT_SERVER);

        esp_mqtt_client_set_uri(m_mqtt_hdl,l_server.c_str());
    }

}

////////////////////////////////////////////////////////////////////////////////////////

MqttManager g_MqttManager;

////////////////////////////////////////////////////////////////////////////////////////