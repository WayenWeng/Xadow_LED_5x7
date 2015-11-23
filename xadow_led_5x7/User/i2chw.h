
#ifndef __I2CHW_H__
#define __I2CHW_H__


#define DEVICE_ADR      		33
#define DEVICE_ADR_WRITE    (I2C_DEVICE_ADR << 1)
#define DEVICE_ADR_READ     ((I2C_DEVICE_ADR << 1) + 1)

#define I2C_BUFFER_MAX			16

#define GPIO_TEST_CMD						0x54 // 'T'


extern unsigned char TxBuffer[I2C_BUFFER_MAX];
extern unsigned char RxBuffer[I2C_BUFFER_MAX];
extern unsigned char i2c_comand;
extern unsigned char flag_data_receive;


void I2C_Configuration(void);
void I2C_Call_Back(void);


#endif
