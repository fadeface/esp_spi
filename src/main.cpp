//
// Created by abata on 2019/01/12.
//
#include <Arduino.h>

/*
  EXIO-SPI　Test
  MCP23S17
  CS:LowActive
  MSB first
  SCK rising

  2016.10.28 created
*/

//SPI IF
#define DATAOUT 13//MOSI
#define DATAIN  12//MISO
#define SPICLOCK  14//sck
#define SLAVESELECT 15//ss

//Interrupt
#define INTPIN 5

//MCP23S17 Device OPCODE
#define HWADDW 0x40
#define HWADDR 0x41

volatile byte address = 0;
volatile byte data = 0;

//SPI 関数
byte spi_transfer(volatile byte data) {
    SPDR = data;                    // Start the transmission

    while (!(SPSR & (1 << SPIF)))   // Wait the end of the transmission
    {
    };

    return SPDR;                    // return the received byte
}

void setup() {
    //--------------------------------------------------------------
    //pinmode 設定

    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK, OUTPUT);
    pinMode(SLAVESELECT, OUTPUT);
    digitalWrite(SLAVESELECT, HIGH); //disable device

    //--------------------------------------------------------------
    //SPI設定

    // SPCR = 01010000
    //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
    //sample on rising edge of clk,system clock/2 rate (fastest)
    SPCR = 0x50;
    SPSR = 0x01; // SPI2X:1

    //--------------------------------------------------------------
    //MCP23S17 初期化

    // IOCON(0x0A):0x20
    //  BANK  :0
    //  MIRROR:0
    //  SEQOP :1
    //  DISSLW:0
    //  HAEN  :0
    //  ODR   :0
    //  INTPOL:0
    //  -     :0
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);   //send MSByte address first
    address = 0x0A;
    spi_transfer(address);   //send MSByte address first
    data = 0x20;
    spi_transfer(data);      //send MSByte DATA
    digitalWrite(SLAVESELECT, HIGH); //release chip

    // GPA -----------------------------------------------
    // Set as outputs
    // IODIRA(0x00): 1:input 0:output
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);   //send MSByte address first
    address = 0x00;
    spi_transfer(address);   //send MSByte address first
    data = 0x00;
    spi_transfer(data);      //send MSByte DATA
    digitalWrite(SLAVESELECT, HIGH); //release chip

    // GPB -----------------------------------------------
    // Set as inputs
    // IODIRB(0x01): 1:input 0:output
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);   //send MSByte address first
    address = 0x01;
    spi_transfer(address);   //send MSByte address first
    data = 0xFF;
    spi_transfer(data);      //send MSByte DATA
    digitalWrite(SLAVESELECT, HIGH); //release chip

    // Pull-up
    // GPPUB(0x0D): 1:pull-up enable 0:pull-up disable
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);   //send MSByte address first
    address = 0x0D;
    spi_transfer(address);   //send MSByte address first
    data = 0xFF;
    spi_transfer(data);      //send MSByte DATA
    digitalWrite(SLAVESELECT, HIGH); //release chip

    // 入力ピンの論理値を反転
    // IPOLB(0x03): 1:opposite 0:same
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);   //send MSByte address first
    address = 0x03;
    spi_transfer(address);   //send MSByte address first
    data = 0xFF;
    spi_transfer(data);      //send MSByte DATA
    digitalWrite(SLAVESELECT, HIGH); //release chip

    // Interrupt-on-change enable
    // GPINTENB(0x05): 1:enable 0:disable
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);   //send MSByte address first
    address = 0x05;
    spi_transfer(address);   //send MSByte address first
    data = 0xFF;
    spi_transfer(data);      //send MSByte DATA
    digitalWrite(SLAVESELECT, HIGH); //release chip

    //--------------------------------------------------------------
    //外部割込み
    pinMode(INTPIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTPIN), dataChanged, FALLING);

    Serial.begin(9600);
    Serial.println("*** MCP23S17 Interrupt Test ***");

    // LED Check
    for (int i = 0; i < 8; i++) {
        // Write data (LED)
        // GPIOA(0x12)
        digitalWrite(SLAVESELECT, LOW);
        spi_transfer(HWADDW);
        spi_transfer(0x12);
        spi_transfer(1 << i);
        digitalWrite(SLAVESELECT, HIGH);
        delay(100);
    }
    // Write data (LED)
    // GPIOA(0x12)
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);
    spi_transfer(0x12);
    spi_transfer(0x00);
    digitalWrite(SLAVESELECT, HIGH);
}

void dataChanged() {
    // Read data (SW)
    // INTCAPB(0x11)
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDR);
    spi_transfer(0x11);
    data = spi_transfer(0x00);
    digitalWrite(SLAVESELECT, HIGH);

    // Write data (LED)
    // GPIOA(0x12)
    digitalWrite(SLAVESELECT, LOW);
    spi_transfer(HWADDW);
    spi_transfer(0x12);
    spi_transfer(data);
    digitalWrite(SLAVESELECT, HIGH);
}

void loop() {
}

