
#include "main.h"
#include "font.h"
#include "led_5x7.h"
#include <string.h>


const unsigned char negative_pins[5] = {MATRIX_N0,MATRIX_N1,MATRIX_N2,MATRIX_N3,MATRIX_N4};
const unsigned char positive_pins[7] = {MATRIX_P0,MATRIX_P1,MATRIX_P2,MATRIX_P3,MATRIX_P4,MATRIX_P5,MATRIX_P6};

unsigned int CCR1_Val = 50000;
unsigned int PrescalerValue = 0;
unsigned int capture = 0;

unsigned char disp_orientation = 0;
unsigned char edge_bit_selector;

unsigned char display_buff[6];
unsigned long interruptions; // the number of timer1 interruptions to be set
unsigned long timer_count; //count the actual number of timer1 interruptions
unsigned char flag_timeout;


void led_5x7_time_init(void)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		TIM_OCInitTypeDef  TIM_OCInitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;

		/* TIM3 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

		/* Enable the TIM3 gloabal Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		/* Compute the prescaler value */
		PrescalerValue = (unsigned int)(SystemCoreClock  / 6000000) - 1;

		/* Time base configuration */
		TIM_TimeBaseStructure.TIM_Period = 65535;
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

		/* Prescaler configuration */
		TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

		/* Output Compare Timing Mode configuration: Channel1 */
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);
		TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);

		/* TIM Interrupts enable */
		TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
		/* TIM3 enable counter */
		//TIM_Cmd(TIM3, ENABLE);
}

void TIM3_Call_Back(void)
{
		if(TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) // 50ms
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
				capture = TIM_GetCapture1(TIM3);
				TIM_SetCompare1(TIM3, capture + CCR1_Val);
				timer_count ++;
				
				if(timer_count == interruptions)flag_timeout = 1;
		}
}

void led_5x7_write_pin_high_low(unsigned char pin, unsigned char value)
{
		switch(pin >> 4)
		{
				case 0: // PA
					if(value)GPIO_SetBits(GPIOA, 1 << (pin & 0x0f));
					else GPIO_ResetBits(GPIOA, 1 << (pin & 0x0f));
				break;
				case 1: // PB
					if(value)GPIO_SetBits(GPIOB, 1 << (pin & 0x0f));
					else GPIO_ResetBits(GPIOB, 1 << (pin & 0x0f));
				break;
				case 5: // PF
					if(value)GPIO_SetBits(GPIOF, 1 << (pin & 0x0f));
					else GPIO_ResetBits(GPIOF, 1 << (pin & 0x0f));
				break;
				default:
				break;
		}
}

void led_5x7_turn_off(void)
{
		unsigned char i;
		for(i=0;i<5;i++)led_5x7_write_pin_high_low(negative_pins[i], 1);
		for(i=0;i<7;i++)led_5x7_write_pin_high_low(positive_pins[i], 0);
}

void led_5x7_print(void)
{
		unsigned char bit_select;
		unsigned char i,j;
		unsigned int temp; // get the 1/5 code address for each asc;
		unsigned char count;
	
		for(count = 0;count < 5;count ++)
		{
				if(disp_orientation == RIGHT_TO_LEFT)
				{
						bit_select = 4;
						edge_bit_selector = 0x01; //always select the LSB of the display control, byte that is read from the array Font5x7
				}
				else 
				{
						bit_select = 0;
						edge_bit_selector = 0x40; //0x40 = 0b0100 0000,select bit 6 of display control, byte that is read from the array Font5x7						              
				}
				for(i=0;i<5;i++)// 5 bytes for each ASC Code;
				{
						temp = display_buff[i];//get the code 
					
						led_5x7_turn_off();//needed
					
						if(disp_orientation == RIGHT_TO_LEFT)led_5x7_write_pin_high_low(negative_pins[bit_select--],0);
						else led_5x7_write_pin_high_low(negative_pins[bit_select++],0);
					
						for(j = 0;j < 7;j ++)
						{
								if(temp & edge_bit_selector) 
								{
										led_5x7_write_pin_high_low(positive_pins[j], 1);
										delay_us(5);
										led_5x7_write_pin_high_low(positive_pins[j], 0);
								}
								else led_5x7_write_pin_high_low(positive_pins[j], 0);
							
								if(disp_orientation == RIGHT_TO_LEFT)temp >>= 1;
								else temp <<= 1;
						}
				}
		}
}

