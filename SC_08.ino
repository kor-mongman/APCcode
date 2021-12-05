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
#define blue 3
#define green 4
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
#define range 70
#define brake 50

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1); //지문인식 통신 함수
MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type); //알코올 센서 지정 함수
/* 지문인식에 필요한 함수 */
int id;
int master;
int id_flag = 1;

/* 알코올 센서에 필요한 함수 */
bool pre_button_state = HIGH;
float ppm = 0; //누적 수치
int cnt = 0; //샘플의 갯수
int mq3_flag = 1;
int re_check = 0;

/* 모터드라이버에 필요한 함수 */
char BT;
int lm1 = 11;
int lm2 = 10; //왼쪽 모터 제어
int rm1 = 12;
int rm2 = 13; //오른쪽 모터 제어

/* 초음파 센서에 필요한 함수 */
//초음파 센서는 4개
const int pinTrig[4]={22,24,26,28}; //22: 전방, 24: 왼쪽 ,26: 오른쪽, 28: 후방
const int pinEcho[4]={23,25,27,29}; //23: 전방, 25: 왼쪽 ,27: 오른쪽, 29: 후방
int distance[4]={0}; //distance[0]; distance[1]; distance[2]; distance[3];
int pulseTime[4]={0}; //0,1,2,3
int buz_flagF=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(9600);
  finger.begin(57600); //아두이노-지문모듈 통신라인

  pinMode(blue,OUTPUT);
  pinMode(green,OUTPUT);
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
  //초음파
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
        digitalWrite(blue,HIGH);
        id_flag = 0;
      }else{
        Serial.println("Your not master");
        id_flag = 1;
      }
    }

  delay(50);

//알코올 측정
  if(id_flag == 0){
    while(re_check < 3){
    bool button_state = digitalRead(button); //버튼을 눌렀을 때

    if(button_state == LOW){
      MQ3.update();
      ppm += MQ3.readSensor();
      cnt++;
    }else if(pre_button_state == LOW && button_state == HIGH){
      Serial.println(ppm);
      Serial.println(cnt);
      //ppm = ppm/cnt;
      ppm = 0.01;
      if(ppm >= 0 && ppm <= 0.03){ //훈방
        Serial.println("Hoon bang");
        //LED 점등
        digitalWrite(green,HIGH);
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
         break;
      }else if(ppm > 0.03){ //면허 정지 및 취소 깜빵행
        Serial.println("GGam bang");
        //LED 점등
        digitalWrite(red,HIGH);
        digitalWrite(green,LOW);
        //부저
        tone(buzzer,1568); //솔
        delay(300);
        tone(buzzer,1568); //솔
        delay(300);
        noTone(buzzer); //off
        delay(300);
        digitalWrite(red,LOW);
        mq3_flag = 1;
        re_check++;
      }
      ppm = 0;
      cnt = 0;
    }
    pre_button_state = button_state; //이월!
    delay(500); //smapling frequency
    }
  }

  delay(500);
  digitalWrite(blue,LOW);
  digitalWrite(green,LOW);

//3번 실패하면 알람가요 ㅋㅋ
  if(re_check == 3){
    digitalWrite(red,LOW);
    tone(buzzer,1568);
    delay(300);
    tone(buzzer,1568);
    delay(300);
  }

  
  while(id_flag == 0 && mq3_flag == 0){
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
          
    //조종
      if(BT == '1'){ //직진
        Serial.println("Forward");
        Forward();
      }else if(BT == '4'){ //후진 
        Serial.println("Back");
        Back();
      }else if(BT == '3'){ //자회전
        Serial.println("Left");
        Left();
      }else if(BT == '2'){ //우회전
        Serial.println("Right");
        Right();
      }else if(BT == '0'){ //정지
        Serial.println("Stop");
        Stop();
      }else if(BT == '5'){ //시동 OFF
        Serial.println("OFF");
        exit(0);
      }
    //BT=0;
     }
     
    if(distance[0] < brake){ //전면
      Stop();
      //delay(1250);
      //BT = '0';
      Serial.println("SSTTOOPP");
    }else if(distance[0] < range){
      tone(buzzer,1568);
      delay(25);
      noTone(buzzer);
      delay(25);
      Serial.println("Forward");
    }
    else {
      noTone(buzzer);
    }
   
     if(distance[3] < (range/10)){ //후면
     tone(buzzer,1568);
     delay(25);
     noTone(buzzer);
     delay(25);
     Serial.println("back");
     }
     else{
      noTone(buzzer);
     }
    
    if(distance[1] < range){ //후측면 left
     digitalWrite(left,HIGH);
     Serial.println("Left");
    }
    else{
      digitalWrite(left,LOW);
    }
    if(distance[2] < range){ //후측면 right
     digitalWrite(right,HIGH);
     Serial.println("Right");
    }
    else{
      digitalWrite(right,LOW);
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

void Forward(){ //전진
  digitalWrite(lm1,HIGH);
  digitalWrite(lm2,LOW);
  digitalWrite(rm1,HIGH);
  digitalWrite(rm2,LOW);
}

void Back(){ //후진
  digitalWrite(lm1,LOW);
  digitalWrite(lm2,HIGH);
  digitalWrite(rm1,LOW);
  digitalWrite(rm2,HIGH);  
}

void Left(){ //자회전
  digitalWrite(lm1,LOW);
  digitalWrite(lm2,HIGH);
  digitalWrite(rm1,HIGH);
  digitalWrite(rm2,LOW);  
}

void Right(){ //우회전
  digitalWrite(lm1,HIGH);
  digitalWrite(lm2,LOW);
  digitalWrite(rm1,LOW);
  digitalWrite(rm2,HIGH);  
}

void Stop(){ //정지
  digitalWrite(lm1,LOW);
  digitalWrite(lm2,LOW);
  digitalWrite(rm1,LOW);
  digitalWrite(rm2,LOW);  
}
