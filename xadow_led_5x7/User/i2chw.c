
#include "main.h"
#include "i2chw.h"


unsigned char TxBuffer[I2C_BUFFER_MAX] = {0};
unsigned char RxBuffer[I2C_BUFFER_MAX] = {0};

unsigned char i2c_comand = 0;
unsigned char i2c_byte_count = 0;

unsigned char scan_data[4] = {0,0,0,DEVICE_ADR};

unsigned char flag_data_receive = 0;

extern unsigned char flag_timeout;
extern unsigned int MovingTime;
extern unsigned char sys_gpio_test;


void I2C_Configuration(void)
{
		GPIO_InitTypeDef GPIO_InitStruct; 
		I2C_InitTypeDef I2C_InitStruct;
		NVIC_InitTypeDef NVIC_InitStructure;
	
		// Set SYSCLK as I2C clock source
		RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
		
		/* Enable GPIOA clock */
		//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); 
	
		/*!< sEE_I2C Periph clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 , ENABLE);
	
		/* Connect PXx to I2C_SCL*/
		GPIO_PinAFConfig(GPIOA , GPIO_PinSource9, GPIO_AF_4);
		/* Connect PXx to I2C_SDA*/
		GPIO_PinAFConfig(GPIOA ,GPIO_PinSource10, GPIO_AF_4);
	  /*!< GPIO configuration */ 
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;//GPIO_Mode_IN
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;//open-drain
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
		/*!< Configure sEE_I2C pins: SCL-PA9 */
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
		GPIO_Init(GPIOA , &GPIO_InitStruct);
		/*!< Configure sEE_I2C pins: SDA-PA10 */
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
		GPIO_Init(GPIOA , &GPIO_InitStruct);

		/* I2C configuration */
		I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
		I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
		I2C_InitStruct.I2C_DigitalFilter = 0x00;
		I2C_InitStruct.I2C_OwnAddress1 = (DEVICE_ADR << 1);
		I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
		I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; 
		I2C_InitStruct.I2C_Timing = 0xb0420f13;//100Kbits
	
		/* I2C Peripheral Enable */
		I2C_Cmd(I2C1, ENABLE);
		/* I2C Interrupt Enable */
		I2C_ITConfig(I2C1, I2C_IT_TXI | I2C_IT_RXI | I2C_IT_ADDRI | I2C_IT_STOPI ,ENABLE);
		//I2C_ITConfig(I2C1, I2C_IT_ADDRI | I2C_IT_STOPI ,ENABLE);
		/* Apply I2C configuration after enabling it */
		I2C_Init(I2C1, &I2C_InitStruct);
		
		/* Reconfigure and enable I2C1 error interrupt to have the higher priority */
		NVIC_InitStructure.NVIC_IRQChannel = I2C1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
} 

void I2C_Call_Back(void)
{
	  unsigned long I2CFlagStatus = 0;
	
		// Read I2C status registers (ISR)
    I2CFlagStatus = (unsigned long)(I2C1->ISR & (uint16_t)0x0000100FE);
	
		// If ADDR event
		if((I2CFlagStatus & I2C_ISR_ADDR) != 0)
		{
				I2C1->ICR |= I2C_ICR_ADDRCF;
		}
		// If TXIS event I2C_ISR_TXE
		else if((I2CFlagStatus & I2C_ISR_TXIS) != 0)
		{
				I2C1->ISR |= I2C_ISR_TXE;
				
				switch(i2c_comand)
				{
						case 0: // scan
								I2C1->TXDR = scan_data[i2c_byte_count];
								i2c_byte_count ++;
								if(i2c_byte_count >= 4)
								{
										i2c_byte_count = 0;
								}
						break;
						
						default:
						break;
				}
		}
		// If RXNE event
		else if((I2CFlagStatus & I2C_ISR_RXNE) != 0)
		{
				I2C1->ISR |= I2C_ISR_TXE;

				RxBuffer[i2c_byte_count ++] = (unsigned char)I2C1->RXDR;
				switch(RxBuffer[0])
				{
						case 0:
								if(i2c_byte_count >= 1)
								{	
										i2c_comand = RxBuffer[0];
										i2c_byte_count = 0;
								}
						break;
						
						case DISP_CHAR_5X7:case DISP_PIC:
								flag_timeout = 1;
								MovingTime = 0;
								if(i2c_byte_count >= 4)
								{	
										i2c_byte_count = 0;
										flag_data_receive = 1;
								}
						break;
						
						case DISP_STRING:
								flag_timeout = 1;
								MovingTime = 0;
								if(i2c_byte_count >= (RxBuffer[1] + 4))
								{	
										i2c_byte_count = 0;
										flag_data_receive = 1;
								}
						break;
						
						case SET_DISP_ORIENTATION:
								if(i2c_byte_count >= 2)
								{	
										i2c_byte_count = 0;
										flag_data_receive = 1;
								}
						break;
						
						case DISP_DATA:
								flag_timeout = 1;
								MovingTime = 0;
								if(i2c_byte_count >= 8)
								{
										i2c_byte_count = 0;
										flag_data_receive = 1;
								}
						break;
								
						case GPIO_TEST_CMD:
							sys_gpio_test = 1;
							flag_timeout = 1;
							MovingTime = 0;
						break;
						
						default:
						break;
				}
		}
		// If NACK event
		else if((I2CFlagStatus & I2C_ISR_NACKF) != 0)
		{
				// Clear NACK flag
				I2C1->ICR |= I2C_ICR_NACKCF;
		}
		// If STOP event
		else if((I2CFlagStatus & I2C_ISR_STOPF) != 0)
		{
				// Clear STOP flag
				I2C1->ICR |= I2C_ICR_STOPCF;
				i2c_byte_count = 0;
		}
}
