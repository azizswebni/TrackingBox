
#include <Arduino.h>
#include <HardwareSerial.h> //Comunicação UART com o módulo SIM808
#include "GPRS_Functions.h" //Funções de configuração do GPRS


const double LATITUDE_REFERENCE = -22.129688; //Latitude de referência para calcular o distanciamento do dispositivo
const double LONGITUDE_REFERENCE = -51.408570; //Longitude de referência para calcular o distanciamento do dispositivo
const double MAX_DISTANCE = 1000; //Distância limite usada para disparar SMS de alerta de distanciamento, medida em metros

HardwareSerial mySerial(1); //Objeto referente a comunicação UART com o módulo SIM808

const String CELLNUMBER = "+21693276754"; //Número de celular que recebe mensagens SMS de alerta referentes ao distanciamento do dispositivo

//Flag que garante só um envio de sms quando o dispositivo estiver muito longe
bool smsSent = false; 
//Tempo que é aguardado após o envio de um comando AT, usado na função "sendAT"
const int TIMEOUT_AT = 2000;

//Pino serial RX que deve ser conectado no TX do SIM808, usado para configurar a variável "mySerial"
const int RX_PIN = 3;
//Pino serial TX que deve ser conectado no RX do SIM808, usado para configurar a variável "mySerial"
const int TX_PIN = 1;

//Pino de sinalização que é ativado no mesmo tempo em que o SMS é enviado para o celular
const int pinLed = 17;
const int BAUD_RATE = 115200;

//Envia comando AT e aguarda resposta com timeout
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
  mySerial.println(response);      
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

  //Se obteve sucesso retorna true
  mySerial.print("SMS CONFIG SUCCESS");
  return true;
}

//Configuração referente ao gps
bool gpsConfig()
{
  if(sendAT("AT+CGNSPWR=1").indexOf("OK") >= 0) //Liga fonte de energia do GNSS (1 = ligar, 0 = desligar)
    if(sendAT("AT+CGPSINF=0").indexOf("OK") >= 0) //Tenta obter pela primeira vez a localização GPS
      return true;
  
  return false;
}

//Informa ao SIM808 o baud rate utilizado, é recomendado rodar este programa pela primeira vez com a velocidade de 9600
bool baudConfig()
{
  //Se obteve sucesso retorna true
  if(sendAT("AT+IPR="+String(BAUD_RATE)).indexOf("OK") >= 0){
    return true;
  }else {
    return false;
  }
      
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
    while(response == "" || mySerial.available()){
      response += char(mySerial.read()); 
    }
          

    //seta GPS
    mySerial.println("AT+CGATT=1"); 
    delay(1000);  
    response = "";
    while(response == "" || mySerial.available()){
      response += char(mySerial.read()); 
    }
          

   mySerial.println(response);
    if(response.indexOf("OK")<=0){
      return false;
    }
        

    //ativa perfil do portador
    mySerial.println("AT + SAPBR = 3,1, \"CONTYPE\", \"GPRS\""); 
    delay(1000);  
    response = "";
    while(response == "" || mySerial.available()){
       response += char(mySerial.read());  
    }
        

    if(response.indexOf("OK")<=0){
      return false;
    }
    return true;
}




void setup() 
{    
 
  
  

  //Seta o baud rate que será usado pelo monitor serial e o que será usado pelo SIM808
  serialConfig();

  mySerial.println("Waiting...");
  
  delay(3000);

  //Informa ao SIM808 o baud rate utilizado, é recomendado rodar este programa pela primeira vez com a velocidade de 9600
  if(!baudConfig()){
    mySerial.println("BAUD CONFIG ERROR");
    ESP.restart();
  }else{
    mySerial.println("BAUD CONFIG OK");
  }
      

  //Seta sms como modo texto
 /* if(!smsConfig())
  {
      //showDisplay(0, "SMS config failed!", true);
      mySerial.println("SMS config failed!");
      delay(3000);
      ESP.restart();
  }else{
    mySerial.println("SMS config ok");
  }*/

 
  

  
  if(!GSMLocationInit(mySerial))
  {        
     // showDisplay(1, "GPS config failed!", false);
      mySerial.println("GSM config failed!");
      delay(3000);
      ESP.restart();
  }else{
    mySerial.println("GSM config Ok!");
  }
//Ativa o recurso de GPS do módulo
  if(!gpsConfig())
  {        
      
      mySerial.println("GPS config failed!");
      delay(3000);
      ESP.restart();
  }else{
    mySerial.println("GPS config ok, setting modem...");
  }
 
  
  
  //Configura e inicializa o GPRS 
  /*if(!modemConfig(mySerial))
  {
      //Exibe no console e no display
      Serial.println("Modem init fail");
     // showDisplay(3, "Modem init fail", false);
      delay(5000);
      ESP.restart();
  }
  //Exibe no console e no display
  //showDisplay(3, "Modem ok", false);
  Serial.println("Modem ok"); */
}

//Calcula a distância em que o dispositivo está, levando em consideração a curvatura da terra (valor retornado em metros)
double getDistance(float lat, float lon)
{
  double dist = 60 * ((acos(sin(LATITUDE_REFERENCE*(PI/180)) * sin(lat*(PI/180)) + cos(LATITUDE_REFERENCE*(PI/180)) * cos(lat*(PI/180)) * cos(abs((lon-LONGITUDE_REFERENCE)) * (PI/180)))) * (180/PI));
  return dist*1852;
}

//Verifica se o dispositivo ultrapassou o limite de distância
bool deviceIsTooFar(float lat, float lon, String *distance)
{
  double dist = getDistance(lat, lon);

  *distance = String(dist);

  if(dist > MAX_DISTANCE)
      return true;
        
  return false;
}

bool flag = false;


















void loop() 
{
  String distance, la, lo;
  float lat, lon;  
  mySerial.println("LOOP START");
  //Tenta obter valores de GPS
  if(sim808.getGPS(&lat, &lon)) //if(getGSMLocation(mySerial, &la, &lo))  Caso o sinal GPS esteja ruim, teste utilizando a rede GSM substituindo este if  
  {    
    lat = la.toFloat();
    lon = lo.toFloat();
    mySerial.println("GPS signal ok. Values obtained successfully.");

    //Verifica se o dispositivo ultrapassa a distância limite
    if(deviceIsTooFar(lat, lon, &distance))
    {
      //Flag que permite o envio de só uma mensagem SMS
      if(!smsSent)
      {          
        //Tenta enviar SMS
        if(sim808.sendSMS(CELLNUMBER, "Device is too far!"))
          Serial.println("Device is too far - SMS sent");        
        else
          Serial.println("Device is too far - fail to send SMS");    

        //showDisplay(0, "Device is too far!", true);
        digitalWrite(pinLed, HIGH); //acende led sinalizando que o dispositivo está muito longe
        smsSent = true;
        delay(5000);
      }      
    }

    //Se foi possível obter os valores de GPS, exibe no display
   // showDisplay(0, "Latitude: " + String(lat, 6), true);
   // showDisplay(1, "Longitude: " + String(lon, 6), false);
   // showDisplay(2, "Distance: " + distance + "m", false); 
  }
  else  
  {
    
    mySerial.println("GPS signal is down. Values not obtained.");
   
  }

  flag = !flag;
  if(flag)
//    showDisplay(5, "(Refresh)", false); 
  delay(3000);

}
