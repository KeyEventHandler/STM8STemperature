#include "stdint.h"

void delay(uint16_t value){
  while(value != 0)
    value--;
}