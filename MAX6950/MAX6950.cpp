// 
// FILE: MAX6950.h
// VERSION: 0.1.1
// PURPOSE: MAX6950 library for Arduino
//
// URL: 
//By Haotian Wang (04/09/2014)
// 

#include <SPI.h>
#include <string.h>
#include "MAX6950.h"
#define DECODE_REG      0x01
#define INTENSITY_REG   0x02
#define SCANLIMIT_REG   0x03
#define CONFIG_REG      0x04
#define TEST_REG        0x07
#define P0_REG          0x20
#define P1_REG			0x40

const int slaveSelectPin = 10;

//decode array 0~15 are digit and 16~31 are digit with dot
extern uint8_t decode[32] = {0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b,0x77,0x1f,0x4e,0x3f,0x4f,0x47,
0xfe,0xb0,0xed,0xf9,0xb3,0xdb,0xdf,0xf0,0xff,0xfb,0xf7,0x9f,0xce,0xbf,0xcf,0xc7};

/////////////////////////////////////////////////////
//
// PUBLIC
//

/*Register Configuration:
Default: D7 D6 D5 D4 D3 D2 D1 D0
          X  X  X  0  0  0  0  0
		 D0: 0->shutdown; 1->normal operation
		 D2: 0->slow-blinking; 1->fast-blinking
		 D3: 0->disable blink; 1->enable blink
		 D4: 0->blink timing counters are unaffected; 1->blink timing counters are cleared on the rising edge of CS
		 D5: 0->Digit data for both planes P0 and P1 are unaffected; 1->Digit data for both planes P0 and P1 are cleared on the rising edge of CS
*/
void MAX6950::config_reg(uint8_t value)
{
	SPIWrite(CONFIG_REG,value);
}

/*Register Intensity:
Default: D7 D6 D5 D4 D3 D2 D1 D0
          X  X  X  X  0  0  0  0
		0x00->0x0f: intensity from 1/16(min on) to 15/16(max on)
*/
void MAX6950::intensity_reg(uint8_t value)
{
	SPIWrite(INTENSITY_REG,value);
}

/*Register Display test:
Default: D7 D6 D5 D4 D3 D2 D1 D0
          X  X  X  X  X  X  X  0
		D0: 0->normal operation; 1->display test
*/
void MAX6950::test_reg(uint8_t value)
{
	SPIWrite(TEST_REG,value);
}

/*Register Decode:
Default: D7 D6 D5 D4 D3 D2 D1 D0
          0  0  0  0  0  0  0  0
		0x00->No decode for digits 7–0
		0x01->Hexadecimal decode for digit 0, no decode for digits 7–1
		0x07->Hexadecimal decode for digits 2–0, no decode for digits 7–3
		0xFF->Hexadecimal decode for digits 7–0
*/
void MAX6950::decode_reg(uint8_t value)
{
	SPIWrite(DECODE_REG,value);
}

/*Register Scan limit:
Default: D7 D6 D5 D4 D3 D2 D1 D0
          X  X  X  X  X  1  0  0
		0x00->Display digit 0 only
		0x01->Display digits 0 and 1
		0x02->Display digits 0 and 1 2
		0x03->Display digits 0 and 1 2 3
		0x04->Display digits 0 and 1 2 3 4
		0x05->Display digits 0 and 1 2 3 4 5
		0x06->Display digits 0 and 1 2 3 4 5 6
		0x07->Display digits 0 and 1 2 3 4 5 6 7
*/
void MAX6950::scanlimit_reg(uint8_t value)
{
	SPIWrite(SCANLIMIT_REG,value);
}

/*set digital# value (decoded)
	num: 		 3   2   1   0
	board's led: left--->right
*/
void MAX6950::set_digital(uint8_t num, uint8_t value)
{
	SPIWrite(P0_REG+num,value);
}

/*set digital# value with dot (decoded)
	num: 		 3   2   1   0
	board's led: left--->right
*/
void MAX6950::set_digital_dot(uint8_t num, uint8_t value)
{
	SPIWrite(P0_REG+num,value | 0x80);
}

