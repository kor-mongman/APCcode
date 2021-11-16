#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <MQUnifiedsensor.h>
#define Board                   ("Arduino UNO")
#define Pin                     (A0)  //Analog input 3 of your arduino
#define Type                    ("MQ-3") //MQ3
#define Voltage_Resolution      (5)
#define ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define RatioMQ3CleanAir        (60) //RS / R0 = 60 ppm
#define button 6
#define green 7
#define yellow 8
#define red 9
#define IN1 11
#define IN2 10
#define IN3 12
#define IN4 13

SoftwareSerial BTSerial(2,3);
SoftwareSerial FPSerial(4,5);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FPSerial);
MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/* 지문인식에 필요한 함수 */
int id;
int id_flag = 1;

/* 알코올 센서에 필요한 함수 */
bool pre_button_state = HIGH;
float ppm = 0; //누적 수치
int cnt = 0; //샘플의 갯수
int mq3_flag = 1;

/* 모터드라이버에 필요한 함수 */
char a;
int lm1, lm2, rm1, rm2;
int speed = 200;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  BTSerial.begin(9600);
  finger.begin(57600); //아두이노-지문모듈 통신라인

  pinMode(green,OUTPUT);
  pinMode(yellow,OUTPUT);
  pinMode(red,OUTPUT);
  pinMode(button,INPUT_PULLUP); //저항 없이 쓸 때
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  MQ3.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ3.setA(0.3934); MQ3.setB(-1.504); // Configurate the ecuation values to get Benzene concentration
  MQ3.init(); 
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ3.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ3.calibrate(RatioMQ3CleanAir);
    Serial.print(".");
  }
  MQ3.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  
  MQ3.serialDebug(true);
  
  digitalWrite(IN1,LOW); //IN1을 HIGH로 하면 모터A가 전진
  digitalWrite(IN2,LOW); //IN2를 HIGH로 하면 모터A가 후진
  digitalWrite(IN3,LOW); //IN3를 HIGH로 하면 모터B가 전진
  digitalWrite(IN4,LOW); //IN4를 HIGH로 하면 모터B가 후진
}

void loop() {
  // put your main code here, to run repeatedly:

  if(id_flag == 1){ //id_flag가 1이라면
    FPSerial.listen(); //지문 인식 통신 우선 대기
    id = getFingerprintIDez();

    if(id != -1){
      Serial.println("인식된 지문 ID = ");
      Serial.println(id);

      if(id == 2){
        Serial.println("Your master");
        digitalWrite(green,HIGH);
        id_flag = 0;
      }else{
        Serial.println("Your not master");
        id_flag = 1;
      }
    }
  }

  delay(50);

  if(id_flag == 0){
    bool button_state = digitalRead(button); //버튼을 눌렀을 때

    if(button_state == LOW){
      MQ3.update();
      ppm += MQ3.readSensor();
      cnt++;
    }else if(pre_button_state == LOW && button_state == HIGH){
      Serial.println(ppm);
      Serial.println(cnt);
      ppm = ppm/cnt;

      if(ppm >= 0 && ppm <= 0.03){ //훈방
        Serial.println("Hoon bang");
        digitalWrite(yellow,HIGH);
         mq3_flag = 0;
      }else if(ppm > 0.03){ //면허 정지 및 취소 깜빵행
        Serial.println("GGam bang");
        digitalWrite(red,HIGH);
        mq3_flag = 1;
      }
      ppm = 0;
      cnt = 0;
    }
    pre_button_state = button_state; //이월!
    delay(500); //smapling frequency
  }

  if(id_flag == 0 && mq3_flag == 0){
    BTSerial.listen();
    if(BTSerial.available()){
      a = BTSerial.read();

      if(a == '1'){ //직진
        lm1 = speed;
        lm2 = 0;
        rm1 = speed;
        rm2 = 0;
      }else if(a == '4'){ //후진
        lm1 = 0;
        lm2 = speed;
        rm1 = 0;
        rm2 = speed;
      }else if(a == '3'){ //자회전
        lm1 = 0;
        lm2 = speed;
        rm1 = speed;
        rm2 = 0;
      }else if(a == '2'){ //정지
        lm1 = speed;
        lm2 = 0;
        rm1 = 0;
        rm2 = speed;
      }else if(a == '0'){ //정지
        lm1 = 0;
        lm2 = 0;
        rm1 = 0;
        rm2 = 0;
      }
      analogWrite(IN1,lm1);
      analogWrite(IN2,lm2);
      analogWrite(IN3,rm1);
      analogWrite(IN4,rm2);
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
