#include "iostm8s103f3.h"
#include "stdint.h"
#include "delay.c"
#define MOSI    6
#define SCK     5
#define CS      4

void SPISendByte(uint8_t data){
  uint8_t mask;
  for(mask = 0x80; mask; mask >>=1){
    if(data & mask)
      PD_ODR |= (1<<MOSI);
    else
      PD_ODR &=~(1<<MOSI);
    delay(4);
    PD_ODR |= (1<<SCK);
    delay(12);
    PD_ODR &=~(1<<SCK);
    delay(2);
    PD_ODR &=~(1<<MOSI);
    delay(2);    
  }
}

void SPISendData(uint8_t regAddr, uint8_t data){
  delay(4);
  PD_ODR &=~(1<<CS);
  SPISendByte(regAddr);
  SPISendByte(data);
  PD_ODR |= (1<<CS);
  delay(4);
}

void initMAX7219(){
  PD_DDR |= (1<<MOSI) | (1<<SCK) | (1<<CS);
  PD_CR1 |= (1<<MOSI) | (1<<SCK) | (1<<CS);
  PD_ODR |= (1<<MOSI) | (1<<CS);
  PD_ODR &=~(1<<SCK);   //set SCK to low otherwise first clock isn't generated
  
  SPISendData(0x0B,0x0F);               //Scan Limit
  delay(2);
  SPISendData(0x0A,0x0F);                //Max Intensity
  delay(2);
  SPISendData(0x0C,0x01);               //Normal Operation Mode
  delay(2);
  SPISendData(0x09,0xFF);               //Decode
  delay(2);
  SPISendData(0x0F,0x00);               //Normal Operation Mode
}