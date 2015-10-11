/*
  ReadAnalogVoltage
  Reads an analog input on pin 0, converts it to voltage, and prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 8); // RX, TX

#define LED_PIN   13
#define NUM_SAMPLES   5


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(19200);
  mySerial.begin(19200);
  pinMode(A0, INPUT); 
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 
}

// the loop routine runs over and over again forever:
void loop() {
  static int alarm, alarm_cnt;
  float total, voltage;
  int i;
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  for(i = 0,total=0.0; i < NUM_SAMPLES; i++)
  {
//  float voltage = sensorValue * (4935.0 / 1023.0);
    total += sensorValue * (4935.0 / 1023.0);
  }
  voltage = total / NUM_SAMPLES;
  // print out the value you read:
  if( voltage <= 4000 )
  {
    if(alarm == 0)
    {
      digitalWrite(LED_PIN, HIGH);
      alarm=1;
    }
    alarm_cnt++;
  }
  else
  {
    if(alarm == 1)
    {
      digitalWrite(LED_PIN, LOW);
      alarm=0;
      alarm_cnt=0;
    }
  }

//  Serial.print("Alarm-Count: ");
//  Serial.print(alarm_cnt);
//  Serial.print(" Voltage im mV: ");   
  Serial.println(voltage);
  
  mySerial.print("Alarm-Count: ");
  mySerial.print(alarm_cnt);
  mySerial.print(" Voltage im mV: ");   
  mySerial.println(voltage);
  delay(1000);
}

