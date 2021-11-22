#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <MQUnifiedsensor.h>
#define Board                   ("Arduino MEGA")
#define Pin                     (A0)  //Analog input 3 of your arduino
#define Type                    ("MQ-3") //MQ3
#define Voltage_Resolution      (5)
#define ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define RatioMQ3CleanAir        (60) //RS / R0 = 60 ppm
#define button 2
#define green 3
#define yellow 4
#define red 5
#define left 6
#define right 7
#define buzzer 8
//모터 
#define IN1 11
#define IN2 10
#define IN3 12
#define IN4 13
//초음파
#define range 10
#define brake 7

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1); //지문인식 통신 함수
MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type); //알코올 센서 지정 함수
/* 지문인식에 필요한 함수 */
int id;
int master;
int id_flag = 1;
int re_check=0;

/* 알코올 센서에 필요한 함수 */
bool pre_button_state = HIGH;
float ppm = 0; //누적 수치
int cnt = 0; //샘플의 갯수
int mq3_flag = 1;

/* 모터드라이버에 필요한 함수 */
char BT;
int lm1, lm2, rm1, rm2;
int speed = 200;

/* 초음파 센서에 필요한 함수 */
//초음파 센서는 4개
const int pinTrig[5]={22,24,26,28}; //22: 전방, 24: 왼쪽 ,26: 오른쪽, 28: 후방
const int pinEcho[5]={23,25,27,29}; //23: 전방, 25: 왼쪽 ,27: 오른쪽, 29: 후방
int distance[5]={0}; //distance[0]; distance[1]; distance[2]; distance[3];
int pulseTime[5]={0}; //0,1,2,3
int buz_flagF=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(9600);
  finger.begin(57600); //아두이노-지문모듈 통신라인

  pinMode(green,OUTPUT);
  pinMode(yellow,OUTPUT);
  pinMode(red,OUTPUT);
  pinMode(button,INPUT_PULLUP); //저항 없이 쓸 때
  pinMode(left,OUTPUT);
  pinMode(right,OUTPUT);
  pinMode(buzzer,OUTPUT);
  //모터
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  //
  for(int i=0;i<4;i++){
   pinMode(pinTrig[i],OUTPUT);
   pinMode(pinEcho[i],INPUT);
  }
  
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

//지문인식
  id = getFingerprintIDez();
  master = id;
    if(id != -1){
      Serial.println("인식된 지문 ID = ");
      Serial.println(id);

      if(id == master){
        Serial.println("Your master");
        digitalWrite(green,HIGH);
        id_flag = 0;
      }else{
        Serial.println("Your not master");
        id_flag = 1;
      }
    }

  delay(50);

//알코올 측정
  if(id_flag == 0){
    letgo:
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
        //LED 점등
        digitalWrite(yellow,HIGH);
        digitalWrite(red,LOW);
        //부저
        tone(buzzer,2093); //도
        delay(300);
        tone(buzzer,1319); //미
        delay(300);
        tone(buzzer,1568); //솔
        delay(300);
        tone(buzzer,2093); //도
        delay(300);
        noTone(buzzer); //off
         mq3_flag = 0;
      }else if(ppm > 0.03){ //면허 정지 및 취소 깜빵행
        Serial.println("GGam bang");
        //LED 점등
        digitalWrite(red,HIGH);
        digitalWrite(yellow,LOW);
        //부저
        tone(buzzer,1568); //솔
        delay(300);
        tone(buzzer,1568); //솔
        delay(300);
        noTone(buzzer); //off
        mq3_flag = 1;
      }
      if(mq3_flag == 1 && re_check < 3){
        goto letgo;
        digitalWrite(red,LOW);
        re_check++;
        ppm = 0;
        cnt = 0;
      }
      ppm = 0;
      cnt = 0;
    }
    pre_button_state = button_state; //이월!
    delay(500); //smapling frequency
  }

  delay(500);

//초음파 세팅
  for(int i=0;i<4;i++){
    digitalWrite(pinTrig[i],LOW);
    delayMicroseconds(2);
    digitalWrite(pinTrig[i],HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTrig[i],LOW);
    pulseTime[i]=pulseIn(pinEcho[i],HIGH);
    distance[i]=pulseTime[i]/58;
  }

if(Serial2.available()){
  BT = Serial2.read();

  while(id_flag == 0 && mq3_flag == 0){
    //LED OFF
    digitalWrite(green,LOW);
    digitalWrite(yellow,LOW);

    //조종
      if(BT == '1'){ //직진
       lm1 = speed;
       lm2 = 0;
       rm1 = speed;
       rm2 = 0;
      }else if(BT == '4'){ //후진
       lm1 = 0;
       lm2 = speed;
       rm1 = 0;
       rm2 = speed;
      }else if(BT == '3'){ //자회전
       lm1 = 0;
       lm2 = speed;
       rm1 = speed;
       rm2 = 0;
     }else if(BT == '2'){ //정지
       lm1 = speed;
       lm2 = 0;
       rm1 = 0;
        rm2 = speed;
      }else if(BT == '0'){ //정지
        lm1 = 0;
        lm2 = 0;
        rm1 = 0;
        rm2 = 0;
      }else if(BT == '5'){
      exit(0);
    }
     analogWrite(IN1,lm1);
     analogWrite(IN2,lm2);
     analogWrite(IN3,rm1);
     analogWrite(IN4,rm2);

    //전면
     if(distance[0] < range && buz_flagF == 0){
      tone(buzzer,1568);
      delay(300);
      tone(buzzer,1568);
      delay(300);
      noTone(buzzer);
      buz_flagF == 1;
    }else if(distance[0] > range){
      noTone(buzzer);
      buz_flagF = 0;
    }else if(distance[0] < brake){
      lm1 = 0;
      lm2 = 0;
      rm1 = 0;
      rm2 = 0;
      delay(1250);
    }
    //후면
    if(distance[3] < range){
     tone(buzzer,1568);
     delay(300);
     tone(buzzer,1568);
     delay(300);
     noTone(buzzer);
    }else{
      noTone(buzzer);
    }
    //후측면 left
    if(distance[1] < range){
     digitalWrite(left,HIGH);  
    }else{
     digitalWrite(left,LOW);
    }
    //후측면 right
    if(distance[2] < range){
     digitalWrite(right,HIGH);
    }else{
      digitalWrite(right,LOW);
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
