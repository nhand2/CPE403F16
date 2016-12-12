/*
 * main.cpp
 *
 *  Created on: Dec 9, 2016
 *      Author: DerekN
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "SimpleGPIO.h"		//header file for GPIO interface
#include "TSL2591.h"		//header file for TSL2591
#include "SocketClient.h"	//header file for creation of socket

using namespace std;
using namespace exploringBB;

#define ADC_PATH "/sys/bus/iio/devices/iio:device0/in_voltage"	//path of the ADC in the Beagle Bone's Linux

int ReadAnalog (int number)
//This function reads the analog data from the passed ADC pin
//number is the ADC pin the data is being read from
{
	stringstream ss;	//stringstream that will hold the concatenated file path
	fstream fs;			//filestream to open the file and store the returned value
	int value;			//holds the value of the ADC

	ss << ADC_PATH << number << "_raw";			//concatenates the adc path to remain general
	fs.open (ss.str().c_str(), fstream::in);	//opens the file path
	fs >> value;	//stores the value  returned in to the value variable
	fs.close ();	//close the file
	return value;	//reutn the adc value
}

float GetDistance (int adc_value)
//Calculates the distance of the IR sensor
//Takes in the ADC raw value and converts it to a voltages by multiplying by 1.8 and dividing by 12-bits or 4096
//Multiply the value by 10 to make it easier to interpret
{
	float curr_volts = adc_value * (1.80f/4096.0f);
	float distance = curr_volts * 10.0f;

	return distance;
}

int SendEmail (string subject, string bodyText)
//Function that will send an email to the use
//Takes in subject line string and the text to send in the body
{
	string to("derek.nhan@gmail.com");	//send the email to the my email address
	stringstream command;	//command string stream to concatenate the command

	command << "echo \"" << bodyText << "\" | mail -s \"" << subject << "\" " << to;	//command to execute to send email
	int result = system (command.str().c_str());	//send the email command
	cout << "Command: " << command.str () << endl;	//returns the command sent to the terminal to ensure proper communication
	cout << "The return value was " << result << endl;
	return result;
}

int main (int arc, char *argv[])
{
	float distance;		//distance of the ir sensor
	int timeElapsed = 0;	//the time elapsed
	bool lightEmailSent = false, doorEmailSent = false;	//bools to determine is an email is sent so only 1 email is sent per event
	bool doorDataSend = false, lightDataSend = false;	//allows events to send data to thingspeak

	TSL2591Lux lux (2, 0x29);	//calls the lux sensors address (0x29) and the i2c bus the device is attached to

	distance = GetDistance (ReadAnalog (0));	//reads the first data, which is dead beef (datasheet)
	while (1)
	{
		distance = 0;
		for (int i = 0; i < 20; i++)
		//average the distances to get a more accurate reading
		{
			distance += GetDistance (ReadAnalog (0));
		}
		distance = distance / 20.0f;

		cout << "The distance is " << distance << endl;		//outputs the distance to ensure proper sending

		if (distance < 2.0f && !doorEmailSent)
		//Sends an email when the distance is close to 0, this means someone is at the door
		{
			SendEmail ("Door Alert", "Check the door!");
			doorEmailSent = true;
			doorDataSend = true;
		}
		else
		{
			if (distance > 2.0f && doorEmailSent)
			//sends an email when the vistor leaves
			{
				SendEmail ("Door Alert", "Visitor has left!");
				doorEmailSent = false;
				doorDataSend = true;
			}
		}

		ostringstream head, data;
		lux.ReadDevice ();	//reads the data of the I2C device
		float luminosity = lux.GetLuminosity ();	//gets the luminosity of the lux sensor

		cout << "Current Luminosity: " << luminosity << endl;	//sends the luminosity read to the terminal

		if (luminosity <= 0 && !lightEmailSent)
		//when the luminosity is 0, send an email to the user to notify that the light is turned on (also turns on the light)
		{
			lightEmailSent = true;
			lightDataSend = true;
			cout<< "TURNING ON LIGHTS!" << endl;
			gpio_set_value (66, HIGH);	//sets the GPIO to send a high value
			SendEmail ("Lights are ON!", "The lights are now on!");	//sends an email
		}
		else
		{
			if (luminosity >= 1.0f && lightEmailSent)
			//when the luminosity if higher than 1, the light is turned off
			{
				lightDataSend = true;
				lightEmailSent = false;
				cout << "LIGHT IS OFF!" << endl;
				gpio_set_value (66, LOW);
			}
		}

		if (timeElapsed%30 == 0 || doorDataSend || lightDataSend)
		//every 30 seconds or significant events will send data to thingspeak
		//Both lux data and ir data is sent to he cloud
		{
			cout << "Starting Send to ThingSpeak" << endl;
			SocketClient sc("184.106.153.149", 80);
			data <<"field1=" << luminosity << "&field2=" << distance << endl;
			sc.connectToServer ();
			head << "POST /update HTTP/1.1\n"
				 << "Host: api.thingspeak.com\n"
				 << "Connection: close\n"
				 << "X-THINGSPEAKAPIKEY: R6Z98C0S5E3TWUSU\n"
				 << "Content-Type: application/x-www-form-urlencoded\n"
				 << "Content-Length:" << string(data.str()).length() << "\n\n";
			sc.send(string(head.str()));
			sc.send(string(data.str()));
			string rec = sc.receive(1024);
			//cout << "[" << rec << "]" << endl;
			cout << "Sending Done!!" << endl;
			doorDataSend = false;
			lightDataSend = false;
		}
		usleep(1000000);	//wait 1 second before continuing
		timeElapsed++;		//increase the counter
	}
}
