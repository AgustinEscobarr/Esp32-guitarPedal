#include <Arduino.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include <driver/dac.h>
#define repeat 10
const int adcPin = 34;
const int dacPin=25;
float Y = 0.0;
float S = Y;
float alpha = 0.13;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dac_output_enable(DAC_CHANNEL_1);  // se habilita del DAC 1 (GPIO25)
  dac_output_voltage(DAC_CHANNEL_1, 23); //se intenta configurar la tensión de salida
  
}

void serialPrinter(){
  // Serial.print(Y);
  // Serial.print(",");
  Serial.println(S);
}

void filtering(){ // Filtro FIR, donde se toman una cantidad determinada de muestras para obtener el valor medio móvil (se obtiene un solo valor).
  for(int i=0;i<repeat;i++){
    Y = (float) analogRead(adcPin); // Valor de la entrada del ADC.
    S = (alpha*Y) + ((1-alpha)*S); // S es el acumulador, donde se obtiene el valor medio de las muestras. Alpha es el valor de suavizado.  
  }
  serialPrinter();
  dacWrite(dacPin,(S*23)/4095); // literalmente ese comando es lo mismo que dac_output_voltage y  dac_output_enable al mismo tiempo, sospecho que hace todo lo mismo.

}

void loop() {
  // put your main code here, to run repeatedly:
   filtering();
 }


