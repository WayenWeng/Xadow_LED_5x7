

#ifndef __LED_5X7_H__
#define __LED_5X7_H__


#define MATRIX_N0 0x11 // PB1
#define MATRIX_N1 0x0d // PA13
#define MATRIX_N2 0x0e // PA14
#define MATRIX_N3 0x50 // PF0
#define MATRIX_N4 0x51 // PF1

#define MATRIX_P6 0x00 // PA0
#define MATRIX_P5 0x02 // PA2
#define MATRIX_P4 0x03 // PA3
#define MATRIX_P3 0x04 // PA4
#define MATRIX_P2 0x05 // PA5
#define MATRIX_P1 0x06 // PA6
#define MATRIX_P0 0x07 // PA7

/*Marco definitions for the display orientation of the LED matrix*/
#define RIGHT_TO_LEFT 0
#define LEFT_TO_RIGHT 1

/*Macro definitions of the control commands from the xadow main board*/
#define DISP_CHAR_5X7					0x80
#define DISP_STRING						0x81
#define SET_DISP_ORIENTATION 	0x82
#define POWER_DOWN						0x83
#define DISP_PIC							0x84
#define DISP_DATA							0x85


extern const unsigned char negative_pins[5];
extern const unsigned char positive_pins[7];


void led_5x7_time_init(void);
void TIM3_Call_Back(void);
void led_5x7_write_pin_high_low(unsigned char pin, unsigned char value);
void led_5x7_turn_off(void);
void led_5x7_print(void);
void led_5x7_print_with_time(unsigned int display_time, unsigned char time_flag);
void led_5x7_print_ascii(unsigned char ascii, unsigned int display_time);
void led_5x7_print_number(unsigned char number, unsigned int display_time);
void led_5x7_print_string(const char *s, unsigned int moving_time);
void led_5x7_print_pic(unsigned char pic_num, unsigned int display_time);
void led_5x7_print_data(unsigned char *buffer, unsigned int display_time);
void led_5x7_set_orientation(unsigned char orientation);


#endif
