#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

SoftwareSerial BTSerial(2,3);
SoftwareSerial FPSerial(4,5);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FPSerial);

int pinLED=8;
int id_value = 1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //
  BTSerial.begin(9600);
  finger.begin(57600); //아두이노-지문모듈 통신라인
  pinMode(pinLED,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte r_data;
  int id = getFingerprintIDez();

  BTSerial.listen();
  if(id != -1){ //인식된 지문의 id가 -1이 아니라면
    Serial.println("인식된 지문의 ID = ");
    Serial.println(id);
    
    if(id == 2){ //id가 2또는 3일 때
      //1이면 권힌이 있는 지문
      id_value = 0;
      FPSerial.listen();
    }else{
      //권한이 없는 지문
     Serial.println("Your not master");
    }
  }
  delay(50);

  if(id_value == 0){
    if(BTSerial.available()){
    //r_data=Serial.read()-'0'; //Serial에서는 아스키코드
    r_data=BTSerial.read();
    switch(r_data){
      case 1:
      digitalWrite(pinLED,HIGH); break;
      case 2:
      digitalWrite(pinLED,LOW); break;
      }
    }
  }
}


int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  return finger.fingerID;
}
