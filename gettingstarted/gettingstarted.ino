#include <Arduino.h>
#include <HardwareSerial.h> //Comunicação UART com o módulo SIM808
HardwareSerial mySerial(1); 
const int TIMEOUT_AT = 2000;
const int RX_PIN = 3;
const int TX_PIN = 1;

const int BAUD_RATE = 115200;

//Envia comando AT e aguarda resposta com timeout
String sendAT(String command)
{
  String response = "";    
  mySerial.println(command); 

  delay(500);
  long int time = millis();   
  while((time+TIMEOUT_AT) > millis())
  {
    while(mySerial.available())
      response += char(mySerial.read());      
      
  }     
  return response;
}

void serialConfig()
{
  //Set console baud rate
  Serial.begin(BAUD_RATE);

  //Configura baud rate do ESP32 UART
  mySerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); //Esp32 lora 0 = RX, 22 = TX
}
bool gpsConfig()
{
  if(sendAT("AT+CGNSPWR=1").indexOf("OK") >= 0) 
    if(sendAT("AT+CGNSSEQ=RMC").indexOf("OK") >= 0) 
      return true;
  
  return false;
}

bool baudConfig()
{
  if(sendAT("AT+IPR="+String(BAUD_RATE)).indexOf("OK") >= 0)
      return true;

  return false;
}

void setup() 
{  
  String start = sendAT("AT+CGPSPWR=1");
  mySerial.println("start");
  serialConfig();
  mySerial.println("Waiting...");
  if(!baudConfig())
      ESP.restart();
  if(!gpsConfig())
  {        
      mySerial.println("GPS config failed!");
      delay(3000);
      ESP.restart();
  }


}




void loop() 
{    
  delay(2000);

   String stat = sendAT("AT+CGPSSTATUS?");
  mySerial.println("//////////////////////////////////////////////////////////////////////////////");
  mySerial.println("status");
  mySerial.println("-----------------------------------------------------------------------------");
  mySerial.println(stat);
   delay(3000);
  String DATAa =  sendAT("AT+CGPSINF=0");

    mySerial.println("//////////////////////////////////////////////////////////////////////////////");
  mySerial.println("my data 1");
  mySerial.println("-----------------------------------------------------------------------------");
  mySerial.println(DATAa);
 
  delay(3000);
  String DATA =  sendAT("AT+CGNSINF");

    mySerial.println("//////////////////////////////////////////////////////////////////////////////");
  mySerial.println("my data 2");
  mySerial.println("-----------------------------------------------------------------------------");
  mySerial.println(DATA);
 
  
}
