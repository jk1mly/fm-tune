/* 
 * FM tuner for PIC12F1572
 *
 *      JA1YTS:Toshiba Amature Radio Station
 *      JK1MLY:Hidekazu Inaba
 *
 *  (C)2021 JA1YTS,JK1MLY All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation and/or 
 * other materials provided with the distribution.
*/

#include <stdio.h>
//#include <c90/stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <pic.h>
#include <pic12F1572.h>

#define _XTAL_FREQ 4000000
// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection
#pragma config WDTE = OFF       // Watchdog Timer
#pragma config PWRTE = OFF      // Power-up Timer
#pragma config MCLRE = OFF      // MCLR Pin Function Select
#pragma config CP = OFF         // Flash Program Memory Code Protection
#pragma config BOREN = ON       // Brown-out Reset Enable
#pragma config CLKOUTEN = OFF   // Clock Out Enable
// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection
#pragma config PLLEN = OFF      // 4x PLL OFF
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable
#pragma config BORV = LO        // Brown-out Reset Voltage Selection
#pragma config LPBOREN = OFF    // Low Power Brown-out Reset Enable
#pragma config LVP = OFF        // Low-Voltage Programming Enable

#define	I2C_SDA_LOW     LATA5 = 0;TRISA5 = 0    // 0
#define I2C_SDA_HIGH	LATA5 = 1;TRISA5 = 1    // Z(1))
#define	I2C_SCK_LOW     LATA4 = 0
#define	I2C_SCK_HIGH	LATA4 = 1
#define	LED_ON      	LATA0 = 0
#define	LED_OFF     	LATA0 = 1
#define	DBG_SW    		RA1 

#define	CHK_TFM_H       0x0A
#define	CHK_TFM_L       0x18
#define	CHK_NHK_H       0x10
#define	CHK_NHK_L       0x58

// I2C mode CHAN[9:0] BAND=2 CHAN = (Freq - 76.0M) / Ch_s
// TFM Freq=80.0 BAND=2 Ch_s=100k CHAN=40 0x28 00 0010 1000
#define	TUN_ADR         0x20
#define	TUN_I2C         0x22
#define	REG02_H         0xE0
#define	REG02_L         0x01
#define	REG03_H         0x0A
#define	REG03_L         0x18
#define	TFM03_H         0x0A
#define	TFM03_L         0x18
#define	REG04_H         0x08
#define	AFC04_H         0x09
#define	REG04_L         0x80
#define	REG05_H         0x88
#define	REG05_L         0x84

#define false           0
#define true            1

void port_init(void) {
    /* CONFIGURE GPIO */ 
    OSCCON  = 0b01101000;    //4MHz
    TRISA   = 0b00101110;    //Input(1)
    OPTION_REG = 0b00000000; //MSB WPUENn
    WPUA    = 0b00101010;    //PupOn(1)
    INTCON  = 0b00000000;
    LATA    = 0b00010000;
    ANSELA  = 0b00000100;    //ANSA RA2
	ADCON0  = 0b00001001;    //CHS00010 AN2
	ADCON1  = 0b10010000;    //ADFM ADCS001 2us
	ADCON2  = 0b00000000;

    LED_ON;
    __delay_ms(100);
    LED_OFF;    
    __delay_ms(100);
    LED_ON;
    __delay_ms(100);
    LED_OFF;    
    __delay_ms(100);
}

uint8_t adconv(void){
	uint16_t temp;
	uint8_t ret;
    temp = 0;
    for (uint8_t i = 0; i < 7; i++){
        ADCON0bits.ADGO = 1;
        while(ADCON0bits.ADGO) ;
        temp += ADRES;
    }
    ret =  (uint8_t)255 - (uint8_t)(temp >> 5);
    return (ret);    
}

