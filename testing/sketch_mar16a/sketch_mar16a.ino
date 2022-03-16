#include <SoftwareSerial.h>

SoftwareSerial GSMSerial(3, 1);

void setup()
{
GSMSerial.begin(115200);               // the GPRS/GSM baud rate   
Serial.begin(115200);                 // the GPRS/GSM baud rate   
Serial.print("-------------START-----------------");
}

void loop()
{
  
if(Serial.available()){
  GSMSerial.print((char)Serial.read());
} else  if(GSMSerial.available()){
  Serial.print((char)GSMSerial.read());
}




}
