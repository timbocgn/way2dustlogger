/*
    --------------------------------------------------------------------------------

    ESPTempLogger       
    
    ESP32 based IoT Device for temperature logging featuring an MQTT client and 
    REST API acess.
    
    --------------------------------------------------------------------------------

    Copyright (c) 2019 Tim Hagemann / way2.net Services

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