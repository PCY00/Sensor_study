/* 
5V, 15mA Trigger Pin Input, Echo pin Output 
*/

#define Trigger 8
#define Echo 9

void setup(){
  Serial.begin(9600);
  
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
}

void loop(){
  int distance = 0;
  digitalWrite(Trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(Trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trigger, LOW);

  unsigned long duration = pulseIn(Echo, HIGH);

  distance =(duration * 0.0343) / 2;
  Serial.print("거리: ");
  Serial.println(distance);

  delay(1000);
}
 
