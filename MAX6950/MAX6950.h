// 
// FILE: max6950.h
// VERSION: 0.1.1
// PURPOSE: MAX6950 library for Arduino
//
// URL: 
//By Haotian Wang (04/09/2014)
// 

#ifndef max6950_h
#define max6950_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define MAX6950_LIB_VERSION "0.1.1"

class MAX6950
{
public:
	void config_reg(uint8_t value);
	void intensity_reg(uint8_t value);
	void scanlimit_reg(uint8_t value);
	void decode_reg(uint8_t value);
	void test_reg(uint8_t value);
	void set_digital(uint8_t num, uint8_t value);
	void set_digital_dot(uint8_t num, uint8_t value);
	void set_digital_decode(uint8_t num, uint8_t value);
	void set_digital_dot_decode(uint8_t num, uint8_t value);
	void init(void);
	void IP_display(char* str);
	void temp_hum_display(int temp, int hum);
	void clearLED(void);
	void blink(boolean value);

private:
	void SPIWrite(uint8_t address, uint8_t value);
};
#endif
//
// END OF FILE
//