void i2c_snd(uint8_t data)
{
	for(uint8_t i = 0; i < 8; i++){
		if ( (data & 0x80) ==0  ){
			I2C_SDA_LOW;
		}else{
			I2C_SDA_HIGH;
		}
		
		data = (uint8_t)(data << 1) ;
		__delay_us(2);
		I2C_SCK_HIGH;
		__delay_us(4);
		I2C_SCK_LOW;
		__delay_us(2);
	}

// ACK    
	I2C_SDA_HIGH;
	__delay_us(6);
    I2C_SCK_HIGH;
    __delay_us(4);
	I2C_SCK_LOW;
	I2C_SDA_LOW;
	__delay_us(10);
}
    
void tun_chk(void)
{
	uint8_t data;
// Start
	I2C_SCK_HIGH;
	I2C_SDA_HIGH;
	__delay_us(10);
	I2C_SDA_LOW;
	__delay_us(6);
	I2C_SCK_LOW;
	__delay_us(2);
// 
	data = (uint8_t)(TUN_ADR & 0xFE );
    i2c_snd(data);	
// 
	data = (uint8_t)(REG02_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(REG02_L & 0xFF );
    i2c_snd(data);	
// 
	data = (uint8_t)(TFM03_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(TFM03_L & 0xFF );
    i2c_snd(data);	
// 
	data = (uint8_t)(AFC04_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(REG04_L & 0xFF );
    i2c_snd(data);	
// 
	data = (uint8_t)(REG05_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(REG05_L & 0xFF );
    i2c_snd(data);	
// Stop            
	I2C_SDA_LOW;
	__delay_us(2);
	I2C_SCK_HIGH;
	__delay_us(6);	
// Wait
	I2C_SDA_HIGH;
	__delay_us(20);
}

void tun_adc(uint8_t ret) {
	uint8_t data;
// Start
	I2C_SCK_HIGH;
	I2C_SDA_HIGH;
	__delay_us(10);
	I2C_SDA_LOW;
	__delay_us(6);
	I2C_SCK_LOW;
	__delay_us(2);
// 
	data = (uint8_t)(TUN_ADR & 0xFE );
    i2c_snd(data);	
// 
	data = (uint8_t)(REG02_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(REG02_L & 0xFF );
    i2c_snd(data);	
// 
	data = (uint8_t)((ret & 0xFC ) >> 2);
    i2c_snd(data);	
// 	
	data = (uint8_t)(((ret & 0x03 ) << 6)|(REG03_L & 0x3F ));
    i2c_snd(data);	
// 
	data = (uint8_t)(REG04_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(REG04_L & 0xFF );
    i2c_snd(data);	
// 
	data = (uint8_t)(REG05_H & 0xFF );
    i2c_snd(data);	
// 	
	data = (uint8_t)(REG05_L & 0xFF );
    i2c_snd(data);	
// Stop            
	I2C_SDA_LOW;
	__delay_us(2);
	I2C_SCK_HIGH;
	__delay_us(6);	
// Wait
	I2C_SDA_HIGH;
	__delay_us(20);
}

void main(void) {

//Initialize
	port_init();
    LED_ON;
	__delay_ms(1000);
    LED_OFF;

    while(DBG_SW == 0){
    	tun_chk();
    	__delay_ms(1000);
        LED_ON;
    	__delay_ms(1000);
        LED_OFF;
    	__delay_ms(8000);
    }
 
    uint8_t freq;
    uint8_t buf2;
    uint8_t flag = false;

    freq = adconv();
    buf2 = freq;
    tun_adc(freq);
    
//Loop    
    while(1){
        uint8_t buf1;
        buf1 = adconv();
        if((freq != buf1) && (buf1 == buf2) && (flag == true)){
                LED_ON;
                freq = buf1;
                tun_adc(freq);
        		__delay_ms(100);
                LED_OFF;
        } else {
            if(buf1 == buf2){
                flag = true;
            } else {
                flag = false;
                buf2 = buf1;
            }
        }
		__delay_ms(10);
    }
}

