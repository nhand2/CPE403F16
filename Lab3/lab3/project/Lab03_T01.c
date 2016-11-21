//The purpose of this program is to alternated between the three available colors on the TIVA C board
//The lights will change every 50 milliseconds

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"


int main(void)
{
	uint8_t ui8LED = 2;
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);	//set the clock to be 40 MHz
	// (Uses PLL = 400 MHz) => 400MHz / (2 * 5) = 40 MHz
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	//Enable the GPIO PORTF pins
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);	//Set pin 1, 2,3 at PORT F at outputs
	while(1)
	{
		// Turn on the LED
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
		// Delay for a bit
		SysCtlDelay(2000000);
		// Cycle through Red, Green and Blue LEDs
		if (ui8LED == 8) {ui8LED = 2;} else {ui8LED = ui8LED*2;}
	}
}
