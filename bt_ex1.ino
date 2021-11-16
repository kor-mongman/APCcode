#include <SoftwareSerial.h>

SoftwareSerial BtSerial(2,3);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.println("hello");
  BtSerial.begin(9600);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (BtSerial.available()) {       
    Serial.write(BtSerial.read());
  }
  if (Serial.available()) {         
    BtSerial.write(Serial.read());
  }


}
