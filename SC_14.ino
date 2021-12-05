#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
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
#define button2 14
#define buzzer 15
//모터
#define IN1 13 //왼쪽 후진
#define IN2 12 //왼쪽 전진
#define IN3 11 //오른쪽 전진
#define IN4 10 //오른쪽 후진
#define ENA 8 //왼쪽 모터 제어 (IN3, IN4)
#define ENB 9 //오른쪽 모터 제어 (IN1, IN2)

//초음파 감지 거리
#define range 50
#define brake 27 //급정거 거리

LiquidCrystal_I2C Lcd(0x27,16,2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1); //지문인식 통신 함수
MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type); //알코올 센서 지정 함수
/* 지문인식에 필요한 함수 */
uint8_t id;
int Id;
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

/* 초음파 센서에 필요한 함수 */
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

  pinMode(blue,OUTPUT);//지문인식 완료
  pinMode(green,OUTPUT);//음주측정 완료
  pinMode(red,OUTPUT);//음주측정 실패
  pinMode(button,INPUT_PULLUP); //저항 없이 쓸 때
  pinMode(button2,INPUT_PULLUP); 
  pinMode(left,OUTPUT);
  pinMode(right,OUTPUT);
  pinMode(buzzer,OUTPUT);
  //모터
  pinMode(ENA,OUTPUT);
  pinMode(ENB,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  //초음파
  for(int i=0;i<4;i++){
   pinMode(pinTrig[i],OUTPUT);
   pinMode(pinEcho[i],INPUT);
  }

  /*음주측정*/
  MQ3.setRegressionMethod(1); 
  MQ3.setA(0.3934); MQ3.setB(-1.504); 
  MQ3.init(); 
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++){
    MQ3.update();
    calcR0 += MQ3.calibrate(RatioMQ3CleanAir);
  }
  MQ3.setR0(calcR0/10);
  MQ3.serialDebug(true);
  //모터를 끄고 시작
  digitalWrite(IN1,LOW); 
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  //LCD모니터 세팅
  Lcd.begin(16,2);
  Lcd.init();

  Lcd.noBacklight();
  delay(250);
  Lcd.backlight();
  delay(250);
  
  Lcd.setCursor(0,0);
  Lcd.print("Welcome");
  delay(1500);
  Lcd.setCursor(0,1);
  Lcd.print("APC 4Team");
  delay(1500);

  Lcd.clear();
}

