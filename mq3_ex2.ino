#include <MQUnifiedsensor.h>
#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 3 of your arduino
#define         Type                    ("MQ-3") //MQ3
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ3CleanAir        (60) //RS / R0 = 60 ppm 
#define green 2
#define yellow 3
#define red 4
#define button 5

MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

void setup() {
 
  Serial.begin(9600); //Init serial port
  pinMode(green,OUTPUT);
  pinMode(yellow,OUTPUT);
  pinMode(red,OUTPUT);
  pinMode(button,INPUT_PULLUP); //저항 없이 쓸 때
  
 
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
  /*****************************  MQ CAlibration ********************************************/ 
  MQ3.serialDebug(true);
}

bool pre_button_state = HIGH;
float ppm = 0; //누적수치
int count = 0;//샘플의갯수
void loop() {
  //1버튼을 누르고 있는 동안에 알콜메터기를 측정한다
  //2(버튼을 누르고 있다가) 버튼을 뗀순간 결과가 출력된다.
  bool button_state = digitalRead(button); //버튼 눌렀을 때
  if(button_state == LOW){
    //버튼을 누르고 있는거
    MQ3.update();
    ppm += MQ3.readSensor();
    count++;
  }else if(pre_button_state == LOW && button_state == HIGH){
      //결과출력
    Serial.println(ppm);
    Serial.println(count);
    ppm = ppm/count;

    if(ppm >= 0 && ppm <= 0.03){
      //훈방
      digitalWrite(green, HIGH);
      digitalWrite(yellow, LOW);
      digitalWrite(red, LOW);
    }else if(ppm > 0.03 && ppm <= 0.08){
      //정지
      digitalWrite(green, LOW);
      digitalWrite(yellow, HIGH);
      digitalWrite(red, LOW);
    }else if(ppm > 0.08){
      //취소
      digitalWrite(green, LOW);
      digitalWrite(yellow, LOW);
      digitalWrite(red, HIGH);
    }
    
    ppm = 0;
    count = 0;
  }
  pre_button_state = button_state; //이월!
  delay(500); //Sampling frequency
}
