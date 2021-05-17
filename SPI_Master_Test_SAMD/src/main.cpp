// SPI Master

#include <Arduino.h>
#include <SPI.h>

#define CS_PIN  10u
#define ledPin  13u

#define SPI_DATA_LENGTH 6
uint8_t spiRxBuf[SPI_DATA_LENGTH];
uint8_t spiTxBuf[] = {100, 101, 102, 103, 104, 105};

void setup() {
  pinMode(CS_PIN, OUTPUT);
  pinMode(ledPin, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SerialUSB.begin(9600);
  digitalWrite(CS_PIN, HIGH);
}

void sendData() {
  digitalWrite(ledPin, HIGH);
  digitalWrite(CS_PIN, LOW);
  delay(5);
  #define TX_DATA_LEN 3
  for (uint8_t i=0; i<TX_DATA_LEN; i++) {
    spiRxBuf[i] = SPI.transfer(spiTxBuf[i]);
  }
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(ledPin, LOW);

  SerialUSB.print("Sent :\t\t");
  for (uint8_t i=0; i<TX_DATA_LEN; i++) {
    SerialUSB.print(spiTxBuf[i]);
    SerialUSB.print("\t");
  }
  SerialUSB.println();

  SerialUSB.print("Received :\t");
    for (uint8_t i=0; i<TX_DATA_LEN; i++) {
    SerialUSB.print(spiRxBuf[i]);
    SerialUSB.print("\t");
  }
  SerialUSB.println();
}

void loop() {
  sendData();

  delay(1000);
}