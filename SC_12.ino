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
#define button2 14
#define buzzer 15
//모터 
#define ENA 8 //왼쪽 모터 제어 (IN1, IN2)
#define ENB 9 //오른쪽 모터 제어 (IN1, IN2)
#define IN1 13
#define IN2 12 //왼쪽 모터 제어
#define IN3 11
#define IN4 10 //오른쪽 모터 제어
//초음파
#define range 10 //70
#define brake 6 //50

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
  
  MQ3.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ3.setA(0.3934); MQ3.setB(-1.504); // Configurate the ecuation values to get Benzene concentration
  MQ3.init(); 
 // Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ3.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ3.calibrate(RatioMQ3CleanAir);
    //Serial.print(".");
  }
  MQ3.setR0(calcR0/10);
  //Serial.println("  done!.");
  
  if(isinf(calcR0)) //{Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0)//{Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  
  MQ3.serialDebug(true);
  
  digitalWrite(IN1,LOW); //IN1을 HIGH로 하면 모터A가 전진
  digitalWrite(IN2,LOW); //IN2를 HIGH로 하면 모터A가 후진
  digitalWrite(IN3,LOW); //IN3를 HIGH로 하면 모터B가 전진
  digitalWrite(IN4,LOW); //IN4를 HIGH로 하면 모터B가 후진
}

void loop() {
// put your main code here, to run repeatedly:
//지문 등록
  bool button2_state = digitalRead(button2);
  if(button2_state == LOW){
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
  
  getFingerprintEnroll();
  }


//지문인식
  Id = getFingerprintIDez();
  master = Id;
    if(Id != -1){
      Serial.println("인식된 지문 ID = ");
      Serial.println(Id);

      if(Id == master){
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
     //BT값을 누르면 BT가 변경됨
     if(Serial2.available()){
     BT = Serial2.read();
     }
      //조종
      if(BT == '1'){ //직진
        Serial.println("Forward");
        Forward(100,100);
        if(distance[0] < brake){ //전면
          Serial.println("SSTTOOPP");
          Stop(0,0);
        }else if(distance[0] < range){
          Serial.println("Forward Buzzzz");
          tone(buzzer,1568);
          delay(25);
          noTone(buzzer);
          delay(25);
        }
        else{
          noTone(buzzer);
            }
      }
      else if(BT == '4'){ //후진 
        Serial.println("Back");
        Back(100,100);
      }else if(BT == '3'){ //자회전
        Serial.println("Left");
        Left(75,100);
      }else if(BT == '2'){ //우회전
        Serial.println("Right");
        Right(100,75);
      }else if(BT == '0'){ //정지
        Serial.println("Stop");
        Stop(0,0);
      }else if(BT == '5'){ //시동 OFF
        Serial.println("OFF");
        exit(0);
      }
     
     if(distance[3] < (range/10)){ //후면
     tone(buzzer,1568);
     delay(25);
     noTone(buzzer);
     delay(25);
     Serial.println("Back Buzzzzz");
     }
     else{
      noTone(buzzer);
     }
    
    if(distance[1] < range){ //후측면 left
     digitalWrite(left,HIGH);
     Serial.println("Left Dangerrrrr");
    }
    else{
      digitalWrite(left,LOW);
    }
    if(distance[2] < range){ //후측면 right
     digitalWrite(right,HIGH);
     Serial.println("Right Dangerrrrr");
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

void Forward(int L_Speed, int R_Speed){ //전진 //IN1 11, IN2 10, IN3 12, IN4 13 그런데 하드웨어는 IN1 13, IN2 12, IN3 11, IN4 10 IN34 왼쪽 IN12 오른쪽 IN23 전진 IN14후진
  digitalWrite(IN1,LOW); //11번핀 high
  digitalWrite(IN2,HIGH);  //10번핀 low
  digitalWrite(IN3,HIGH); //12번핀 high
  digitalWrite(IN4,LOW);  //13번핀 low //11, 12번핀 high
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);
}

void Back(int L_Speed, int R_Speed){ //후진
  digitalWrite(IN1,HIGH);  //
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH); //10, 13번핀 high
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);  
}

void Left(int L_Speed, int R_Speed){ //자회전
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);  //10,12번핀 high
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);
}

void Right(int L_Speed, int R_Speed){ //우회전
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);  //11, 13번핀 high
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);
}

void Stop(int L_Speed, int R_Speed){ //정지
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW); 
  analogWrite(ENA, L_Speed);
  analogWrite(ENB, R_Speed);   
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial2.available());
    //Serial.write(Serial2.read());
    num = Serial2.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}
