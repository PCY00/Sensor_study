#include <SoftwareSerial.h>

#define Baud_Rate         9600
#define Start_Byte        0xFF
#define Gas_Type          0x04
#define Unit              0x03
#define Of_Decimal        0x00

SoftwareSerial CO_Serial(2,3);

void read_data();
uint8_t calculate_checksum(uint8_t *data, uint8_t length);

void setup(){
  Serial.begin(9600);
  CO_Serial.begin(Baud_Rate);
  delay(1000);
}

void loop(){
  read_data();
  delay(1000);
}

void read_data(){
  // 데이터를 읽을 때까지 기다립니다.
  while (CO_Serial.available() <= 9);

  // 데이터 배열을 초기화합니다.
  uint8_t data[9];

  // 데이터를 읽습니다.
  for (int i = 0; i < 9; i++) {
    data[i] = CO_Serial.read();
  }

  // 체크섬을 계산합니다.
  uint8_t checksum = calculate_checksum(data, 7);
  
  // 수신된 데이터와 체크섬을 비교합니다.
  if (checksum != data[8]) {
    Serial.println("Checksum error");
    return;
  }

  // 데이터를 파싱하고 출력합니다.
  if (data[0] != Start_Byte) {
    Serial.println("Start error");
  } else {
    Serial.println("Start_Byte : 0xFF");
  }

  if (data[1] != Gas_Type) {
    Serial.println("Not CO error");
  } else {
    Serial.println("Gas_Type : 0x04");
  }

  if (data[2] != Unit) {
    Serial.println("ppm error");
  } else {
    Serial.println("Unit : 0x03");
  }

  if (data[3] != Of_Decimal) {
    Serial.println("Not float error");
  } else {
    Serial.println("Of_Decimal : 0x00");
  }

  uint16_t gas_concentration_value = (data[4] << 8) | data[5];
  Serial.print("ppm : ");
  Serial.println(gas_concentration_value);

  uint16_t range = (data[6] << 8) | data[7];
  if (range != 500) {
    Serial.println("Range error");
  } else {
    Serial.println("Range : 500");
  }

  Serial.print("Checksum : ");
  Serial.println(checksum);
}

// 체크섬을 계산하는 함수
uint8_t calculate_checksum(uint8_t *data, uint8_t length) {
  uint8_t checksum = 0;
  for (int i = 1; i <= length; i++) {
    checksum += data[i];
  }
  return ~checksum + 1;
}
