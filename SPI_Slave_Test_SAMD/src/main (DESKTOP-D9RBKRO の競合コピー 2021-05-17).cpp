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
    SERCOM1->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt
  }
  // if (SERCOM1->SPI.INTFLAG.bit.ERROR && SERCOM1->SPI.INTENSET.bit.ERROR) {
  //   SerialUSB.println("\tOverflow Interrupt");
  //   SERCOM1->SPI.INTFLAG.bit.ERROR = 1; // clear
  // } 
  if (SERCOM1->SPI.INTFLAG.bit.RXC && SERCOM1->SPI.INTENSET.bit.RXC) 
  {
    SerialUSB.println("\tSPI Data Received Complete Interrupt");
    SerialUSB.print("\t\tRXD: ");
    // while ( SERCOM1->SPI.INTFLAG.bit.RXC ) 
    //for (uint8_t i=0; i<3; i++)
    {
      byte rxd = SERCOM1->SPI.DATA.reg;
      SerialUSB.print(rxd);
      SerialUSB.print(" ");
    }
    SerialUSB.println();
  }
  // In a slave mode, this interrupt means SS pin goes back to HIGH.
  if(SERCOM1->SPI.INTFLAG.bit.TXC && SERCOM1->SPI.INTENSET.bit.TXC)
	{
    SerialUSB.println("SPI Data Transmit Complete Interrupt");
    SERCOM1->SPI.INTFLAG.bit.TXC = 1; // Clear the flag
  }
  // Data Register Empty Interrupt
  // This interrput will be cleared by setting DATA 
  if(SERCOM1->SPI.INTFLAG.bit.DRE && SERCOM1->SPI.INTENSET.bit.DRE)
	{
    SerialUSB.println("\tSPI Data Register Empty Interrupt");
		SERCOM1->SPI.DATA.reg = rxCount++;
  }

  interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  SerialUSB.print("\tInterrupt: "); SerialUSB.println(interrupts);
}

void __SERCOM1_Handler() 
{
  SerialUSB.println("----------");
  SerialUSB.println("In SPI Interrupt");
  uint8_t interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  SerialUSB.print("\tInterrupt: "); SerialUSB.println(interrupts);
  // Slave select low interrupt
  if (SERCOM1->SPI.INTFLAG.bit.SSL && SERCOM1->SPI.INTENSET.bit.SSL) 
  {
    SerialUSB.println("\tSPI SSL Interupt");
    SerialUSB.print("\trxCount:");
    SerialUSB.println(rxCount);
    SerialUSB.print("\ttxCount:");
    SerialUSB.println(txCount);
    SERCOM1->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt
  }
  // Receive complete interrupt
  if (SERCOM1->SPI.INTFLAG.bit.RXC && SERCOM1->SPI.INTENSET.bit.RXC) 
  {
    SerialUSB.println("\tSPI Data Received Complete Interrupt");
    while ( SERCOM1->SPI.INTFLAG.bit.RXC ) {
      spiRxBuf[rxCount] = SERCOM1->SPI.DATA.reg;
      //SERCOM1->SPI.DATA.reg = spiTxBuf[txCount++];
      rxCount++;
    }
    //spiRxBuf[rxCount++] = SERCOM1->SPI.DATA.reg;
    if (rxCount >= SPI_DATA_LENGTH) 
    {
      // SERCOM1->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_RXC;
      //SERCOM1->SPI.INTFLAG.bit.RXC = 1;
      rxDone = true;
    }
  }
  if(SERCOM1->SPI.INTFLAG.bit.DRE && SERCOM1->SPI.INTENSET.bit.DRE)
	{
    SerialUSB.println("\tSPI Data Register Empty Interrupt");
		SERCOM1->SPI.DATA.reg = spiTxBuf[txCount++];
		if (txCount == SPI_DATA_LENGTH) {
      SerialUSB.println("\ttxCount max");
			SERCOM1->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_DRE;
    }
	}

  if(SERCOM1->SPI.INTFLAG.bit.TXC && SERCOM1->SPI.INTENSET.bit.TXC)
	{
    SerialUSB.println("SPI Data Transmit Complete Interrupt");
    SERCOM1->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_TXC;
  }
  
  interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  SerialUSB.print("\tInterrupt: "); SerialUSB.println(interrupts);

}

void _SERCOM1_Handler() // 25.7 Register Summary, page 454 atmel 42181, samd21
{
  static uint8_t byteCount = 0;
  SerialUSB.println("----------");
  SerialUSB.println("In SPI Interrupt");
  uint8_t data = 0;
  uint8_t interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  SerialUSB.print("\tInterrupt: "); SerialUSB.println(interrupts);

  if(interrupts & SERCOM_SPI_INTFLAG_SSL) // 8 = 1000 = SSL (Slave Slect Low)
  {
    SerialUSB.println("SPI SSL Interupt");
    byteCount = 0;
    SERCOM1->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt
    //data = SERCOM1->SPI.DATA.reg; //Read data register

    SerialUSB.print("DATA: "); SerialUSB.println(data);

    //SERCOM1->SPI.INTFLAG.bit.RXC = 1; //clear receive complete interrupt
  }

  // This is where data is received, and is written to a buffer, which is used in the main loop
  if(interrupts & SERCOM_SPI_INTFLAG_RXC) // 4 = 0100 = RXC (Rx Complete)
  {
    SerialUSB.println("SPI Data Received Complete Interrupt");

    data = SERCOM1->SPI.DATA.reg; //Read data register
    buf[0] = data; // copy data to buffer
    SerialUSB.print("DATA: ");
    SerialUSB.println(data);
    SERCOM1->SPI.INTFLAG.bit.RXC = 1; //clear receive complete interrupt
  }

  if(interrupts & SERCOM_SPI_INTFLAG_TXC) // 2 = 0010 = TXC (Tx Complete) // SERCOM_SPI_INTFLAG_TXC
  {
    SerialUSB.println("SPI Data Transmit Complete Interrupt");
    SERCOM1->SPI.INTFLAG.bit.TXC = 1; //clear receive complete interrupt
  }
  
  if(interrupts & SERCOM_SPI_INTFLAG_DRE) // 1 = 0001 = DRE (Data Register Empty)
  {
    SerialUSB.println("SPI Data Register Empty Interrupt");
    SERCOM1->SPI.DATA.reg = spiTxBuf[byteCount++]; // tx data
    //SERCOM1->SPI.INTFLAG.bit.DRE = 1;
  }



}