////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_MANAGER_H_
#define	CONFIG_MANAGER_H_

////////////////////////////////////////////////////////////////////////////////////////

#include "ESP32_SHT1x.h"
#include "sdkconfig.h"

////////////////////////////////////////////////////////////////////////////////////////

class ConfigManager
{
public:

    ConfigManager()
    {
        m_nvs_handle = 0;
    }

    // --- init functions

    esp_err_t InitConfigManager(void);
    void ShutdownConfigManager(void);

    // --- getters / setters

    esp_err_t SetStringValue(const char *f_key,const char *f_value);

    esp_err_t SetStringValue(const char *f_key,const std::string &f_value)
    {
        return SetStringValue(f_key,f_value.c_str());
    }
    
    std::string GetStringValue(const char *f_key);

    esp_err_t SetIntValue(const char *f_key,int f_value);
    int GetIntValue(const char *f_key);
    
private:

    nvs_handle_t m_nvs_handle;
    
};

////////////////////////////////////////////////////////////////////////////////////////


extern ConfigManager g_ConfigManager;


#endif