#include "MAX7219SPI.c"
#include "BMP280I2C.c"
#include "SSD1306.c"
#include "stdbool.h"
#define H 0x0C
#define E 0x0B
#define L 0x0D
#define P 0x0E
#define S 0x0A
#define N 0x0F

bool didInitBMP280 = false;
volatile uint16_t tim4Time;
volatile uint8_t memoryIndex = 0;
uint8_t buffer[] = {
  N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N
};
uint8_t storage[85];

void applyBuffer(uint8_t amount){
  uint8_t counter;
  for(counter = 0; counter < amount; counter++){
    if((buffer[counter] & 0x80) == 1){
      SPISendData((counter+1)|0x80,buffer[counter]);    //check whether data has a decimal point
    }
    else{
      SPISendData(counter+1,buffer[counter]);
    }
  }
}

void shuffleRight(){
  int8_t counter;
  uint8_t temp;
  for(counter = sizeof(buffer)-1; counter >= 0; counter--){
    if(counter != sizeof(buffer) - 1){
      uint8_t temp2 = buffer[counter];
      buffer[counter] = temp;
      temp = temp2;
    } else {
      temp = buffer[counter];
      buffer[counter] = buffer[0];
    }
  }  
}

void shuffleLeft(uint8_t *buf, uint8_t len){
  uint8_t counter, temp;
  for(counter = 0; counter < len; counter++){
    if(counter != 0){
      uint8_t temp2 = buf[counter];
      buf[counter] = temp;
      temp = temp2;
    } else {
      temp = buf[counter];
      buf[counter] = buf[len-1];
    }
  }
}

void addNewMeasurement(int32_t temperature){
  shuffleLeft(storage,sizeof(storage));
  storage[0] = (uint8_t)(temperature / 100);
}

void updateTemperature(){
  int8_t counter = 0;
  int32_t cpyTmp = getBMP280Temperature();
  addNewMeasurement(cpyTmp);
  
  bool isNegative = cpyTmp < 0;  
  if(isNegative)                              //temperature is negative
    cpyTmp *= -1;
  
  while(cpyTmp > 0 || counter < 4){
    if(cpyTmp > 0){
      buffer[11+counter] = cpyTmp % 10;
      cpyTmp /= 10;
    }else{
      buffer[11+counter] = 0x00;                              //fill up to 4 digits        
    }
    
    if(counter == 2){
      buffer[11+counter] |= (0x80 | buffer[11+counter]);        //add digit point
    }
    counter++;
  }
  
  if(isNegative)
    buffer[11+counter] = S;                                //add sign for negative
  else
    buffer[11+counter] = N;
}

void displayGraph(){
  uint8_t counter;
  lcd_clrscr(false);
  lcd_gotoxy(0,0);
  lcd_puts("Last 85 measurements:");
  lcd_gotoxy(0,1);
  lcd_puts("1°C / Pixel");
  lcd_gotoxy(0,3);
  lcd_puts("23°C");
  for(counter = 0; counter < sizeof(storage); counter++){
    lcd_drawPixel(28+counter,51-storage[counter]);
  }
  lcd_display();
}

#pragma vector = TIM4_OVR_UIF_vector
__interrupt void TIM4_UPD_OVF_IRQHandler(void){
  if(tim4Time % 80 == 0){
    initMAX7219();
    applyBuffer(8);
    shuffleLeft(buffer,sizeof(buffer));
  }
  if(tim4Time == 640 || tim4Time == 0){         //*8
    if(!didInitBMP280){
      initBMP280();
      lcd_init();
      didInitBMP280 = true;
    }
    updateTemperature();
    displayGraph();
    tim4Time = 0;
  }
  tim4Time++;
  TIM4_SR &=~(1<<0);     //Clear Update interrupt flag
}

void initTimer(){
  CLK_PCKENR1 |= (1<<4); //Enable Clock for Timer 4
  TIM4_PSCR = (0x0F);    //Prescaler, Divide Clock by Max
  TIM4_IER = 1;          //Enable Update Interrupt
  TIM4_CR1 |= 1;         //Enable Timer
  asm("rim");            //Enable Interrupts        
}

int main(){
  CLK_CKDIVR = 0x00;
  initTimer();
  initI2C();
  while(1){
  }
}