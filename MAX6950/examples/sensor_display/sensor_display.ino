/* 
 * didn't change previous PIR and DHT sensors function
 * receive IP from serial port, end with '$', eg: "127.0.0.1$"
 * display IP, temperature and humidity on LEDs, shift 1 LED every 1 sec
*/

#include <dht.h>
#include <SPI.h> 
#include <MAX6950.h>
#include <TimerOne.h>

extern uint8_t decode[32]; //for decoding, from MAX6950.cpp
int calibrationTime = 1;        
dht DHT;
char data_buffer[50] = {0x30,0x67,0x00,decode[1],decode[2],decode[7+16],decode[0+16],decode[0+16],
decode[1],0x00,0x70,0x4f,0x00,decode[3],decode[8+16],decode[0],decode[0],0x4e,0x00,0x37,0x3e,0x00,decode[4],decode[4+16],decode[0],decode[0],'$'}; //default display: "IP 127.0.0.1 TE 38.00C HU 44.00"
int cnt_data_buffer = 0;
char IP_buffer[50] = {'1','2','7','.','0','.','0','.','1','$'};
int cnt_IP_buffer = 0;
MAX6950 m;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime;  

// output DHT11 every 1s
int dht11_last = 0, dht11_now = 0;
int temperature_interval = 3000; // 3s interval reporting

int pir_last = 0, pir_now = 0;
// 30s interval for PIR detection
// Note: On the Arduino Uno (and other ATMega based boards) an int stores a 16-bit (2-byte) value. 
// This yields a range of -32,768 to 32,767 (minimum value of -2^15 and a maximum value 
// of (2^15) - 1). 
int PIR_interval = 30000;
int pir_reported = 0;

int pirPin = 3;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;

int data_merge_interval = 0; //every # seconds, data is merged in data_buffer

/////////////////////////////
//SETUP
void setup(){
  m.init();
  m.config_reg(0x11);
  m.intensity_reg(0x0f);
  m.decode_reg(0x00);
  Timer1.initialize(1000000);         // 1s timer interrupt
  Timer1.attachInterrupt(data_display); 
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);
  pinMode(2,INPUT);
  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
  for(int i = 0; i < calibrationTime; i++){
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);
}

////////////////////////////
//LOOP
void loop(){
  dht11_now = millis();
  if (dht11_now - dht11_last > temperature_interval) {
    int chk = DHT.read11(2);
    switch (chk)
    {
    case 0:  
      Serial.print("{\"DHT11_humidity\":");
      Serial.print(DHT.humidity,1);
      Serial.print(",\"DHT11_temperature\":");
      Serial.print(DHT.temperature,1);
      Serial.print("}\n");
      break;
    case -1: 
      Serial.print("Checksum error,\t"); 
      break;
    case -2: 
      Serial.print("Time out error,\t"); 
      break;
    default: 
      Serial.print("Unknown error,\t"); 
      break;
    }
    dht11_last = millis();
  }

  if(digitalRead(pirPin) == HIGH){
    // detected motion
    digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
    
    if (!pir_reported) { 
      Serial.print("{\"PIR\":1}\n");
      pir_reported = 1;
      pir_last = millis();
    }
  }
  pir_now = millis();
  if (pir_now - pir_last > PIR_interval) {
    pir_reported = 0;
  }
  delay(100);  
}

//receive IP from serial port
void serialEvent(){
  noInterrupts();
  while(Serial.available()){
    char inChar = (char)Serial.read();
    IP_buffer[cnt_IP_buffer] = inChar;
    cnt_IP_buffer++; 
   }
   IP_buffer[cnt_IP_buffer] = '\0';
   cnt_IP_buffer = 0;
   interrupts();
}

//display info on LEDs
void data_display(){
  Serial.println("displaying...");
  noInterrupts();
  if(data_merge_interval == 4) //merge every 5 sec
  {
    data_merge_interval = 0;
    int index_IP_buffer = 0;
    int index_data_buffer = 3;
    data_buffer[0] = 0x30;
    data_buffer[1] = 0x67;
    data_buffer[2] = 0x00;  //display "IP "
    while(IP_buffer[index_IP_buffer] != '$')
    {
      if(IP_buffer[index_IP_buffer] == 0x2e) // dot
        data_buffer[index_data_buffer - 1] = data_buffer[index_data_buffer - 1] + 0x80;
      else
        {
           data_buffer[index_data_buffer] = decode[IP_buffer[index_IP_buffer] - '0'];
           index_data_buffer++;
        }
        index_IP_buffer++;
    }
    data_buffer[index_data_buffer] = 0x00;
    data_buffer[index_data_buffer + 1] = 0x70; //T
    data_buffer[index_data_buffer + 2] = 0x4f; //E
    data_buffer[index_data_buffer + 3] = 0x00;
    data_buffer[index_data_buffer + 4] = decode[int(DHT.temperature) / 10];
    data_buffer[index_data_buffer + 5] = decode[int(DHT.temperature) % 10 + 16];
    data_buffer[index_data_buffer + 6] = decode[0];
    data_buffer[index_data_buffer + 7] = decode[0];
    data_buffer[index_data_buffer + 8] = 0x4e; 
    data_buffer[index_data_buffer + 9] = 0x00;
    data_buffer[index_data_buffer + 10] = 0x37; //H
    data_buffer[index_data_buffer + 11] = 0x3e; //U
    data_buffer[index_data_buffer + 12] = 0x00;
    data_buffer[index_data_buffer + 13] = decode[int(DHT.humidity) / 10];
    data_buffer[index_data_buffer + 14] = decode[int(DHT.humidity) % 10 + 16];
    data_buffer[index_data_buffer + 15] = decode[0];
    data_buffer[index_data_buffer + 16] = decode[0];
    data_buffer[index_data_buffer + 17] = '$';
  }
  data_merge_interval ++;
  interrupts();
  if(data_buffer[cnt_data_buffer + 3] != '$')
  {
    m.set_digital(0x03,data_buffer[cnt_data_buffer]);
    m.set_digital(0x02,data_buffer[cnt_data_buffer + 1]);
    m.set_digital(0x01,data_buffer[cnt_data_buffer + 2]);
    m.set_digital(0x00,data_buffer[cnt_data_buffer + 3]);
    cnt_data_buffer++;
  }
  else
    cnt_data_buffer = 0;
}
