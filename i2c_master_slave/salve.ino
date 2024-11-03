#include <Wire.h>

#define LED           13

// Slave address
#define SLAVE_ADDRESS 0x57

// Register addresses
#define WRITE_REGISTER 0x01
#define READ_REGISTER  0x02

// LED control
#define LED_ON         0xAC
#define LED_OFF        0x1F

bool ledState = false;

void setup() {
  pinMode(LED, OUTPUT);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Do any other tasks here if needed
  delay(100);
}

void receiveEvent() {
  int registerAddress = Wire.read();
  if (registerAddress == WRITE_REGISTER) {
    int value = Wire.read();
    if (value == LED_ON){
      digitalWrite(LED, HIGH);
      ledState = true;
    }
    else if (value == LED_OFF){
      digitalWrite(LED, LOW);
      ledState = false;
    }
  }
}

void requestEvent() {
   int registerAddress = Wire.read(); // Read the requested register address
   if (registerAddress == READ_REGISTER) {
      Wire.write(ledState ? LED_ON : LED_OFF); // Send current LED state
   }
}
