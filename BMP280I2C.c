#include "iostm8s103f3.h"
#include "stdint.h"
#include "SoftI2C.c"

static uint8_t BMP280_SLAVE_WRITE = 0xEE;        //0b11101110
static uint8_t BMP280_SLAVE_READ = 0xEF;        //0b11101111

static uint32_t dig_T1,dig_T2,dig_T3;

uint32_t readBMP280(uint8_t regAddr, int8_t amount){
  uint32_t result = 0;
  I2CStart();
  if(I2CSendDataWithSlaveAck(BMP280_SLAVE_WRITE)){
    if(I2CSendDataWithSlaveAck(regAddr)){
      PD_ODR |= (1<<SCL);
      delay(4);
      I2CStart();
      if(I2CSendDataWithSlaveAck(BMP280_SLAVE_READ)){
        while(amount > 0){
          uint32_t byteRead = I2CReadByte();
          result |= byteRead << ((amount-1) * 8);
          amount --;
          if(amount == 0)
            I2CMasterNAck();
          else{
            I2CMasterAck();
            PD_ODR |= (1<<SDA);
          }
        }
      }
    }
  }
  I2CStop();
  return result;
}

void initBMP280(){
  I2CStart();
  if(I2CSendDataWithSlaveAck(BMP280_SLAVE_WRITE)){
    if(I2CSendDataWithSlaveAck(0xF4)){
      if(I2CSendDataWithSlaveAck(0x23)){
      }
    }
  }
  I2CStop();
  
  uint8_t dig_T1_1 = readBMP280(0x89,1);
  uint8_t dig_T1_2 = readBMP280(0x88,1);
  
  uint8_t dig_T2_1 = readBMP280(0x8B,1);
  uint8_t dig_T2_2 = readBMP280(0x8A,1);
  
  uint8_t dig_T3_1 = readBMP280(0x8D,1);
  uint8_t dig_T3_2 = readBMP280(0x8C,1);
  
  dig_T1 = (dig_T1_1 << 8) | dig_T1_2;
  dig_T2 = (dig_T2_1 << 8) | dig_T2_2;
  dig_T3 = (dig_T3_1 << 8) | dig_T3_2;
}

int32_t getBMP280Temperature(){
  if(dig_T1 != 0 && dig_T2 != 0 && dig_T3 != 0){
    uint32_t T = readBMP280(0xFA,3) >> 4;
    int32_t part1 = (T >> 3) - (dig_T1 << 1);
    int32_t var1 = (part1 * dig_T2) >> 11;
    int32_t part2 = (T >> 4) - dig_T1;
    int32_t var2 = (((part2 * part2) >> 12) * dig_T3) >> 14;
    int32_t t_fine = var1 + var2;
    int32_t calc = (t_fine * 5 + 128) >> 8;
    return calc;
  } else
    return 0;
}

