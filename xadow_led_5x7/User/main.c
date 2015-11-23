
#include "main.h"


unsigned char buffer[BUFFER_SIZE] = {0};
unsigned int MovingTime;
unsigned char sys_gpio_test = 0;


void copyArray(unsigned char source[],unsigned char destination[],unsigned char length);
void clearBuffer(unsigned char *buff, unsigned char size);


int main(void)
{
		unsigned char i;
		unsigned char str_len;
		char string_disp[10] = {0};
		
		//RCC_Configuration();
		delay_init();
		sys_gpio_init();
		I2C_Configuration();
		led_5x7_time_init();
		
		led_5x7_print_pic(6, 2);
		
		while(1)
		{
				if(flag_data_receive)
				{					
						copyArray(RxBuffer, buffer, BUFFER_SIZE);
						
						switch(buffer[0])
						{
								case DISP_CHAR_5X7:
										MovingTime = ((unsigned int)buffer[2]) << 8;
										MovingTime += buffer[3];
										led_5x7_print_ascii(buffer[1], MovingTime);
								
										if(MovingTime == 0)break;
								
										if(MovingTime != 0)
										{
												clearBuffer(buffer, BUFFER_SIZE);
												flag_data_receive = 0;
												clearBuffer(RxBuffer, I2C_BUFFER_MAX);
										}
								break;
								
								case DISP_STRING:
										str_len = buffer[1];
										for(i = 0;i < str_len;i ++)
										{
												string_disp[i] = (char)buffer[i+2];
										}
										MovingTime = ((unsigned int)buffer[2+str_len])<<8;
										MovingTime += buffer[3+str_len];
										led_5x7_print_string(string_disp, MovingTime);
										
										if(MovingTime == 0)break;
										
										clearBuffer(buffer, BUFFER_SIZE);
										flag_data_receive = 0;
										clearBuffer(RxBuffer, I2C_BUFFER_MAX);
								break;
								
								case SET_DISP_ORIENTATION:
										led_5x7_set_orientation(buffer[1]);
										clearBuffer(buffer, BUFFER_SIZE);
                    flag_data_receive = 0;
										clearBuffer(RxBuffer, I2C_BUFFER_MAX);
								break;
								
								case DISP_PIC:
										MovingTime = ((unsigned int)buffer[2]) << 8;
										MovingTime += buffer[3];
										led_5x7_print_pic(buffer[1], MovingTime);
								
										if(MovingTime == 0)break;
										
										if(buffer[1] == 7)
										{
												clearBuffer(buffer, BUFFER_SIZE);
												flag_data_receive = 0;
												clearBuffer(RxBuffer, I2C_BUFFER_MAX);
										}
										else if(MovingTime != 0)
										{
												clearBuffer(buffer, BUFFER_SIZE);
												flag_data_receive = 0;
												clearBuffer(RxBuffer, I2C_BUFFER_MAX);
										}
								break;
								
								case DISP_DATA:
										MovingTime = ((unsigned int)buffer[6]) << 8;
										MovingTime += buffer[7];
										led_5x7_print_data(&buffer[1], MovingTime);
								
										if(MovingTime == 0)break;
								
										if(MovingTime != 0)
										{
												clearBuffer(buffer, BUFFER_SIZE);
												flag_data_receive = 0;
												clearBuffer(RxBuffer, I2C_BUFFER_MAX);
										}
								break;
								
								default:
								break;
						}
				}
				
				GPIO_Test_for_TE();
		}
}

void sys_gpio_init(void)
{
		GPIO_InitTypeDef        GPIO_InitStructure;
	
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
		// For test
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure); // INT2
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits(GPIOA,GPIO_Pin_13 | GPIO_Pin_14); 
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOF, &GPIO_InitStructure);
		GPIO_SetBits(GPIOF,GPIO_Pin_0 | GPIO_Pin_1);
		RCC->CR &= 0xFFFEFFFF;
	  
		GPIO_SetBits(GPIOB,GPIO_Pin_1);
		GPIO_SetBits(GPIOA,GPIO_Pin_13 | GPIO_Pin_14); 
		GPIO_SetBits(GPIOF,GPIO_Pin_0 | GPIO_Pin_1);
}

void RCC_Configuration(void)
{
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);///*!< PLL input clock*12 */
		RCC_PLLCmd(ENABLE);
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){}
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource() != 0x08){}
		RCC->CR &= 0xFFFEFFFF;
}

void copyArray(unsigned char source[],unsigned char destination[],unsigned char length)
{
		unsigned char i;
		for(i = 0;i < length;i ++)
		{
				destination[i] = source[i];
		}
}

void clearBuffer(unsigned char *buff, unsigned char size)
{
		unsigned char i;
		for(i = 0;i < size;i++)
		{
				buff[i]=0x00;
		}
}

void GPIO_Test_for_TE(void)
{
		GPIO_InitTypeDef	GPIO_InitStructure;
	
		if(sys_gpio_test)
		{
				sys_gpio_test = 0;
				delay_ms(50);
			
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; // SCL, SDA output.
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
				GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			
				GPIO_SetBits(GPIOA,GPIO_Pin_9); // SCL
				GPIO_SetBits(GPIOA,GPIO_Pin_10); // SDA
			
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // INT2 input.
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
				GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
				GPIO_Init(GPIOA, &GPIO_InitStructure);

				while(1)
				{
						if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)) // Read INT2 port
						{
								GPIO_SetBits(GPIOA,GPIO_Pin_9); // SCL high
								GPIO_ResetBits(GPIOA,GPIO_Pin_10); // SDA high
						}
						else
						{
								GPIO_ResetBits(GPIOA,GPIO_Pin_9); // SCL low
								GPIO_SetBits(GPIOA,GPIO_Pin_10); // SDA high
						}
						
						delay_ms(100);
				}
		}
}
