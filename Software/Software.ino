#include <SPI.h>

void setup() {
  pinMode(36, OUTPUT); // set the SS pin as an output
  SPI2.begin();

  Serial.begin(9600);
}

int t = 0;
int i = 0;
float voltage_out = 0;
void loop() {

  SPI2.beginTransaction(SPISettings(50000000, MSBFIRST, SPI_MODE2));         // initialize the SPI library
  //delayMicroseconds(2);
  digitalWriteFast(36, LOW);            // set the SS pin to LOW
  //delayMicroseconds(2);
  
  if (i > 1){
    i = 0;
  }
  
  SPI2.transfer16(i*65535);             // send a write command to the MCP4131 to write at registry address 0x00
  i = i+1;
  //delayMicroseconds(2);
  digitalWriteFast(36, HIGH);           // set the SS pin HIGH
  SPI2.endTransaction();
  
}
