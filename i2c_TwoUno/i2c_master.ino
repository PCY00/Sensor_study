#include <Wire.h>

void write_reg(uint8_t _Add, uint8_t _regAdd, uint8_t _data);
void read_reg(uint8_t _Add);

void setup() {
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  write_reg((uint8_t)0x57, (uint8_t)0x01, (uint8_t)0xAC);
  delay(100);
  read_reg((uint8_t)0x57);
  delay(1000); // 1초 대기
  
  write_reg((uint8_t)0x57, (uint8_t)0x01, (uint8_t)0x1F);
  delay(100);
  read_reg((uint8_t)0x57);
  delay(1000); // 1초 대기
}

void write_reg(uint8_t _Add, uint8_t _regAdd, uint8_t _data){
  Wire.beginTransmission(_Add);
  Wire.write(_regAdd);
  Wire.write(_data);
  Wire.endTransmission();
}

void read_reg(uint8_t _Add){
  //Wire.beginTransmission(_Add);
  //Wire.write(_regAdd);
  //Wire.endTransmission();

  Wire.requestFrom(_Add, (uint8_t)1);
  uint8_t receivedData = Wire.read();
  Serial.print("Received data: 0x");
  Serial.println(receivedData, HEX);
}
