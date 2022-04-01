
#include <Arduino.h>
#include <HardwareSerial.h> //Comunicação UART com o módulo SIM808
#include "GPRS_Functions.h" //Funções de configuração do GPRS
HardwareSerial mySerial(1); //Objeto referente a comunicação UART com o módulo SIM808

const int BAUD_RATE = 115200;
const int TIMEOUT_AT = 2000;

//Pino serial RX que deve ser conectado no TX do SIM808, usado para configurar a variável "mySerial"
const int RX_PIN = 16;
//Pino serial TX que deve ser conectado no RX do SIM808, usado para configurar a variável "mySerial"
const int TX_PIN = 17;
String sendAT(String command)
{
  String response = "";    
  mySerial.println(command); 

  delay(5);
  long int time = millis();   
  while((time+TIMEOUT_AT) > millis())
  {
    while(mySerial.available())
      response += char(mySerial.read());      
      
  } 
/*Serial.println("------------------------------------------------");
Serial.println(command);    
Serial.println(response);    
Serial.println("------------------------------------------------");*/
  return response;
}

void serialConfig()
{
  //Set console baud rate
  Serial.begin(BAUD_RATE);

  //Configura baud rate do ESP32 UART
  mySerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); //Esp32 lora 0 = RX, 22 = TX
}


//Configuração de mensagens SMS
bool smsConfig()
{    
  //Define modo SMS para texto (0 = PDU mode, 1 = Text mode)
  
  if(sendAT("AT+CMGF=1").indexOf("OK") < 0)
    return false;


  return true;
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

bool getGSMLocation(HardwareSerial mySerial, String *latitude, String *longitude)
{    
    int i, tempoEspera;
    String response; 
    *latitude = *longitude = "";
    
    //limpa serial (flush)
    /*response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   */

    //obtém localização
    mySerial.println("AT+CIPGSMLOC=1,1"); 
    delay(1000);  
    response = "";
   
    tempoEspera = 0;
    while(response.indexOf("CIPGSMLOC")<=0 && tempoEspera<5)
    {
      if(mySerial.available())
      response += char(mySerial.read());   
      tempoEspera++;
      delay(1000);
    }

    while(mySerial.available())
         response += char(mySerial.read()); 
         
    if(response.indexOf("OK")<=0 || response.indexOf("CIPGSMLOC")<=0)
        return false;

    for(i = 0; i<response.length() && response.charAt(i)!=','; i++);

    for(i++; i<response.length() && response.charAt(i)!=','; i++)  
        *longitude+=response.charAt(i);

    for(i++; i<response.length() && response.charAt(i)!=','; i++)  
        *latitude+=response.charAt(i);

    if(*latitude == "" || *longitude =="")
        return false;

    return true;
}

bool GSMLocationInit(HardwareSerial mySerial)
{  
    String response;    

    //limpa serial (flush)
    response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   

    //seta GPS
    mySerial.println("AT + CGATT = 1"); 
    delay(1000);  
    response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   

    if(response.indexOf("OK")<=0)
        return false;

    //ativa perfil do portador
    mySerial.println("AT + SAPBR = 3,1, \"CONTYPE\", \"GPRS\""); 
    delay(1000);  
    response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   

    if(response.indexOf("OK")<=0)
        return false;

    return true;
}
/*void sendLocationToserver(float lat ,float lon){


   Serial.println(sendAT("AT+CIPSHUT"));
}*/
void setup() 
{    

  serialConfig();

  Serial.println("Waiting...");
  delay(3000);

if(!baudConfig())
      ESP.restart();
      
 if(!smsConfig())
  {
      Serial.println("SMS config failed!");
      delay(3000);
      ESP.restart();
  }

  Serial.println("SMS config ok");
//Configura e inicializa o GPRS 
  if(!modemConfig(mySerial))
  {
      Serial.println("Modem init fail");
      delay(5000);
      ESP.restart();
  }
  Serial.println("Modem ok"); 
  if(!GSMLocationInit(mySerial))
  {        
      Serial.println("GPS config failed!");
      delay(3000);
      ESP.restart();
  }

  if(!gpsConfig())
  {        
      Serial.println("GPS config failed!");
      delay(3000);
      ESP.restart();
  }
  Serial.println("GPS config ok");
  Serial.println(sendAT("AT"));
  Serial.println(sendAT("AT+CPIN?"));
  Serial.println(sendAT("AT+CREG?"));
  Serial.println(sendAT("AT+CGATT?"));
  Serial.println(sendAT("AT+CIPSHUT"));
  Serial.println(sendAT("AT+CIPSTATUS"));
  Serial.println(sendAT("AT+CIPMUX=0"));
  Serial.println(sendAT("AT+CSTT?"));
  Serial.println(sendAT("AT+CIPSPRT=0"));
}

void loop() 
{
Serial.println("-------------status------------");
Serial.println(sendAT("AT+CGPSSTATUS?"));
  float lat, lon; 
  if(sim808.getGPS(&lat, &lon)) 
  {    
    Serial.println("GPS signal ok. Values obtained successfully.");
  }
  else  
  {
    Serial.println("GPS signal is down. Values not obtained.");
  }
  String data1= sendAT("AT+CGPSINF=0");
  String data2= sendAT("AT+CGNSINF");
  Serial.println(lat,6);
  Serial.println(lon,6);
  Serial.println(sendAT("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\""));
  Serial.println(sendAT("AT+CIPSEND"));
  
  String str = "GET https://api.thingspeak.com/update?api_key=480G428YIEA6LCQW&field1=" + String(lat)+"&field2="+String(lon);
   Serial.println(str);
   mySerial.println(str);
   mySerial.println((char)26);
   Serial.println("waiting for response");
   }
