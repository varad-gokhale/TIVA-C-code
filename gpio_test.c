#include "tm4c123gh6pm.h"
#include "stdint.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>

#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)

void init_clock()
{
	SYSCTL_RCC_R |= SYSCTL_RCC_BYPASS;
	SYSCTL_RCC_R &= ~SYSCTL_RCC_USESYSDIV;														//bypass PLL and don't use divider so that
	SYSCTL_RCC_R |= SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV;		//set OSC freq, specify OSCMAIN as sysclk and use divider
	SYSCTL_RCC_R &= ~SYSCTL_RCC_PWRDN;

	SYSCTL_RCC_R |= BIT25;
	SYSCTL_RCC_R &= ~(BIT23 | BIT24 | BIT26);

	while(SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS != 0)												//until PLL has not reached stable frequency, spin
		;
	SYSCTL_RCC_R &= ~SYSCTL_RCC_BYPASS;
}
void init_gpio()
{
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
	GPIO_PORTF_DIR_R |= 0x02;
	GPIO_PORTF_AFSEL_R &= ~0x02;
	GPIO_PORTF_DR8R_R |= 0x02;
	GPIO_PORTF_DEN_R |= 0x02;
}
void waitMicrosecond(uint32_t us)
{
	                                            // Approx clocks per us
	__asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*3
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             B    WMS_LOOP0");       // 1*3
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}

int main(void)
{
	init_clock();
	init_gpio();
	while(1){
		GPIO_PORTF_DATA_R |= 0x02;
		waitMicrosecond(1000000);
		GPIO_PORTF_DATA_R &= ~0x02;
		waitMicrosecond(1000000);
	}
}
