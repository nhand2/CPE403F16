#include <stdint.h>
#include <stdbool.h>
#include "utils/ustdlib.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/hibernate.h"
#include "driverlib/gpio.h"

#ifdef DEBUG
void__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

int main (void)
{
	SysCtlClockSet (SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);	//set the system clock to 40 MHz
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);	//enables port F, which is connected to the LEDs
	GPIOPinTypeGPIOOutput (GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);	//set GPIO.PF1, 2, 3 as outputs
	GPIOPinWrite (GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);	//writes 1 to pin 3 of the gpio (green led)


	SysCtlPeripheralEnable (SYSCTL_PERIPH_HIBERNATE);	//Enables hibernate mode on Tiva C
	HibernateEnableExpClk (SysCtlClockGet());	//Sets the clock to be the same as the System clock (40MHz)
	HibernateGPIORetentionEnable ();	//Keeps the hibernate pin enabled and maintained during hibernation
	SysCtlDelay (64000000);	//delay of 4 seconds
	HibernateRTCSet (0);	//reset the RTC to 0
	HibernateRTCEnable ();	//enables the RTC
	HibernateRTCMatchSet (0, 5);	//sets wake up time to 5 seconds
	HibernateWakeSet (HIBERNATE_WAKE_PIN | HIBERNATE_WAKE_RTC);	//sets the wake condition of the wake pin (sw2 and RTC)
	GPIOPinWrite (GPIO_PORTF_BASE, GPIO_PIN_3, 0x00);	//turn off the green led before going to sleep

	HibernateRequest ();
	while (1)
	{};
}
