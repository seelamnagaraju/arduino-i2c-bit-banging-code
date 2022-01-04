#include <Arduino.h>

#define SDA_PIN  2
#define SCL_PIN  3

// Note: I2C SDA is open drain, this means we need to use INPUT for an SDA high logic. 
// I.e. we can't just write digitalWrite(pin, HIGH) for the SDA.
#define SDA_H     pinMode(SDA_PIN, INPUT);
#define SDA_L     pinMode(SDA_PIN, OUTPUT);
#define SCL_H     pinMode(SCL_PIN, INPUT);
#define SCL_L     pinMode(SCL_PIN, OUTPUT);

#define SDA_READ  digitalRead(SDA_PIN)

#define AT24C04_ADDR   0xA0 // 0b1010000  EEPROM 0x50
#define DS1307_ADDRESS 0xD0  ///< I2C address for DS1307
#define DS1307_CONTROL 0x07 ///< Control register
#define MAX            16

#define YEAR       0x20 // 2020
#define MONTH      0x11
#define DAY        0x26
#define WEEKDAY    0x02
#define HOUR       0x13
#define MINIT      0x24
#define SEC        0x10

#define iDelay     200

int pushButton = 2;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const unsigned char clock[7] = {SEC, MINIT, HOUR, WEEKDAY, DAY, MONTH, YEAR};

const unsigned char *s="NAGARAJU\n";
char wrArray[MAX]={"DaysOfTheWeek123"}; //DaysOfTheWeek123  NAGARAJUNAGARAJU 1234567887654321
char rdArray[MAX];

