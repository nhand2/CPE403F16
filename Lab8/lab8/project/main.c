#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "driverlib/eeprom.h"

int main (void)
{
	uint32_t pui32Data[2], pui32Read[2];
	pui32Data[0] = 0x12345678;
	pui32Data[1] = 0x56789abc;

	SysCtlClockSet (SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);	//sets the clock to 40 MHz

	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);	//enables the gpio port f
	GPIOPinTypeGPIOOutput (GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);	//sets GPIO.PF.1-3 as outputs
	GPIOPinWrite (GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);	//writes 0 to PF 1-3
	SysCtlDelay (20000000);	//delay of 20 seconds

	FlashErase (0x10000);	//erases the block of data at 0x10000
	FlashProgram (pui32Data, 0x10000, sizeof (pui32Data));	//programs the data to the start of the block
	GPIOPinWrite (GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);	//writes 0x02 to port F
	SysCtlDelay (20000000);

	SysCtlPeripheralEnable (SYSCTL_PERIPH_EEPROM0);	//enables the EEPROM
	EEPROMInit ();	//performs recovery if power failed
	EEPROMMassErase ();	//erases the EEPROM
	EEPROMRead (pui32Read, 0x0, sizeof (pui32Read));	//read the EEPROM memory at location 0x0 and store in puid32Read
	EEPROMProgram (pui32Data, 0x0, sizeof (pui32Data));	//write pui32Data contents to 0x0
	EEPROMRead (pui32Read, 0x0, sizeof (pui32Read));	//read the EEPROM memory at location 0x0 and store in puid32Read
	GPIOPinWrite (GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x04);	//writes 0x04 to port f
	while (1)
	{}
}
