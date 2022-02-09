#include <Arduino.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include <driver/dac.h>
#define repeat 10
#define samples 44100
const int adcPin = 32;
const int adc1Pin = 27;
const int dac1Pin = 25;
const int dac2Pin = 26;
const int clean_toggle = 16;
const int prev_effect = 4;
const int next_effect = 17;
const int gpioPOT0 = 36; // Volumen
const int gpioPOT1= 39; // Freq
const int gpioPOT2 = 34; // Algo qsy
int POT0,POT1, POT2, in_ADC0, in_ADC1, out_DAC0, out_DAC1;
int count = 0, sample=0;
int selector = 0;
int LFO = 0;
float Y0 = X0, Y1 = X1;
float alpha = 0.13;
float X0 = 0.0, X1 = 0.0;
uint16_t nSineTable[samples];


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

void sinewaveGenerator(){
  for(uint32_t nIndex=0; nIndex<44100; nIndex++)
  {
    float arg = ((2.0*PI)/samples)*nIndex;
    // normalised to 12 bit range 0-4095
    nSineTable[nIndex] = (uint16_t)  (((1+sin(arg))*4095.0)/2);
  }
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
  sinewaveGenerator();
  
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
  //  for (float deg=0; deg <360; deg++){
  //    filtering();
  //   float argSin = deg*PI/180;
  //   Y0=int(128 + (Y0/32)*(sin(argSin)));
  //   Y1=int(128 + (Y1/32)*(sin(argSin)));
  //   // Serial.println(Y0);
  //   dacWrite(dac1Pin,Y0);
  //   dacWrite(dac2Pin,Y1);
    
  //   //Serial.println(analogRead(adc1Pin)); 
    
  // }
  filtering();
  in_ADC0=Y0;
  in_ADC1=Y1;
 POT2 = POT2>>1; //divide value by 2 (its too big) 
 count++; 
 if (count>=160) //160 chosen empirically
 {
   count=0;
   sample=sample+POT2;
   if(sample>=samples) sample=0;
 }

  LFO=map(nSineTable[sample],0,4095,(4095-POT1),4095); // maneja la frecuencia con el POT1
  out_DAC0=map(in_ADC0,1,4095,1, LFO);
  out_DAC1=map(in_ADC1,1,4095,1, LFO);
 
  //Add volume feature with POT0
  out_DAC0=map(out_DAC0,1,4095,1, POT0);
  out_DAC1=map(out_DAC1,1,4095,1, POT0);

  dacWrite(dac1Pin,out_DAC0);
  dacWrite(dac2Pin,out_DAC1);
}

void distortion(){
  filtering();
  if((Y0/16) < 127){
    Serial.println(127);
  }else{
    Serial.print(Y0);
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


