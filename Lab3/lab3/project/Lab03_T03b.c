//This program will blink the LEDs in a different sequence and will light more than one at a time
//The base 50 ms delay will be used

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

int main(void)
{
	uint8_t ui8LED = 0;	//output pin data
	uint8_t colorValues[3] = { 6, 12, 10 };	//array to hold the pattern of the output values
	int i = 0;	//index of the array
	SysCtlClockSet(SYSCTL_SYSDIV_33|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);	//set the clock to be 40 MHz
	//Set clock to 6 MHz => 400MHz / (33*2) = 6MHz
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	//Enable the GPIO PORTF pins
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);	//Set pin 1, 2,3 at PORT F at outputs
	while(1)
	{
		for (i = 0; i < 3; i++)
		{
			ui8LED = colorValues[i];	//Assign the value of the pin to output
			// Turn on the LED
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
			// Delay for a bit
			SysCtlDelay(2000000);
		}
		i = 0;
	}
}
