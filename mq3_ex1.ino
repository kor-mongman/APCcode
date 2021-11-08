int mq3Pin=A0;
float value;
void setup() {
  Serial.begin(9600);
}

void loop() {
  value=analogRead(mq3Pin);
  Serial.print("Alcohol value:");
  Serial.println((value-700)/10000);
  Serial.print("mg/L");

  delay(200);
}
