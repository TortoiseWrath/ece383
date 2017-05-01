#include "pic24_all.h"
#include <stdio.h>

//CS on RB5 for DAC
#define CONFIG_SLAVE_ENABLE() CONFIG_RB5_AS_DIG_OUTPUT()
#define SLAVE_ENABLE()        _LATB5 = 0  //low true assertion
#define SLAVE_DISABLE()       _LATB5 = 1

//DAC functions from Task 2

void configSPI1(void) {
  //spi clock = 40MHz/1*4 = 40MHz/4 = 10MHz
  SPI1CON1 = SEC_PRESCAL_1_1 |     //1:1 secondary prescale
             PRI_PRESCAL_4_1 |     //4:1 primary prescale
             CLK_POL_ACTIVE_HIGH | //clock active high (CKP = 0)
             SPI_CKE_ON          | //out changes active to inactive (CKE=1)
             SPI_MODE8_ON        | //8-bit mode
             MASTER_ENABLE_ON;     //master mode
#if (defined(__dsPIC33E__) || defined(__PIC24E__))
  //nothing to do here. On this family, the SPI1 port uses dedicated
  //pins for higher speed. The SPI2 port can be used with remappable pins.
  //you may need to add code to disable analog functionality if the SPI ports
  //are on analog-capable pins.
#else
  CONFIG_SDO1_TO_RP(6);      //use RP6 for SDO
  CONFIG_SCK1OUT_TO_RP(7);   //use RP7 for SCLK
#endif

  SPI1STATbits.SPIEN = 1;  //enable SPI mode
}
void configDAC() {
  CONFIG_SLAVE_ENABLE();       //chip select for DAC
  SLAVE_DISABLE();             //disable the chip select
}

void writeDAC (uint8_t dacval) {
  SLAVE_ENABLE();                 //assert Chipselect line to DAC
  ioMasterSPI1(0b00001001);      //control byte that enables DAC A
  ioMasterSPI1(dacval);          //write DAC value
  SLAVE_DISABLE();
}

//EEPROM configuration from Task 3
#define EEPROM 0xA0   //LC515 address assuming both address pins tied low.

//Assumes WDT is configured for longer than EEPROM write time
void waitForWriteCompletion(uint8_t u8_i2cAddr) {
  uint8_t u8_ack, u8_savedSWDTEN;
  u8_savedSWDTEN = _SWDTEN;
  _SWDTEN = 1; //enable WDT so that do not get stuck in infinite loop!
  u8_i2cAddr = I2C_WADDR(u8_i2cAddr);  //write operation, R/W# = 0;
  do {
    startI2C1();
    u8_ack = putNoAckCheckI2C1(u8_i2cAddr);
    stopI2C1();
  } while (u8_ack == I2C_NAK);
  _SWDTEN = u8_savedSWDTEN;  //restore WDT to original state
}

void memWriteLC515(uint8_t u8_i2cAddr,  uint16_t u16_MemAddr, uint8_t *pu8_buf) {
  uint8_t u8_AddrLo, u8_AddrHi;

  u8_AddrLo = u16_MemAddr & 0x00FF;
  u8_AddrHi = (u16_MemAddr >> 8);
  pu8_buf[0] = u8_AddrHi;   //place address into buffer
  pu8_buf[1] = u8_AddrLo;

  if (u16_MemAddr & 0x8000) {
    // if MSB set , set block select bit
    u8_i2cAddr = u8_i2cAddr | 0x08;
  }
  waitForWriteCompletion(u8_i2cAddr);
  writeNI2C1(u8_i2cAddr,pu8_buf,3);
}

void memReadLC515(uint8_t u8_i2cAddr,  uint16_t u16_MemAddr, uint8_t *pu8_buf) {

  uint8_t u8_AddrLo, u8_AddrHi;

  u8_AddrLo = u16_MemAddr & 0x00FF;
  u8_AddrHi = (u16_MemAddr >> 8);

  if (u16_MemAddr & 0x8000) {
    // if MSB set , set block select bit
    u8_i2cAddr = u8_i2cAddr | 0x08;
  }
  waitForWriteCompletion(u8_i2cAddr);
  //set address counter
  write2I2C1(u8_i2cAddr,u8_AddrHi, u8_AddrLo);
  //read data
  readNI2C1(u8_i2cAddr,pu8_buf, 1);
}


//Interrupt timer configuration

