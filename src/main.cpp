#include <Arduino.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include <driver/dac.h>
#define repeat 10
#define samples 44100
#define MAX_DELAY 20000
const int adcPin = 32;
const int adc1Pin = 27;
const int dac1Pin = 25;
const int dac2Pin = 26;
const int clean_toggle = 16;
const int prev_effect = 4;
const int next_effect = 17;
const int led_power = 13;
const int led_clean=5;
const int led_tremolo=18;
const int led_distortion=19;
const int led_delay = 3;
const int led_OVERLOAD = 12;
const int gpioPOT0 = 36; // Volumen
const int gpioPOT1= 39; // Freq            // POT 2 ahora, POT 1 inutilizado
const int gpioPOT2 = 34; // Algo qsy config delay 
int POT0=4095,POT1 = 2045, POT2, in_ADC0, in_ADC1, out_DAC0, out_DAC1, upper_threshold, lower_threshold;
int count = 0, sample=0;
int selector = 0;
int LFO = 0;
float alpha = 0.13;
float X0 = 0.0, X1 = 0.0;
float Y0 = X0, Y1 = X1;

uint16_t sDelayBuffer0[MAX_DELAY];
uint16_t sDelayBuffer1[MAX_DELAY];
unsigned int DelayCounter = 0;
unsigned int Delay_Depth = MAX_DELAY;
uint16_t nSineTable[samples];
hw_timer_t *timer= NULL ,*timer1=NULL;

void ledEffectsControl();
void clean();
void prev();
void next();
void sinewaveGenerator();
void filtering();
void tremolo();
void distortion();
void delayEffect();
void timerInit();
void timer1Init();



void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  pinMode(led_power, OUTPUT);
  pinMode(led_clean, OUTPUT);
  pinMode(led_tremolo, OUTPUT);
  pinMode(led_distortion, OUTPUT);
  pinMode(led_delay, OUTPUT);
  
  pinMode(led_power, OUTPUT);
  pinMode(led_OVERLOAD, OUTPUT);

  pinMode(clean_toggle, INPUT); // se utiliza el modo pull up para poder usar el boton.
  attachInterrupt(clean_toggle, clean, FALLING);
   pinMode(prev_effect, INPUT); 
  attachInterrupt(prev_effect, prev, FALLING);
   pinMode(next_effect, INPUT); 
  attachInterrupt(next_effect, next, RISING);

  analogSetPinAttenuation(adcPin,ADC_0db);
  analogSetPinAttenuation(adc1Pin,ADC_0db);
  dac_output_enable(DAC_CHANNEL_1);  // se habilita del DAC 1 (GPIO25)
  dac_output_enable(DAC_CHANNEL_2); // se habilita el DAC 2 (GPIO26)

  sinewaveGenerator();
  
}



void loop() {
  // POT0=analogRead(gpioPOT0);
  // POT1=analogRead(gpioPOT1);
  POT2=analogRead(gpioPOT2);
  switch (selector)
  {
  case 0:
      
    filtering();
    dacWrite(dac1Pin,map(Y0,1,4095,1, POT0)/16); 
    dacWrite(dac2Pin,map(Y1,1,4095,1, POT0)/16);
    break;
  case 1:
  
    timerInit();
    tremolo();
    break;
  case 2:
  
    filtering();
    distortion();
    break;
  case 3:
  
    timer1Init();
    delayEffect();
    break;
  default:
    break;
  }
  // put your main code here, to run repeatedly:
  
   
 }











 void ledEffectsControl(){
  switch (selector)
  {
  case 0:
    digitalWrite(led_clean,LOW);
    digitalWrite(led_tremolo,HIGH);
    digitalWrite(led_distortion,HIGH);
    digitalWrite(led_delay,HIGH);
    
    break;
  case 1:
    digitalWrite(led_clean,HIGH);
    digitalWrite(led_tremolo,LOW);
    digitalWrite(led_distortion,HIGH);
    digitalWrite(led_delay,HIGH);
    
    break;
  case 2:
    digitalWrite(led_clean,HIGH);
    digitalWrite(led_tremolo,HIGH);
    digitalWrite(led_distortion,LOW);
    digitalWrite(led_delay,HIGH);
    
    break;
  case 3:
    digitalWrite(led_clean,HIGH);
    digitalWrite(led_tremolo,HIGH);
    digitalWrite(led_distortion,HIGH);
    digitalWrite(led_delay,LOW);
    
    break;
  default:
    break;
  }

  
}

