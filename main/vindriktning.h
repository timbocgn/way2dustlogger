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

////////////////////////////////////////////////////////////////////////////////////////

#ifndef ESP32_vindriktning_H_
#define	ESP32_vindriktning_H_

////////////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>


////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////

class CVindriktning
{

public:

	// --- construct 

	CVindriktning(void);

	// --- actions

	bool SetupSensor(gpio_num_t f_data);
	bool PerformMeasurement(void);

	// --- getter

	float GetPM2(void) const
	{
		return m_pm2;
	}

	float GetPM1(void) const
	{
		return m_pm1;
	}

	float GetPM10(void) const
	{
		return m_pm10;
	}

private:

	float m_pm2;
	float m_pm10;
	float m_pm1;
	
	gpio_num_t m_pin_data;
	
	bool m_Initialized;
};

////////////////////////////////////////////////////////////////////////////////////////

#endif

