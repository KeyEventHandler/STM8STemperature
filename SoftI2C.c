#include "iostm8s103f3.h"
#include "stdint.h"
#include "stdbool.h"

#define SCL 3
#define SDA 2
#define SDO 1

void initI2C(){
  //init C7 as 3.3V supply
  PC_DDR |= (1<<7);
  PC_CR1 |= (1<<7);
  PC_ODR |= (1<<7);

  //init C6 as 3.3V supply
  PC_DDR |= (1<<6);
  PC_CR1 |= (1<<6);
  PC_ODR |= (1<<6);
  
  //init C5 as GND supply
  PC_DDR |= (1<<5);
  PC_CR1 |= (1<<5);
  PC_ODR &=~(1<<5);

  //init I2C Pins
  PD_DDR |= (1<<SCL) | (1<<SDA) | (1<<SDO);
  PD_CR1 &=~((1<<SCL) | (1<<SDA));        //Set as pseudo open-drain
  PD_CR1 |= (1<<SDO);
  PD_ODR |= (1<<SCL) | (1<<SDA) | (1<<SDO);
}

void I2CStart(){
  PD_ODR &=~(1<<SDA);
  delay(1);
  PD_ODR &=~(1<<SCL);
}

void I2CStop(){
  PD_ODR |= (1<<SCL);
  delay(1);
  PD_ODR |= (1<<SCL);
}

void I2CMasterAck(){
  PD_ODR &=~(1<<SDA);
  delay(2);
  PD_ODR |= (1<<SCL);
  delay(2);
  PD_ODR &=~(1<<SCL);
  PD_ODR |= (1<<SDA);
}

void I2CMasterNAck(){
  PD_ODR |= (1<<SDA);
  delay(2);
  PD_ODR |= (1<<SCL);
  delay(2);
  PD_ODR &=~(1<<SCL);
}

void I2CSendData(uint8_t data){
  uint8_t mask;
  for(mask = 0x80; mask; mask >>=1){
    if(data & mask)
      PD_ODR |= (1<<SDA);
    else
      PD_ODR &=~(1<<SDA);
    delay(1);
    PD_ODR |= (1<<SCL);
    delay(3);
    PD_ODR &=~(1<<SCL);
    delay(1);
    PD_ODR |= (1<<SDA);
    delay(1);    
  }
  PD_ODR |= (1<<SDA);
}

uint8_t I2CReadByte(){
  uint8_t counter;
  uint8_t readByte = 0;
  for(counter = 0; counter < 8; counter++){
    PD_ODR |= (1<<SCL);
    delay(4);
    uint8_t readValue = (PD_IDR & (1<<SDA));
    readByte |=  (readValue >> SDA) << (7 - counter);
    PD_ODR &=~(1<<SCL);
    delay(4);
  }
  return readByte;
}

bool I2CSendDataWithSlaveAck(uint8_t data){
  uint8_t slaveAck;
  I2CSendData(data);
  delay(2);
  PD_ODR |= (1<<SCL);
  delay(10);
  slaveAck = PD_IDR & (1<<SDA);
  PD_ODR &=~ (1<<SCL);
  delay(2);
  return slaveAck == 0;
}