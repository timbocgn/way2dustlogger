////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSOR_MANAGER_H_
#define	SENSOR_MANAGER_H_

////////////////////////////////////////////////////////////////////////////////////////

#include "ESP32_SHT1x.h"
#include "sdkconfig.h"

////////////////////////////////////////////////////////////////////////////////////////

class SensorWrapper
{

public:
    SensorWrapper(void)
    {

    }

	bool SetupSensor(gpio_num_t f_sck,gpio_num_t f_data)
    {
        return m_Sensor.SetupSensor(f_sck,f_data);
    }

	bool PerformMeasurement(void)
    {
        return m_Sensor.PerformMeasurement();
    }

	float GetTemp(void) 
	{
		return m_Sensor.GetTemp();
	}

	float GetRH(void) 
	{
		return m_Sensor.GetRH();
	}

	float GetDP(void) 
	{
		return m_Sensor.GetDP();
	}

private:

    SHT1x m_Sensor;
};

////////////////////////////////////////////////////////////////////////////////////////

class SensorManager
{
public:

    // --- action functions

    void ProcessMeasurements(void);
    void InitSensors(void);

    // --- low level getters

    SensorWrapper &GetSensor(int f_idx)
    {
        assert(f_idx < CONFIG_TEMP_SENSOR_CNT);
        return m_Sensors[f_idx];
    }

private:
    SensorWrapper m_Sensors[CONFIG_TEMP_SENSOR_CNT];
};

////////////////////////////////////////////////////////////////////////////////////////


extern SensorManager g_SensorManager;


#endif