void led_5x7_print_with_time(unsigned int display_time, unsigned char time_flag)
{
	if(time_flag)
	{
			if(display_time < 50)display_time = 50;
			interruptions = display_time / 50;
	}
	else interruptions = display_time * 1000 / 50; // the number of timer1 interruptions
	
	timer_count = 0;
	flag_timeout = 0;
	TIM_Cmd(TIM3, ENABLE);
	while(!flag_timeout)led_5x7_print();
	TIM_Cmd(TIM3, DISABLE);
}

void led_5x7_print_ascii(unsigned char ascii, unsigned int display_time)
{
		unsigned char i;
		unsigned int temp;
	
		for(i = 0;i < 5;i ++)
		{
			temp = (ascii-32)*5+i; // get the 1/5 code address for each asc;
			display_buff[i]= Font5x7[temp]; //get the code 
		}
		
		led_5x7_print_with_time(display_time, 0);
}

void led_5x7_print_number(unsigned char number, unsigned int display_time)
{
		unsigned char units,tens;
		unsigned char tempL,tempH;
		unsigned char i;
	
		if(number > 99)number =99;
		
		units = number % 10;
		tens  = number / 10;
		
		for(i = 0;i < 5;i ++)
		{
				tempL =  units*5+i;
				tempL =  Seg[tempL];
				tempH =  tens*5+i;
				tempH =  Seg[tempH];
				tempH <<=4;
				display_buff[i] = tempH | tempL;
		}
		
		led_5x7_print_with_time(display_time, 0);
}

void led_5x7_print_string(const char *s, unsigned int moving_time)
{
		unsigned int i,j;
		unsigned char temp;
		unsigned int index;
		unsigned char currenr_seg_num = 5;
		unsigned int length = strlen(s);
		
		for(i = 0; i < length;)
		{
				if(currenr_seg_num == 5) // it's time to read 5 bytes of a new character
				{
						if(i < length)
						{
								for(j = 0;j < 5;j ++)
								{
										index = (s[i]-32)*5+j; // get the 1/5 code address for each asc;
										display_buff[j]= Font5x7[index]; // get the code 
								}
								display_buff[5] = 0x00;
						}
						currenr_seg_num = 0;
						i ++;
				}
				else
				{
						if(i < length)
						{
								index = (s[i]-32)*5+currenr_seg_num;
								temp = Font5x7[index];
						}
						else temp = 0x00;
						display_buff[0] = display_buff[1];
						display_buff[1] = display_buff[2];
						display_buff[2] = display_buff[3];
						display_buff[3] = display_buff[4];
						display_buff[4] = display_buff[5];
						display_buff[5] = temp;
						currenr_seg_num ++;
				}
				led_5x7_print_with_time(moving_time, 1); // print with the array display_buff
		}
}

void led_5x7_print_pic(unsigned char pic_num, unsigned int display_time)
{
	  unsigned char i;
		
		if(pic_num > 7)return;
		
		for(i = 0;i < 5;i ++)
		{
				display_buff[i] = Pic[i + pic_num * 5];
		}
		
		led_5x7_print_with_time(display_time, 0);
}

void led_5x7_print_data(unsigned char *buffer, unsigned int display_time)
{
	  unsigned char i;
		
		for(i = 0;i < 5;i ++)
		{
				display_buff[i] = *(buffer + i);
		}
		
		led_5x7_print_with_time(display_time, 0);
}

void led_5x7_set_orientation(unsigned char orientation)
{
		disp_orientation = orientation;
}