//#define RTC_EN     1
#define EEPROM_EN    1

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void setup() {
   char add=0;
   Serial.begin(115200);
   pinMode(pushButton, INPUT);
   I2CInit();
   delay(10);
   
#ifdef EEPROM_EN
  Serial.println("Writing Data to EEPROM....!");
  multiByteWrite_i2c(AT24C04_ADDR, add, wrArray,16);
  for(add=0; add<16; add++) {
      //write_i2c(AT24C04_ADDR, add, wrArray[add]);
      //delay(10);
  }
#endif

#ifdef RTC_EN 
  Serial.println("Set RTC Time....!"); 
  multiByteWrite_i2c(DS1307_ADDRESS, 0, clock, 7);
  for(add=0; add<7; add++) {
      //write_i2c(DS1307_ADDRESS, add, clock[add]);
      //delay(10);
  }
#endif
}
//----------------------------------------------------------------------------------
void loop() {
  char idata;
  int add=0;
  int buttonState = digitalRead(pushButton);
  //digitalWrite(led, buttonState);

#ifdef EEPROM_EN
  Serial.print("EEPROM Read Data: "); 
  multiByteRead_i2c(AT24C04_ADDR, add, rdArray,16);
  for(add=0; add < 16; add++) {
    idata=rdArray[add];
    Serial.print(idata); 
  }
  Serial.println(' ');
#endif
#ifdef RTC_EN 
   Serial.println("RTC Data:"); 
   readRTC();
#endif
  delay(5000);
}
//----------------------------------------------------------------------------------
void readRTC(void) {
  char add=0; 
  uint8_t ss = bcd2bin(read_i2c(DS1307_ADDRESS, add++) & 0x7F);
  uint8_t mm = bcd2bin(read_i2c(DS1307_ADDRESS, add++));
  uint8_t hh = bcd2bin(read_i2c(DS1307_ADDRESS, add++));
  uint8_t daysOfWeek = bcd2bin(read_i2c(DS1307_ADDRESS, add++));
  uint8_t d = bcd2bin(read_i2c(DS1307_ADDRESS, add++));
  uint8_t m = bcd2bin(read_i2c(DS1307_ADDRESS, add++));
  uint16_t y = bcd2bin(read_i2c(DS1307_ADDRESS, add)) + 2000U; 
  Serial.print(y, DEC);
  Serial.print('/');
  Serial.print(m, DEC);
  Serial.print('/');
  Serial.print(d, DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[daysOfWeek]);
  Serial.print(") ");
  Serial.print(hh, DEC);
  Serial.print(':');
  Serial.print(mm, DEC);
  Serial.print(':');
  Serial.print(ss, DEC);
  Serial.println();  
  return;
}
//----------------------------------------------------------------------------------
void inline sleep_us(int intDelay)
{
     delayMicroseconds(intDelay);
}
//----------------------------------------------------------------------------------
void I2CInit(void)
{
    SDA_H   
    sleep_us(iDelay);   
    SDA_H   
    sleep_us(iDelay);  
}
//----------------------------------------------------------------------------------
void start(void) 
{   
    SDA_H   
    sleep_us(iDelay); 
    SCL_H 
    sleep_us(iDelay);   
    SDA_L   
    sleep_us(iDelay); 
    SCL_L
    sleep_us(iDelay); 
}   
//----------------------------------------------------------------------------------
void stop(void)  
{   
    SDA_L   
    sleep_us(iDelay);   
    SCL_H   
    sleep_us(iDelay);   
    SDA_H   
    sleep_us(iDelay);   
    SCL_L   
    sleep_us(iDelay); 
}   
//----------------------------------------------------------------------------------
unsigned char send_byte(unsigned char c) {
  unsigned int i=0, mask=0x80, ack;
  
  for (i=0; i<8; i++) //transmits 8 bits
  {
    if(c & mask) //set data line accordingly(0 or 1)
      SDA_H //data high
    else
      SDA_L //data low
   
    SCL_H   //clock high
    sleep_us(iDelay); 
    SCL_L   //clock low
    c <<= 1;  //shift mask
    sleep_us(iDelay); 
  } // for i
  // read ack bit
  SDA_H  //release data line for acknowledge
  SCL_H  //send clock for acknowledge
  sleep_us(iDelay); 
  ack=SDA_READ; //read data pin for acknowledge
  SCL_L  //clock low
  sleep_us(iDelay); 
  SDA_L  // data low
  return (ack == 0) ? 1:0; // a low ACK bit means success
} /* i2cByteOut() */
//----------------------------------------------------------------------------------
//receives one byte of data from i2c bus
unsigned char receive_byte(unsigned char master_ack) {
  unsigned char c=0,mask=0x80;
  SCL_H 
  SDA_H
  do  //receive 8 bits
  {
     //sleep_us(iDelay);
     SCL_H //clock high
     sleep_us(iDelay);
     if(SDA_READ==1) //read data
      c |= mask;  //store data
     SCL_L  //clock low
     sleep_us(iDelay); 
     mask/=2; //shift mask
  }while(mask>0);

  if(master_ack==1)
    SDA_H //don't acknowledge
  else
    SDA_L //acknowledge

  SCL_H //clock high
  sleep_us(iDelay);
  SCL_L //clock low
  sleep_us(iDelay);
  SDA_L //data low
  return c;
}
//----------------------------------------------------------------------------------
//writes one byte of data(c) to slave device(device_id) at given address(location)
void write_i2c(unsigned char device_id, unsigned char location, unsigned char c)
{
  unsigned char slave_ack;
  start();      //starts i2c bus
  slave_ack=send_byte(device_id); //select slave device
  send_byte(location); //send address location
  send_byte(c); //send data to i2c bus
  stop(); //stop i2c bus
}   
//----------------------------------------------------------------------------------
//writes multi bytes data to slave device(device_id) at given address(location)
void multiByteWrite_i2c(unsigned char device_id, unsigned char location, unsigned char *pBuf, unsigned char len)
{
  unsigned char i,slave_ack;
  start();      //starts i2c bus
  slave_ack=send_byte(device_id); //select slave device
  send_byte(location); //send address location
  for (i=0; i<len; i++)   
  {   
       send_byte(pBuf[i]);  //send data to i2c bus
  }   
  stop(); //stop i2c bus
}      
//----------------------------------------------------------------------------------
unsigned char read_i2c(unsigned char device_id,unsigned char location)
{
  unsigned char c, slave_ack;
  start();   //starts i2c bus   
  slave_ack=send_byte(device_id); //select slave device
  send_byte(location);  //send address location      
  stop(); //stop i2c bus
  start(); //starts i2c bus   
  send_byte(device_id+1); //select slave device in read mode
  c=receive_byte(1); //receive data from i2c bus
  stop(); //stop i2c bus
  return c;
}
//----------------------------------------------------------------------------------
void multiByteRead_i2c(unsigned char device_id, unsigned char location, unsigned char *Buf, unsigned char len)   
{   
    int i;     
    start();        
    send_byte(device_id); 
    send_byte(0x00);
    start(); 
    send_byte(device_id+1);
    for (i=0; i<len; i++)   
    {         
        if (i == 15)   
           Buf[i] = receive_byte(1);              
        else   
           Buf[i] = receive_byte(0);   
    }   
    stop(); 
}         
//----------------------------------------------------------------------------------
uint8_t bin2bcd(uint8_t val) 
{ 
  uint8_t bcdVal;
  bcdVal = (val + 6 * (val / 10));
  return bcdVal;
}
//----------------------------------------------------------------------------------
uint8_t bcd2bin(uint8_t val) 
{ 
  uint8_t bcdVal;
  bcdVal = (val - 6 * (val >> 4));
  return bcdVal; 
} 
//----------------------------------------------------------------------------------