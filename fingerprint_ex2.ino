#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //
  finger.begin(57600); //아두이노-지문모듈 통신라인
}

void loop() {
  // put your main code here, to run repeatedly:
  int id = getFingerprintIDez();
  
  if(id != -1){ //인식된 지문의 id가 -1이 아니라면
    Serial.println("인식된 지문의 ID = ");
    Serial.println(id);
    
    if(id == 1){ //id가 1일때 //(if(id < 0 && id > 128) //id가 0 초과고 128 미만 
      //1이면 권힌이 있는 지문
      Serial.println("Your master");
    }else{
      //권한이 없는 지문
     Serial.println("Your not master");
    }
  }
  delay(50);
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
