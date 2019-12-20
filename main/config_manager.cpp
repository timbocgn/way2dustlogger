

////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "config_manager.h"

////////////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "ConfigManager";

////////////////////////////////////////////////////////////////////////////////////////

ConfigManager g_ConfigManager;

esp_err_t ConfigManager::InitConfigManager(void)
{
    // ----- Initialize NVS
    
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // --- NVS partition was truncated and needs to be erased
        // --- Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error initialising NVS storage %s",esp_err_to_name(err)); 
        return err;      
    }

    // ---- open nvs handle
   
    err = nvs_open("storage", NVS_READWRITE, &m_nvs_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error openning NVS storage %s",esp_err_to_name(err)); 
        return err;      
    }

    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

void ConfigManager::ShutdownConfigManager(void)
{
    if (m_nvs_handle)
    {
        esp_err_t err = nvs_commit(m_nvs_handle);
        if (err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Error committing NVS storage on shutdown: %s",esp_err_to_name(err)); 
        }

        nvs_close(m_nvs_handle);
    }
    
}

////////////////////////////////////////////////////////////////////////////////////////

esp_err_t ConfigManager::SetStringValue(const char *f_key,const char *f_value)
{
    assert(m_nvs_handle);

    esp_err_t err = nvs_set_str(m_nvs_handle,f_key,f_value);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error writing key '%s' to '%s': %s",f_key,f_value,esp_err_to_name(err)); 
        return err;
    }

    err = nvs_commit(m_nvs_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error to commit after writing key '%s' to '%s': %s",f_key,f_value,esp_err_to_name(err)); 
        return err;
    }

    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

std::string ConfigManager::GetStringValue(const char *f_key)
{    
    assert(m_nvs_handle);

    // --- get the size first

    size_t required_size;
    esp_err_t err = nvs_get_str(m_nvs_handle, f_key, NULL, &required_size);

    // --- check for "empty" string

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        // ---- return empty string

        return string();
    }

    // --- every other error: fail

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error reading key '%s': %s",f_key,esp_err_to_name(err)); 
        return string();
    }

    // --- allocate the buffer and read the string

    char *buffer = (char *)malloc(required_size);
    nvs_get_str(m_nvs_handle, f_key, buffer, &required_size);

    // --- copy to c++ string
    
    string f_s(buffer);

    // --- free temp buffer again

    free(buffer);

    return f_s;    
}

////////////////////////////////////////////////////////////////////////////////////////

esp_err_t ConfigManager::SetIntValue(const char *f_key,int f_value)
{
    assert(m_nvs_handle);

    esp_err_t err = nvs_set_i32(m_nvs_handle,f_key,(int32_t)f_value);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error writing key '%s' to '%d': %s",f_key,f_value,esp_err_to_name(err)); 
        return err;
    }

    err = nvs_commit(m_nvs_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error to commit after writing key '%s' to '%d': %s",f_key,f_value,esp_err_to_name(err)); 
        return err;
    }

    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

int ConfigManager::GetIntValue(const char *f_key)
{
   assert(m_nvs_handle);

    // --- get the size first

    int32_t f_i;
    esp_err_t err = nvs_get_i32(m_nvs_handle, f_key,&f_i);

    // --- check for "empty" string

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        // ---- return empty string

        return 0;
    }

    // --- every other error: fail

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error reading key '%s': %s",f_key,esp_err_to_name(err)); 
        return 0;
    }

    return (int)f_i;        
}
