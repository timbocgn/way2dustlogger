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

///////////////////////////////////////////////////////////////////////////////////////

#ifndef INFO_MANAGER_H_
#define	INFO_MANAGER_H_

////////////////////////////////////////////////////////////////////////////////////////

#include "sdkconfig.h"
#include "freertos/timers.h"


enum InfoMode
{
    InfoMode_Nothing,
    InfoMode_WaitToConnect,
    InfoMode_Connected,
    InfoMode_Bootstrap,
};

////////////////////////////////////////////////////////////////////////////////////////

class InfoManager
{
public:
    InfoManager()
    {
        m_InfoMode = InfoMode_Nothing;
    }

    esp_err_t InitManager(void);

    bool IsBootstrapActivated(void)
    {
        return gpio_get_level(m_bootstrappin) == 0;
    }

    void SetInfoPin(bool f_value)
    {
        gpio_set_level(m_infopin,f_value ? 1 : 0);
    }

    void SetMode(InfoMode f_mode)
    {
        m_InfoMode = f_mode;
    }

    InfoMode GetMode(void) const
    {
        return m_InfoMode;
    }

private:

    gpio_num_t      m_bootstrappin;
    gpio_num_t      m_infopin;

    TimerHandle_t   m_timer;
    InfoMode        m_InfoMode;
};

////////////////////////////////////////////////////////////////////////////////////////


extern InfoManager g_InfoManager;


#endif