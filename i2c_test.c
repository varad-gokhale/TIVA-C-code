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

	while((SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) == 0)												//until PLL has not reached stable frequency, spin
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
void init_i2c()
{
	SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;		//enable clock for i2c
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;	//enable clock for port B

	// select alternate function, drive current, enable digital function
	// set as open drain, enable pull-up
	GPIO_PORTB_DEN_R |= ((1 << 2) | (1 << 3));
	GPIO_PORTB_AFSEL_R |= ((1 << 2) | (1 << 3));
	GPIO_PORTB_DR2R_R |= ((1 << 2) | (1 << 3));
	GPIO_PORTB_ODR_R |= ((1 << 3));
	GPIO_PORTB_PUR_R |= ((1 << 2) | (1 << 3));

	//set pin mux for i2c
	GPIO_PORTB_PCTL_R |= (GPIO_PCTL_PB2_I2C0SCL | GPIO_PCTL_PB3_I2C0SDA);

	I2C0_MCR_R |= 0x00000010;													//master mode
	I2C0_MTPR_R |= 0x00000013;													//set clock

}

void set_device_address(uint8_t address){
	I2C0_MSA_R |= address;												//set slave address + write bit
}
void set_SR(uint8_t SR){
	if(SR == 1)
		I2C0_MSA_R |= 1;
	else
		I2C0_MSA_R &= ~1;
}
void send_data(uint8_t data){
	I2C0_MDR_R = data;															//actual data to be written
}
void start_i2c(){
	I2C0_MCS_R |= (I2C_MCS_STOP | I2C_MCS_START | I2C_MCS_RUN);
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
//	init_gpio();
	init_i2c();
	set_device_address(0x1C);
	set_SR(0);
	send_data(0x5A);
	start_i2c();
	while(1);
}
