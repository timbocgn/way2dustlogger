////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

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

    esp_err_t InitInitManager(void);

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