void loop() {
  Lcd.setCursor(0,0);
  Lcd.print("Start");
  delay(1000);
  Lcd.clear();  
//지문 등록
  bool button2_state = digitalRead(button2);//버튼 작동 이후 지문인식
  if(button2_state == LOW){
  Lcd.setCursor(0,0);  
  Lcd.print("Ready to enroll");
  delay(200);
  Lcd.setCursor(0,1);  
  Lcd.print("a fingerprint");
  delay(1500);
  Lcd.clear();
  
  Lcd.setCursor(0,0); //지문 등록
  Lcd.print("Please type in");
  delay(200);
  Lcd.setCursor(0,1);
  Lcd.print("the ID 1 to 127");
  delay(1500);
  Lcd.clear();

  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Lcd.print("Enrolling ID = ");
  Lcd.print(id);
  
  getFingerprintEnroll();
  }


//지문인식
  Id = getFingerprintIDez();
  master = Id;
    if(Id != -1){
      Lcd.clear();
      Lcd.setCursor(0,0);
      Lcd.print("Your ID = ");
      delay(500);
      Lcd.print(Id);
      delay(800);
      if(Id == master){//지문 인식 성공
        Lcd.setCursor(0,1);
        Lcd.print("Your master");
        delay(1000);
        Lcd.clear();
        digitalWrite(blue,HIGH);
        id_flag = 0;
      }else{//지문 인식 실패
        Lcd.clear();
        Lcd.setCursor(0,0);
        Lcd.print("Your not master");
        delay(1000);
        Lcd.clear();
        id_flag = 1;
      }
    }

  delay(50);

//알코올 측정
  if(id_flag == 0){
    //총 3번의 기회가 주어짐
    while(re_check < 3){
      
      Lcd.clear();
      Lcd.setCursor(0,0);
      Lcd.print("Press button");
      delay(1500);
      Lcd.clear();
      bool button_state = digitalRead(button); //버튼을 눌렀을 때

    if(button_state == LOW){//버튼을 눌렀을 때 값을 축적
      MQ3.update();
      ppm += MQ3.readSensor();
      cnt++;
    }else if(pre_button_state == LOW && button_state == HIGH){//버튼을 떼자마자 알콜 농도를 계산함
      ppm = ppm/cnt; //음주 평균 값 만들기
      
      //음주 측정 결과
      if(ppm >= 0 && ppm <= 0.03){ //훈방, 음주 측정 통과
        Lcd.clear();
        Lcd.setCursor(0,0);
        Lcd.print("Hoon Bang");
        delay(200);
        Lcd.setCursor(0,1);
        Lcd.print(ppm);
        delay(1000);
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
        Lcd.clear();
         mq3_flag = 0;
         break;
      }else if(ppm > 0.03){ //면허 정지 및 취소 깜빵행
        Lcd.clear();
        Lcd.setCursor(0,0);
        Lcd.print("GGam Bang");
        delay(200);
        Lcd.setCursor(0,1);
        Lcd.print(ppm);
        delay(1000);
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
        Lcd.clear();
        mq3_flag = 1;
        re_check++;
      }
      ppm = 0;
      cnt = 0;
    }
    pre_button_state = button_state; //둘의 값을 동일하게 맞춰줌
    delay(500); //smapling frequency
    }
  }

  delay(500);
  digitalWrite(blue,LOW);
  digitalWrite(green,LOW);

//3번 실패하면 알람
  if(re_check == 3){
    digitalWrite(red,LOW);
    tone(buzzer,1568);
    delay(300);
    tone(buzzer,1568);
    delay(300);
    noTone(buzzer);
    Lcd.clear();
    Lcd.setCursor(0,0);
    Lcd.print("Chance OUT");
    delay(500);
    Lcd.setCursor(0,1);
    Lcd.print("Go To Sleep");
    Lcd.noBacklight();
    delay(300);
    Lcd.backlight();
    delay(300);
    Lcd.noBacklight();
    delay(300);
    Lcd.backlight();
    delay(1000);
    
    re_check++;
  }
  Lcd.clear();
  //제어 while문
  while(id_flag == 0 && mq3_flag == 0){
    //초음파 세팅
    Lcd.clear();
     for(int i=0;i<4;i++){
       digitalWrite(pinTrig[i],LOW);
       delayMicroseconds(2);
       digitalWrite(pinTrig[i],HIGH);
       delayMicroseconds(10);
       digitalWrite(pinTrig[i],LOW);
       pulseTime[i]=pulseIn(pinEcho[i],HIGH);
       distance[i]=pulseTime[i]/58; //distance 값으로 센싱
     }
     //BT값을 누르면 BT가 변경됨
     if(Serial2.available()){
     BT = Serial2.read();
     }
      //BT값에 따라 해당 명령 수행
      if(BT == '1'){ //직진
        Forward(100,100);
        if(distance[0] < brake){ //직진 중 사물이 매우 가까울 시 급정거
          Stop();
          BT = '0'; //BT에 '0'을 넣어서 정지 상태로 만듦.
        }else if(distance[0] < range){ //전진 감지
          tone(buzzer,1568);//부저 울림
          delay(75);
          tone(buzzer,1568);
          delay(75);
          noTone(buzzer);
        }
        else{
          noTone(buzzer);
            }
      }
      else if(BT == '4'){ //후진 
        Back(100,100);
        
      }else if(BT == '3'){ //좌회전
        Left(75,100);
        
      }else if(BT == '2'){ //우회전
        Right(100,75);
        
      }else if(BT == '0'){ //정지
        Stop();
        
      }else if(BT == '5'){ //시동 OFF
        Lcd.clear();
        Lcd.setCursor(0,0);
        Lcd.print("Open Door");
        delay(200);
        Lcd.setCursor(0,1);
        Lcd.print("Bye Bye");
        delay(1500);
        Lcd.noBacklight();   
        Lcd.clear();
      }
     
     if(distance[3] < (range/5)){ //후면 감지
      tone(buzzer,1568);//부저 울림
      delay(75);
      tone(buzzer,1568);
      delay(75);
      noTone(buzzer);
      
     }
     else{
      noTone(buzzer);
     }
    
    if(distance[1] < range){ //후측면 left 감지
     digitalWrite(left,HIGH);
    }
    else{
      digitalWrite(left,LOW);
    }
    if(distance[2] < range){ //후측면 right 감지
     digitalWrite(right,HIGH);
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

/*
  #define IN1 13 //왼쪽 후진
  #define IN2 12 //왼쪽 전진
  #define IN3 11 //오른쪽 전진
  #define IN4 10 //오른쪽 후진
 */
void Forward(int L_Speed, int R_Speed){ //전진 
  digitalWrite(IN1,LOW); 
  digitalWrite(IN2,HIGH); 
  digitalWrite(IN3,HIGH); 
  digitalWrite(IN4,LOW);  
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);
}

void Back(int L_Speed, int R_Speed){ //후진
  digitalWrite(IN1,HIGH);  
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH); 
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);  
}

