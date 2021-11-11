void setup() {
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(10,OUTPUT);
  /*
  digitalWrite(13,HIGH);
  digitalWrite(11,HIGH);
  delay(1000);
  digitalWrite(13,LOW);
  digitalWrite(12,LOW);
  digitalWrite(11,LOW);
  digitalWrite(10,LOW);
  digitalWrite(12,HIGH);
  digitalWrite(10,HIGH);
  delay(1000);
  */
  digitalWrite(13,LOW); //IN1을 HIGH로 하면 모터A가 전진
  digitalWrite(12,LOW); //IN2를 HIGH로 하면 모터A가 후진
  digitalWrite(11,LOW); //IN3를 HIGH로 하면 모터B가 전진
  digitalWrite(10,LOW); //IN4를 HIGH로 하면 모터B가 후진
  //그러나 10번 13번이 후진이고 11번 12번이 전진임.
  //블로그 상 내용과 실제 회로도가 다름을 의미함.
  
}

void loop() {

  
  //digitalWrite(11,HIGH); digitalWrite(12,HIGH);  //digitalWrite(11,HIGH);  digitalWrite(10,HIGH);
  delay(200);
  
  digitalWrite(13,LOW); digitalWrite(12,LOW); digitalWrite(11,LOW); digitalWrite(10,LOW); delay(1200);
  
  /*
  if(t == 'F'){
    digitalWrite(13,HIGH);
    digitalWrite(11,HIGH); 
  }
  else if(t == 'B'){
    digitalWrite(12,HIGH);
    digitalWrite(10,HIGH); 
  }
  else if(t == 'L'){
    digitalWrite(11,HIGH); 
  }
  else if(t == 'R'){
    digitalWrite(13,HIGH);
  }
  else if(t == 'W'){
    digitalWrite(9,HIGH);
  }
  else if(t == 'w'){
    digitalWrite(9,LOW);
  }
  else if(t == 'S'){
    digitalWrite(13,LOW);
    digitalWrite(12,LOW);
    digitalWrite(11,LOW);
    digitalWrite(10,LOW);
  }
  delay(10);
  */
}
