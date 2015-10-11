#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 8); // RX, TX



int led = 13;  //Pin der LED
int adc_low, adc_high;  //Zwischenspeicher für die Ergebnisse des ADC
long adc_result;  //Gesamtergebnis der Messung des ADC
long vcc;  //Versorgungsspannung

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 Serial.println(result);
//  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  result = 1102200L / result; 
  return result; // Vcc in millivolts
}





void setup() {    
  Serial.begin(57600);
  pinMode(led, OUTPUT);  
  pinMode(A0, INPUT);  
  mySerial.begin(19200);

}

// the loop routine runs over and over again forever:
void loop() {    
  
vcc = readVcc();

  //wenn Spannung kleiner als 5V 
  if (vcc < 5000)
  {
    digitalWrite(led, HIGH);  //schalte LED an
  }
  //wenn größer oder gleich 5V
  else 
  {
    digitalWrite(led, LOW);  //schalte LED aus
  }
mySerial.println(vcc);
  delay(1000);
}