#define ISR2_PERIOD     100                // in ms
void  configTimer2(void) {
  //ensure that Timer2,3 configured as separate timers.
  T2CONbits.T32 = 0;     // 32-bit mode off
  //T3CON set like this for documentation purposes.
  //could be replaced by T3CON = 0x0020
  T2CON = T2_OFF |T2_IDLE_CON | T2_GATE_OFF
          | T2_SOURCE_INT
          | T2_PS_1_64 ;  //results in T3CON= 0x0020
  PR2 = msToU16Ticks (ISR2_PERIOD, getTimerPrescale(T2CONbits)) - 1;	
  TMR2  = 0;                       //clear timer3 value
  _T2IF = 0;                       //clear interrupt flag
  _T2IE = 1;					   //enable the interrupt
  _T2IP = 2;                       //choose a priority
  T2CONbits.TON = 1;               //turn on the timer
}


#define VREF 3.3  //assume Vref = 3.3 volts

volatile uint16_t u16_adcVal; //Store value read from ADC
volatile uint8_t u8_dacVal; //Store value output to DAC
volatile float f_adcVal; //Store ADC value as float
volatile float f_dacVal; //Store DAC value as float

volatile uint16_t count = 0; //Number of interrupts performed
#define MAXCOUNT 600 //up to 600 readings

volatile uint8_t au8_buf[3];  //2 bytes for address and 1 byte for data
volatile uint16_t u16_MemAddr = 0; //Start at location 0 in EEPROM

//ISR for Timer 2 (sample every 100 ms)
void _ISRFAST _T2Interrupt (void) {
    u16_adcVal = convertADC1();   //get ADC value
    u8_dacVal = (u16_adcVal>>4) & 0x00FF;  //upper 8 bits to DAC value
    writeDAC(u8_dacVal);
    f_adcVal = u16_adcVal;
    f_adcVal = f_adcVal/4096.0 * VREF;  //convert to float 0.0 to VREF
    f_dacVal = u8_dacVal;
    f_dacVal = f_dacVal/256.0 * VREF;
	
    printf("ADC in: %4.3f V (0x%04x), To DAC: %4.3f V (0x%02x) \n",
           (double) f_adcVal, u16_adcVal, (double) f_dacVal, u8_dacVal);
		   
	if (count % 5 == 0) { //every 500 ms
		printf("Storing value 0x%02x in EEPROM at address 0x%04x. \n",
			u8_dacVal, u16_MemAddr);
		au8_buf[2] = u8_dacVal; //copy 8-bit value to EEPROM char buffer
		memWriteLC515(EEPROM,u16_MemAddr, au8_buf); // do write
		u16_MemAddr++; //move to next address
	}
	
	count++; //increment count (up to MAXCOUNT)
	_T2IF = 0;
}

int main (void) {
	float f_eepromVal; //store read value as float

	configClock();                //clock configuration
	configDefaultUART(9600);      //serial port config
	outString(HELLO_MSG);         //say Hello!
	printResetCause();

	//ADC configuration
	CONFIG_AN0_AS_ANALOG();
	// Configure A/D to sample AN0 for 31 Tad periods in 12-bit mode
	// then perform a single conversion.
	configADC1_ManualCH0(ADC_CH0_POS_SAMPLEA_AN0, 31, 1);
	
	//DAC configuration
	configSPI1();
	configDAC();
	
	//EEPROM configuration
	configI2C1(400);            //configure I2C for 400 KHz
	
	//Stage 1 begins
	configTimer2();                       //enable the interrupt
	while (count < MAXCOUNT); //Let the interrupt do the work
	
	//Stage 2 begins
	_T2IE = 0; //disable the interrupt
	outString("Reading values back from EEPROM \n");
	u16_MemAddr = 0; //go back to start of EEPROM
	while (u16_MemAddr < MAXCOUNT/5) { //output values from EEPROM
		memReadLC515(EEPROM,u16_MemAddr,au8_buf); // read back value into au8_buf
		f_eepromVal = au8_buf[0];
		f_eepromVal = f_eepromVal/256.0 * VREF; //convert to float 0.0 to VREF
		
		//Output to console
		printf("Stored value at 0x%04x: %4.3f V (0x%02x) \n ",
			u16_MemAddr, (double) f_eepromVal, au8_buf[0]);
			
		u16_MemAddr++; //next address
	}

	//end
}
