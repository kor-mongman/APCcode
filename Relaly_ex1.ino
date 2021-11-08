int mq3Pin=A0; //mq3 pin
int RelPin=3; //relay pin
float value; //mq3 순수 값
float value2; //mq3 계산 값
void setup() {
  Serial.begin(9600);
  pinMode(RelPin,OUTPUT);
}
//0.05 이상 면허정지
void loop() {
  value=analogRead(mq3Pin);
  Serial.print("Alcohol value:");
  Serial.print((value)/10000);
  Serial.println("mg/L");
  value2=value/10000;
  delay(500);
  if(value2>0.05){
    digitalWrite(RelPin,LOW);
    //delay(100);
  }
  else {
    digitalWrite(RelPin,HIGH);
    //delay(100);
  }
}
