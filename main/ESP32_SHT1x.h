/*

ESP32 IDF SHT1x communication library
By:      Tim Hagemann (tim@way2.net)
Date:    26 November 2019
License: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/

This is a derivative work based on:

	Raspberry Pi SHT1x communication library.
	By:      John Burns (www.john.geek.nz)
	Date:    01 November 2012
	License: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/

	Adapted by Tim Hagemann (tim@way2.net) in Sep'14 to support multiple sensors

	This is a derivative work based on
	
		Name: Nice Guy SHT11 library
		By: Daesung Kim
		Date: 04/04/2011
		Source: http://www.theniceguy.net/2722
		License: Unknown - Attempts have been made to contact the author
*/

////////////////////////////////////////////////////////////////////////////////////////

#ifndef ESP32_SHT1x_H_
#define	ESP32_SHT1x_H_

////////////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>

/*// Defines
#define	TRUE	1
#define	FALSE	0
*/

/* Definitions of all known SHT1x commands */

#define SHT1x_MEAS_T	0x03			// Start measuring of temperature.
#define SHT1x_MEAS_RH	0x05			// Start measuring of humidity.
#define SHT1x_STATUS_R	0x07			// Read status register.
#define SHT1x_STATUS_W	0x06			// Write status register.
#define SHT1x_RESET		0x1E			// Perform a sensor soft reset.

////////////////////////////////////////////////////////////////////////////////////////

/* Enum to select between temperature and humidity measuring */
typedef enum _SHT1xMeasureType {
	SHT1xMeaT	= SHT1x_MEAS_T,		// Temperature
	SHT1xMeaRh	= SHT1x_MEAS_RH		// Humidity
} SHT1xMeasureType;

////////////////////////////////////////////////////////////////////////////////////////

class SHT1x
{

public:

	// --- construct 

	SHT1x(void);

	// --- actions

	bool SetupSensor(gpio_num_t f_sck,gpio_num_t f_data);
	bool PerformMeasurement(void);

	// --- utiliy conversion

	static float SHT1x_CalcDewpoint(float fRH ,float fTemp);
	static float SHT1x_CalcAbsHumidity(float r ,float T);

	// --- getter

	float GetTemp(void) const
	{
		return m_temp;
	}

	float GetRH(void) const
	{
		return m_rh;
	}

	float GetDP(void) const
	{
		return SHT1x_CalcDewpoint(m_rh,m_temp);
	}

private:

	void SHT1x_Transmission_Start(void);
	bool SHT1x_Sendbyte(unsigned char value );
	bool SHT1x_InitPins(void);
	bool SHT1x_Measure_Start(SHT1xMeasureType type );
	bool SHT1x_Get_Measure_Value(unsigned short int * value );
	bool SHT1x_Reset(void);

	unsigned char SHT1x_Readbyte(bool sendAck);

	void SHT1x_Calc(unsigned short int p_humidity ,unsigned short int p_temperature);


	unsigned char SHT1x_Mirrorbyte(unsigned char value);
	void SHT1x_Crc_Check(unsigned char value);

	float m_temp;
	float m_rh;

	gpio_num_t m_SHT1x_pin_sck;
	gpio_num_t m_SHT1x_pin_data;
	
	unsigned char m_SHT1x_crc;
	unsigned char m_SHT1x_status_reg;

	bool m_Initialized;
};

////////////////////////////////////////////////////////////////////////////////////////

#endif

