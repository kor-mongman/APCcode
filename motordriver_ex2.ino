#include <SoftwareSerial.h>
SoftwareSerial BT(2, 3);

#define IN1 11
#define IN2 10
#define IN3 12
#define IN4 13

char a;
int lm1, lm2, rm1, rm2;
int speed = 200;

void setup() {
  BT.begin(9600);
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(10,OUTPUT);
  digitalWrite(13,LOW); //IN1을 HIGH로 하면 모터A가 전진
  digitalWrite(12,LOW); //IN2를 HIGH로 하면 모터A가 후진
  digitalWrite(11,LOW); //IN3를 HIGH로 하면 모터B가 전진
  digitalWrite(10,LOW); //IN4를 HIGH로 하면 모터B가 후진
  //그러나 10번 13번이 후진이고 11번 12번이 전진임.
  //10번 12번 좌회전 11번 13번 우회전
}

void loop() {
  if (BT.available()) {
    a = BT.read();
    if (a == '1') { // 직진
      lm1 = speed;
      lm2 = 0;
      rm1 = speed;
      rm2 = 0;
    }
    else if (a == '4') { //후진
      lm1 = 0;
      lm2 = speed;
      rm1 = 0;
      rm2 = speed;
    }
    else if (a == '3') { //좌회전
      lm1 = 0;
      lm2 = speed;
      rm1 = speed;
      rm2 = 0;
    }
    else if (a == '2') { //우회전      
      lm1 = speed;
      lm2 = 0;
      rm1 = 0;
      rm2 = speed;
    } else if (a == '0') { //정지
      lm1 = 0;
      lm2 = 0;
      rm1 = 0;
      rm2 = 0;
    }
    analogWrite(IN1, lm1);
    analogWrite(IN2, lm2);
    analogWrite(IN3, rm1);
    analogWrite(IN4, rm2);
  }
}