void clean(){
  timerAlarmDisable(timer1);
  timerWrite(timer1,0);
  timerAlarmDisable(timer);
  timerWrite(timer,0);
  selector = 0;
  ledEffectsControl();
  delay(1000);
}
void prev(){
  timerAlarmDisable(timer1);
  timerWrite(timer1,0);
  timerAlarmDisable(timer);
  timerWrite(timer,0);
  if(selector>0){
    selector-=1;
  }else{
    selector = 3;
  }
  ledEffectsControl();
  delay(1000);
}
void next(){
  timerAlarmDisable(timer1);
  timerWrite(timer1,0);
  timerAlarmDisable(timer);
  timerWrite(timer,0);
  if(selector<3){
    selector+=1;
  }else{
    selector=0;
  }
  ledEffectsControl();
  delay(1200);
}

void sinewaveGenerator(){
  for(uint32_t nIndex=0; nIndex<44100; nIndex++)
  {
    float arg = ((2.0*PI)/samples)*nIndex;
    // normalised to 12 bit range 0-4095
    nSineTable[nIndex] = (uint16_t)  (((1+sin(arg))*4095.0)/2);
  }
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
  
  filtering();
  
  POT1 = POT1>>1; //divide value by 2 (its too big) 
  count++; 
  if (count>=160) //160 chosen empirically
  {
     count=0;
    sample=sample+POT1;
    if(sample>=samples) sample=0;
  }

  LFO=map(nSineTable[sample],0,4095,(4095-POT2),4095); // maneja la frecuencia con el POT2
  out_DAC0=map(Y0,1,4095,1, LFO);
  out_DAC1=map(Y1,1,4095,1, LFO);
 
  //Add volume feature with POT0
  out_DAC0=map(out_DAC0,1,4095,1, POT0);
  out_DAC1=map(out_DAC1,1,4095,1, POT0);

  dacWrite(dac1Pin,out_DAC0/16);
  dacWrite(dac2Pin,out_DAC1/16);
}

void distortion(){
  
  upper_threshold=map(POT2,0,4095,4095,2047);
  lower_threshold=map(POT2,0,4095,0000,2047);
  if(Y0>=upper_threshold) Y0=upper_threshold;
  else if(Y0<lower_threshold)  Y0=lower_threshold;
 
  if(Y1>=upper_threshold) Y1=upper_threshold;
  else if(Y1<lower_threshold)  Y1=lower_threshold;
 
  //adjust the volume with POT0
  out_DAC0=map(Y0,0,4095,1,POT0);
  out_DAC1=map(Y1,0,4095,1,POT0);

  dacWrite(dac1Pin,out_DAC0/16);
  dacWrite(dac2Pin,out_DAC1/16);

  
}
void delayEffect(){
  filtering();
  sDelayBuffer0[DelayCounter] = Y0;
  sDelayBuffer1[DelayCounter] = Y0;
  sDelayBuffer0[DelayCounter] = in_ADC0;
  sDelayBuffer1[DelayCounter] = in_ADC1;
  
  //Adjust Delay Depth based in pot2 position.
  Delay_Depth=map(POT2>>2,0,1023,1,MAX_DELAY);
  
  //Increse/reset delay counter.   
  DelayCounter++;
  if(DelayCounter >= Delay_Depth) DelayCounter = 0; 

  out_DAC0 = ((sDelayBuffer0[DelayCounter]));
  out_DAC1 = ((sDelayBuffer1[DelayCounter]));
  

  //Add volume feature
  out_DAC0=map(out_DAC0,0,4095,1,POT0);
  out_DAC1=map(out_DAC1,0,4095,1,POT0);

  dacWrite(dac1Pin,out_DAC0/16);
  dacWrite(dac2Pin,out_DAC1/16);
}
void timerInit(){
  timer = timerBegin(0, 5442, true);
  timerAttachInterrupt(timer,tremolo, true);
  timerAlarmWrite(timer, 1, true);
  timerAlarmEnable(timer);
}
void timer1Init(){
  timer1 = timerBegin(0, 5442, true);
  timerAttachInterrupt(timer1,delayEffect, true);
  timerAlarmWrite(timer1, 1, true);
  timerAlarmEnable(timer1);
}

