

////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "infomanager.h"

////////////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "InfoManager";

////////////////////////////////////////////////////////////////////////////////////////

// The buffer used to hold the software timer's data structure.

static StaticTimer_t xTimerBuffer;

////////////////////////////////////////////////////////////////////////////////////////

static void prvTimerCallback( TimerHandle_t xExpiredTimer )
{
    InfoManager *l_infomgr;

    // --- Obtain the address of the info manager

    l_infomgr = (InfoManager *) pvTimerGetTimerID( xExpiredTimer );

    // --- one 100ms flash --> allright, connected

    if (l_infomgr->GetMode() == InfoMode_Connected)
    {
        l_infomgr->SetInfoPin(true);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        l_infomgr->SetInfoPin(false);
    }

    // --- two 100ms flashes --> waiting for WLAN connection
    
    if (l_infomgr->GetMode() == InfoMode_WaitToConnect)
    {
        l_infomgr->SetInfoPin(true);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        l_infomgr->SetInfoPin(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        l_infomgr->SetInfoPin(true);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        l_infomgr->SetInfoPin(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    // --- one long flash -> bootstrapping

   if (l_infomgr->GetMode() == InfoMode_Bootstrap)
    {
        l_infomgr->SetInfoPin(true);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        l_infomgr->SetInfoPin(false);
    }
}

////////////////////////////////////////////////////////////////////////////////////////

esp_err_t InfoManager::InitInitManager(void)
{
    // --- hardware stuff

    m_bootstrappin = (gpio_num_t)CONFIG_BOOTSTRAP_GPIO;
    m_infopin = (gpio_num_t)CONFIG_INFOLED_GPIO;

    // --- configure the bootstrap pin s input

    gpio_reset_pin(m_bootstrappin);
    esp_err_t l_err = gpio_set_direction(m_bootstrappin,GPIO_MODE_INPUT);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "Error setting bootstrap pin (%d) direction to input",m_bootstrappin); return l_err; }

    gpio_reset_pin(m_infopin);
    l_err = gpio_set_direction(m_infopin,GPIO_MODE_OUTPUT);
	if (l_err != ESP_OK) { ESP_LOGE(TAG, "Error setting info LED pin (%d) direction to output",m_infopin); return l_err;}

    // ---- timer stuff

    m_timer = xTimerCreate( "T1", 2000 / portTICK_PERIOD_MS, pdTRUE, (void *)this, prvTimerCallback);
    
    // The scheduler has not started yet so a block time is not used.
    xTimerStart( m_timer, 0 );

    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

InfoManager g_InfoManager;