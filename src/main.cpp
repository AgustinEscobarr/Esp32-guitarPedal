#include <Arduino.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include <driver/dac.h>
#define repeat 10
const int adcPin = 32;
const int adc1Pin = 27;
const int dac1Pin = 25;
const int dac2Pin = 26;
const int clean_toggle = 16;
const int prev_effect = 4;
const int next_effect = 17;
// const int POT0 = 36; // Volumen
float X0 = 0.0, X1 = 0.0;
float Y0 = X0, Y1 = X1;
float alpha = 0.13;
int selector = 0;


void clean(){
  selector = 0;
  delay(100);
}
void prev(){
  if(selector>0){
    selector-=1;
  }else{
    selector = 3;
  }
  delay(100);
}
void next(){
  if(selector<3){
    selector+=1;
  }else{
    selector=0;
  }
  delay(100);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(clean_toggle, INPUT_PULLUP); // se utiliza el modo pull up para poder usar el boton.
  attachInterrupt(clean_toggle, clean, FALLING);
   pinMode(prev_effect, INPUT_PULLUP); 
  attachInterrupt(prev_effect, prev, FALLING);
   pinMode(next_effect, INPUT_PULLUP); 
  attachInterrupt(next_effect, next, FALLING);
  // analogSetAttenuation(ADC_0db);
  dac_output_enable(DAC_CHANNEL_1);  // se habilita del DAC 1 (GPIO25)
  dac_output_enable(DAC_CHANNEL_2); // se habilita el DAC 2 (GPIO26)
  
}
void serialPrinter(){
  // Serial.print(Y);
  // Serial.print(",");
  Serial.println(Y0);
}

void filtering(){ // Filtro FIR, donde se toman una cantidad determinada de muestras para obtener el valor medio móvil (se obtiene un solo valor).
   for(int i=0;i<repeat;i++){
     X0 = (float) analogRead(adcPin); // Valor de la entrada del ADC.
     Y0 = (alpha*X0) + ((1-alpha)*Y0); // Y es el acumulador, donde se obtiene el valor medio de las muestras. Alpha es el valor de suavizado.  
     X1 = (float) analogRead(adc1Pin); 
     Y1 = (alpha*X1) + ((1-alpha)*Y1); 
   }
  
  
  // literalmente ese comando es lo mismo que dac_output_voltage y  dac_output_enable al mismo tiempo, sospecho que hace todo lo mismo.
   
}
void tremolo(){
   for (float deg=0; deg <360; deg++){
     filtering();
    float argSin = deg*PI/180;
    Y0=int(128 + (Y0/32)*(sin(argSin)));
    Y1=int(128 + (Y1/32)*(sin(argSin)));
    // Serial.println(Y0);
    dacWrite(dac1Pin,Y0);
    dacWrite(dac2Pin,Y1);
    //Serial.println(analogRead(adc1Pin)); 
    delay(8);
  }
}

void loop() {
  
  switch (selector)
  {
  case 0:
    filtering();
    dacWrite(dac1Pin,Y0);
    dacWrite(dac2Pin,Y1);
    break;
  case 1:
    tremolo();
    break;
  default:
    break;
  }
  // put your main code here, to run repeatedly:
  
   
 }