/*set digital# value (none-decoded)
	num: 		 3   2   1   0
	board's led: left--->right
*/
void MAX6950::set_digital_decode(uint8_t num, uint8_t value)
{
	SPIWrite(P0_REG+num,decode[value]);
}

/*set digital# value with dot (none-decoded)
	num: 		 3   2   1   0
	board's led: left--->right
*/
void MAX6950::set_digital_dot_decode(uint8_t num, uint8_t value)
{
	SPIWrite(P0_REG+num,decode[value + 16]);
}


/*initialization of SPI
*/
void MAX6950::init(void)
{
	pinMode (slaveSelectPin, OUTPUT);
	SPI.begin();
}

/*
 * display a IP on LEDs and IP ends with '$', eg: "127.0.0.1$"
*/
void MAX6950::IP_display(char* str)
{
	//display "IP" on digital 1 and 0
	scanlimit_reg(0x01);
	set_digital(0x0,0x67);
	set_digital(0x1,0x30);
	delay(1000);
	scanlimit_reg(0x04);
	
	int cnt = 0;
	int cnt1 = 0;
	char str1[20];
	char* tmp = str;
	while(tmp[cnt1] != '$')
	{
		if(tmp[cnt1] == 0x2e)
			str1[cnt-1] = str1[cnt-1] + 16;
		else
		{
			str1[cnt] = tmp[cnt1] - '0';
			cnt++;
		}  
		cnt1++;
	}
	str1[cnt] = '$';
	int i = 0;
	while(str1[i + 3] != '$')
	{
		set_digital_decode(0x3,str1[i]);
		set_digital_decode(0x2,str1[i+1]);
		set_digital_decode(0x1,str1[i+2]);
		set_digital_decode(0x0,str1[i+3]);
		i++;
		delay(1000); 
	}
}

/*
 * display temperature and humidity on LEDs
*/
void MAX6950::temp_hum_display(int temp,int hum)
{
	//display "TE" on LEDs
	scanlimit_reg(0x02);
	set_digital(0x0,0x4f); //'E'
	set_digital(0x1,0x46); //'T'
	set_digital(0x2,0x40); //'T'
	delay(1000);
	scanlimit_reg(0x04);
	//display temperature
	set_digital_decode(0x0,0x0c); //"C"
	set_digital_decode(0x1,0x00);
	set_digital_dot_decode(0x2,(temp % 10));
	set_digital_decode(0x3,(temp / 10));
	delay(1000);
	
	//display "HU" on LEDs
	scanlimit_reg(0x01);
	set_digital(0x0,0x3e); //'U'
	set_digital(0x1,0x37); //'H'
	delay(1000);
	scanlimit_reg(0x04);
	//display humidity
	set_digital_decode(0x0,0x00);
	set_digital_decode(0x1,0x00);
	set_digital_dot_decode(0x2,(hum % 10));
	set_digital_decode(0x3,(hum / 10));
	delay(1000);
}

/*
 * clear LED
*/
void MAX6950::clearLED(void)
{
	set_digital(0x0,0x00);
	set_digital(0x1,0x00);
	set_digital(0x2,0x00);
	set_digital(0x3,0x00);
}

/*
 * blind
	true->blink starts; false->blink ends
*/
void MAX6950::blink(boolean value)
{
	if(value == true)
	{
		config_reg(0x11 | 0x08);
		SPIWrite(P1_REG,0x00);      //set P1 none and LED blinks between P0 and P1
		SPIWrite(P1_REG+1,0x00);
		SPIWrite(P1_REG+2,0x00);
		SPIWrite(P1_REG+3,0x00);
	}
	else
		config_reg(0x11);
}

/////////////////////////////////////////////////////
//
// PRIVATE
//

void MAX6950::SPIWrite(uint8_t address, uint8_t value)
{
// take the SS pin low to select the chip:
	digitalWrite(slaveSelectPin,LOW);
//  send in the address and value via SPI:
	SPI.transfer(address);
	SPI.transfer(value);
// take the SS pin high to de-select the chip:
	digitalWrite(slaveSelectPin,HIGH); 
}
//
// END OF FILE
//
