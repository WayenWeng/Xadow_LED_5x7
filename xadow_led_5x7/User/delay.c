
#include "main.h"
#include "delay.h"


static unsigned long  TimingDelay;


void delay_init(void)
{	  
	SysTick_Config(SystemCoreClock / 6000);
}

void Delay_Decrement(void)
{
  if(TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

void delay_ms(unsigned int nTime)
{
  TimingDelay = nTime;
  while(TimingDelay != 0);
}

void delay_us(unsigned int nTime)
{
  nTime *= 14;
	nTime /= 10;
  while(nTime -- );
}
