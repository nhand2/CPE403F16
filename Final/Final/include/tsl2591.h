/*
 * tsl2591.h
 *
 *  Created on: Dec 9, 2016
 *      Author: DerekN
 */

#ifndef INCLUDE_TSL2591_H_
#define INCLUDE_TSL2591_H_
#define TSL2591_I2C_BUFFER 64

class TSL2591Lux
{
	private:
		int I2CBus, I2CAddress;		//the bus and address of the I2C device
		char dataBuffer [TSL2591_I2C_BUFFER];	//buffer to hold all values of the I2C bus

		int ch0Lux, ch1Lux;	//holds the luminosity value of the lux sensor
		float luminosity;

		int GetChannelLux (int msb_addr, int lsb_addr);	//returns the lux of a channel
		float CalcLuminosity (int chan0, int chan1);	//calculates the luminosity of the lux sesnor
		int WriteI2C (char addr, char value);			//writes to the I2C device

	public:
		TSL2591Lux (int bus, int address);				//constructor

		float GetLuminosity() { return luminosity; }	//returns the luminosity
		int ReadDevice ();		//reads the I2C device

		virtual ~TSL2591Lux ();
};

#endif /* INCLUDE_TSL2591_H_ */
