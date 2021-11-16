#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


int mq3 = A0;
int LED1 = 4;
int LED2 = 5;            
int id;
int id_value; //id가 apply 되었을 때 사용되는 값

float mq3_value1; //mq3 순수 값
float mq3_value2; //mq3 계산 값

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  finger.begin(57600); //아두이노-지문모듈 통신라인
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
}

void loop() {
  
  //fingerprint source code
  id = getFingerprintIDez();//id 가져오기
  delay(1000);
  mq3_value1 =analogRead(mq3); //mq3 값 가져오기
  mq3_value2 = (mq3_value1/10000)-0.005; //mq3 계산하기 및 기준치 정하기
  
  if(id != -1){ //인식된 지문의 id가 -1이 아니라면
    Serial.println("인식된 지문의 ID = ");
    Serial.println(id);
    
    if(id == 2){ //id가 1일때
      //1이면 권한이 있는 지문

      digitalWrite(LED1,HIGH);
      delay(5000);
      Serial.print(mq3_value2);
      if(mq3_value2 >= 0.03){
        Serial.println("Your master");
        digitalWrite(LED1,LOW);
        digitalWrite(LED2,LOW);
      }else{
       digitalWrite(LED2,HIGH);
      }
      
    }else{
      //권한이 없는 지문
     Serial.println("Your not master");
    }
  }
  delay(50);

} //loop문 끝

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  return finger.fingerID;
}
