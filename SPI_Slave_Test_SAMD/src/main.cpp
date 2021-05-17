// SPI Slave

#include <Arduino.h>
#include <SercomSPISlave.h>
SercomSPISlave SPISlave;


#define DEBUG // comment this line out to not print debug data on the serial bus

// initialize variables
byte buf[1];

#define SPI_DATA_LENGTH 6
volatile uint8_t spiRxBuf[SPI_DATA_LENGTH] = {255,255,255,255,255,255};
volatile uint8_t spiTxBuf[SPI_DATA_LENGTH] = {10, 11, 12, 13, 14, 15};
volatile bool rxDone = false;
volatile uint8_t rxCount = 0, txCount = 0;

void setup()
{  
  SerialUSB.begin(115200);
  SerialUSB.println("Serial started");
  SPISlave.Sercom1init();
  SerialUSB.println("Sercom1 SPI slave initialized");
}

void loop()
{
  if ( rxDone ) {
    SerialUSB.print("Received :\t");
    for (uint8_t i=0; i<SPI_DATA_LENGTH; i++) {
      SerialUSB.print(spiRxBuf[i]);
      SerialUSB.print("\t");
    }
  SerialUSB.println();
  SerialUSB.print("\trxCount:");
  SerialUSB.println(rxCount);
  SerialUSB.print("\ttxCount:");
  SerialUSB.println(txCount);
  rxCount = 0;
  txCount = 0;
  rxDone = false;
  }
}

void SERCOM1_Handler() 
{
  SerialUSB.println("----------");
  uint8_t interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  SerialUSB.print("\tInterrupt: "); SerialUSB.println(interrupts);
  if (SERCOM1->SPI.INTFLAG.bit.SSL && SERCOM1->SPI.INTENSET.bit.SSL) 
  {
    SerialUSB.println("\tSPI SSL Interupt");
    // SERCOM1->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt 1を書いてリセットされる他の割り込み(ERRORなど)が全部リセットされてしまう
    // SERCOM1->SPI.INTFLAG.reg |= SERCOM_SPI_INTFLAG_SSL; // 違う書き方、結果は上の行と同じ
  }
  if (SERCOM1->SPI.INTFLAG.bit.ERROR && SERCOM1->SPI.INTENSET.bit.ERROR) { 
    SerialUSB.println("\tERROR Overflow Interrupt");
    // SERCOM1->SPI.INTFLAG.bit.ERROR = 1; // clear
  } 
  // In a slave mode, this interrupt means SS pin goes back to HIGH.
  if(SERCOM1->SPI.INTFLAG.bit.TXC && SERCOM1->SPI.INTENSET.bit.TXC)
	{
    SerialUSB.println("\tTXC SPI Data Transmit Complete Interrupt");
    // SERCOM1->SPI.INTFLAG.bit.TXC = 1; // Clear the flag
  }
  if (SERCOM1->SPI.INTFLAG.bit.RXC && SERCOM1->SPI.INTENSET.bit.RXC) 
  {
    SerialUSB.println("\tRXC SPI Data Received Complete Interrupt");
    SerialUSB.print("\t\tRXD: ");
     //while ( SERCOM1->SPI.INTFLAG.bit.RXC ) 
    {
      byte rxd = SERCOM1->SPI.DATA.reg;
      SerialUSB.print(rxd);
      SerialUSB.print(" ");
    }
    SerialUSB.println();
  }

  // Data Register Empty Interrupt
  // This interrput will be cleared by setting 'DATA' 
  if(SERCOM1->SPI.INTFLAG.bit.DRE && SERCOM1->SPI.INTENSET.bit.DRE)
	{
    SerialUSB.println("\tDRE SPI Data Register Empty Interrupt");
		SERCOM1->SPI.DATA.reg = rxCount++;
  }
  SERCOM1->SPI.INTFLAG.reg |= SERCOM_SPI_INTFLAG_SSL | SERCOM_SPI_INTFLAG_TXC | SERCOM_SPI_INTFLAG_ERROR;
  interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  SerialUSB.print("\tInterrupt: "); SerialUSB.println(interrupts);
}
