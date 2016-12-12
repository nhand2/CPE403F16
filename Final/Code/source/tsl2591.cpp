#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

#include "tsl2591.h"
#include "SimpleGPIO.h"
using namespace std;

#define TSL2591_VISIBLE 		(2)
#define TSL2591_INFRARED 		(1)
#define TSL2591_FULLSPECTRUM	(0)

#define TSL2591_ADDR			(0x29)
#define TSL2591_READBIT			(0x01)

#define TSL2591_COMMAND_BIT		(0xA0)
#define TSL2591_CLEAR_INT		(0xE7)
#define TSL2591_TEST_INT		(0xE4)
#define TSL2591_WORD_BIT		(0x20)
#define TSL2591_BLOCK_BIT		(0x10)

#define TSL2591_ENABLE_POWEROFF	(0x00)
#define TSL2591_ENABLE_POWERON	(0x01)
#define TSL2591_ENABLE_AEN		(0x02)
#define TSL2591_ENABLE_AIEN		(0x10)
#define TSL2591_ENABLE_NPIEN	(0x80)

#define TSL2591_LUX_DF			(408.0f)
#define TSL2591_LUX_COEFB		(1.64f)
#define TSL2591_LUX_COEFC		(0.59f)
#define TSL2591_LUX_COEFD		(0.86f)

#define TSL2591_ENABLE			(0x00)
#define TSL2591_CONFIG 			(0x01)
#define TSL2591_ID				(0x12)
#define TSL2591_C0DATAL			(0x14)
#define TSL2591_C0DATAH			(0x15)
#define TSL2591_C1DATAL			(0x16)
#define TSL2591_C1DATAH			(0x17)

TSL2591Lux::TSL2591Lux(int bus, int address)
//constructor to configure the sensor and sets the bus and address values
{
	I2CBus = bus;
	I2CAddress = address;
	WriteI2C (TSL2591_CONFIG, 0x10);	//medium gain on the TSL
	WriteI2C (TSL2591_ENABLE, (TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | TSL2591_ENABLE_NPIEN));	//sets the configurations
	ReadDevice ();	//reads the entire device
}

int TSL2591Lux::ReadDevice ()
//Reads all the registers of the I2C device
{
	char namebuf[64];

	snprintf (namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);	//write "/dev/i2c-x" to the namebuf var, where x is the bus #

	int file = open(namebuf, O_RDWR);	//opens the i2c device at the bus designated

	if (file < 0)
	{
		cout << "Failed to read TSL2591 on " << namebuf << " I2C Bus" << endl;
		return (1);
	}
	if (ioctl (file, I2C_SLAVE, I2CAddress) < 0) //opens the slave address
	{
		cout << "I2C_Slave address " << I2CAddress << " failed..." << endl;
		return (2);
	}

	int numberBytes = TSL2591_I2C_BUFFER;	//the number of bytes to read
	int bytesRead = read (file, this->dataBuffer, numberBytes);	//reads the device path file, store the characters in a buffer
	if (bytesRead != numberBytes)
	//sends an error if the bytes are not read correctly
	{
		cout << "Failure to read Byte Stream in ReadDevice()" << endl;
	}
	close (file);	//close the file

	/*
	for (int i = 0; i < numberBytes; i++)
	{
		if (i % 10 == 0)
			cout<< "\n" <<endl;
		printf ("%x ", this->dataBuffer[i]);
	}
	 */
	//printf ("Device ID %x\n", (this->dataBuffer[(TSL2591_ID + 0x01)]));

	if (this->dataBuffer[(TSL2591_ID + 0x01)] != 0x50)
	//if the device address is incorrect, send an error
	//after printing buffer data, ID was stored at index TSL2591_ID + 1
	{
		cout << "DEVICE ID INCORRECT!!" << endl;
	}

	this -> ch0Lux = GetChannelLux ((TSL2591_C0DATAH + 0x01), (TSL2591_C0DATAL + 0x01));	//gets the channel lux values
	this -> ch1Lux = GetChannelLux (TSL2591_C1DATAH + 0x01, (TSL2591_C1DATAL + 0x01));		//gets the channel lux value
	this -> luminosity = CalcLuminosity (ch0Lux, ch1Lux);	//calculates the luminosity

	return 0;
}

int TSL2591Lux::GetChannelLux (int msb_reg_addr, int lsb_reg_addr)
//gets the channels lux values
{
	short temp = dataBuffer[msb_reg_addr];			//stores upper 8 bits
	temp = (temp<<8) | dataBuffer[lsb_reg_addr];	//shift left to store lower 8 bits

	return (int)temp;
}

float TSL2591Lux::CalcLuminosity (int chan0, int chan1)
//calculates the luminosity by passing both channel lux
{
	float atime = 100.0f, again = 25.0f;
	float cp1, lux1, lux2, lux;

	cp1 = (atime * again) / TSL2591_LUX_DF;
	lux1 = ((float) chan0 - (TSL2591_LUX_COEFB * (float) chan1)) / cp1;
	lux2 = ((TSL2591_LUX_COEFC * (float) chan0) - (TSL2591_LUX_COEFD * (float) chan1)) / cp1;

	return (lux = (lux1 > lux2) ? lux1:lux2);
}

int TSL2591Lux::WriteI2C(char addr, char value)
//writes to the I2C device
{
	cout<< "Writing to I2C Device!!" << endl;
	char namebuf[64];
	snprintf (namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	int file = open (namebuf, O_RDWR);
	if (file < 0)
	{
		cout << "Sensor failed to read!!" << endl;
		return (1);
	}

	if (ioctl (file, I2C_SLAVE, I2CAddress) < 0)
	{
		cout << "I2C_SLAVE address "<< I2CAddress << " failed..." << endl;
		return (2);
	}

	char buffer[2];
	buffer[0] = addr;
	buffer[1] = value;

	if (write (file, buffer, 2) != 2)
	{
		cout << "Failure to write values to I2C device" << endl;
		return (3);
	}
	close (file);
	cout << "Finished writing to TSL2591!" << endl;
	return 0;
}

TSL2591Lux::~TSL2591Lux ()
{

}
