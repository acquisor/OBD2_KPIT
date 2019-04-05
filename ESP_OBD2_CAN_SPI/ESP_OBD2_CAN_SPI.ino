#include<SoftwareSerial.h>
SoftwareSerial M3(D3,D2);

void setup() {
  Serial.begin(9600);
  M3.begin(57600);

}

void loop() {
  while(M3.available())
  {
    digitalWrite(D4,0);
    int ADC_val = M3.parseInt();
    if(M3.read()=='\n')
    {
      Serial.println();
    }
  }
  digitalWrite(D4,1);

}
