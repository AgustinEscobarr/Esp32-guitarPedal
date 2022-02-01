#include <Arduino.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#define repeat 5
const int adcPin = 34;
float Y = 0.0;
float S = Y;
int samples[repeat];
boolean firstTime = true;

void filterInit(){   //carga con valores tomados del adc al vector de muestras la primera vez.
  int i=0;
  for(i=0;i<=repeat-1;i++){
    Y = (float) analogRead(adcPin);
    S +=  Y / repeat;
    samples[i]= Y / repeat;
  }
  firstTime = false; 
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  int i;
  
  for(i=0 ;i<=repeat-1 ;i++){  // Se inicializan los valores del vector que va a guardar las muestras
    samples[i]=0;
  };
  filterInit();
  
  
}

void serialPrinter(float sampleFiltered){  //imprime los valores de salida del filtro.
  // Serial.print(",");
  Serial.println(sampleFiltered);
}


void filtering(){
  // Y = (float) analogRead(adcPin);
  // S = 0.13*Y + (1-0.13)*S;
  // serialPrinter(S);
  float lastSample = ((float) analogRead(adcPin) )/ repeat; // toma un nuevo valor del adc, que se agrega en la ultima posicion del vector.
  S = S - samples[0] + lastSample;
  serialPrinter(S);

  for(int i = 0 ; i<=repeat-1 ; i++){
    if(i<repeat-1){
      samples[i]=samples[i+1];
    }
    else{
      samples[repeat-1]= lastSample;
    }
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.print(analogRead(adcPin));

   filtering();
 }