void Left(int L_Speed, int R_Speed){ //자회전
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW); 
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);
}

void Right(int L_Speed, int R_Speed){ //우회전
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW); 
  digitalWrite(IN4,HIGH);
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);
}

void Stop(){ //정지
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);   
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial2.available());
    num = Serial2.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Lcd.clear();
  delay(400);
  Lcd.setCursor(0,0);    
  Lcd.print("Waiting finger");
  delay(250);
  Lcd.setCursor(0,1);  
  Lcd.print("enroll ID = ");
  delay(1000);
  Lcd.print(id);
  delay(1500);
  Lcd.clear();
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Lcd.setCursor(0,0);
      Lcd.print("Image taken");
      delay(1000);
      Lcd.clear();
      break;
    case FINGERPRINT_NOFINGER:
      Lcd.setCursor(0,0);
      Lcd.print(".");
      delay(800);
      Lcd.clear();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Lcd.setCursor(0,0);
      Lcd.print("Communication error");
      delay(800);
      Lcd.clear();
      break;
    case FINGERPRINT_IMAGEFAIL:
      break;
    default:
      Lcd.setCursor(0,0);    
      Lcd.print("Unknown error");
      delay(800);
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Lcd.setCursor(0,0);
      Lcd.print("Image converted");
      delay(1000);
      Lcd.clear();
      break;
    case FINGERPRINT_IMAGEMESS:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      return p;
    case FINGERPRINT_FEATUREFAIL:
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      return p;
    default:
      Lcd.setCursor(0,0);    
      Lcd.print("Unknown error");
      delay(800);
      Lcd.clear();
      return p;
  }
  Lcd.setCursor(0,0);  
  Lcd.print("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Lcd.clear();
  Lcd.setCursor(0,0);
  Lcd.print("ID = "); Lcd.print(id);
  delay(1000);
  
  p = -1;

  Lcd.clear();
  Lcd.setCursor(0,0);  
  Lcd.print("Place same");
  delay(1000);
  Lcd.setCursor(0,1);
  Lcd.print("finger again");
  delay(1000);
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Lcd.setCursor(0,1);
      Lcd.print("Image taken");
      delay(1000);
      break;
    case FINGERPRINT_NOFINGER:
      Lcd.clear();
      Lcd.setCursor(0,0);  
      Lcd.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      break;
    case FINGERPRINT_IMAGEFAIL:
      break;
    default:
      Lcd.setCursor(0,0);    
      Lcd.print("Unknown error");
      delay(800);
      Lcd.clear();
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Lcd.clear();
      Lcd.setCursor(0,0);
      Lcd.println("Image converted");
      delay(1000);
      
      break;
    case FINGERPRINT_IMAGEMESS:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      return p;
    case FINGERPRINT_FEATUREFAIL:
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      return p;
    default:
      Lcd.setCursor(0,0);    
      Lcd.print("Unknown error");
      delay(800);
      Lcd.clear();
      return p;
  }
  
  // OK converted!
  Lcd.clear();
  Lcd.setCursor(0,0);
  Lcd.print("Creating model");
  delay(200);
  Lcd.setCursor(0,1);
  Lcd.print("for = ");
  delay(200);
  Lcd.print(id);
  delay(800);
  Lcd.clear();
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Lcd.setCursor(0,0);  
    Lcd.print("Prints matched!");
    delay(800);
    Lcd.clear();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Lcd.setCursor(0,0);   
    Lcd.print("Fingerprints did not match");
    delay(200);
    Lcd.setCursor(0,1);
    Lcd.print("not match");
    delay(200);
    return p;
  } else {
    Lcd.setCursor(0,0);  
    Lcd.print("Unknown error");
    delay(800);
    Lcd.clear();
    return p;
  }
  Lcd.clear();   
  Lcd.setCursor(0,0)
  Lcd.print("ID = ");
  delay(200);
  Lcd.setCursor(0,1);
  Lcd.println(id);
  delay(500);
  Lcd.clear();
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Lcd.setCursor(0,0);  
    Lcd.print("Stored!");
    delay(800);
    Lcd.clear(); 
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    return p;
  } else {
    Lcd.setCursor(0,0);  
    Lcd.print("Unknown error");
    delay(800);
    Lcd.clear();
    return p;
  }   
}
