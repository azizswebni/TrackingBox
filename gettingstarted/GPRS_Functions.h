//#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_XBEE

#include <TinyGsmClient.h> //Biblioteca que configura o modem GSM

const char apn[]  = "APN"; //YourAPN
const char user[] = "";
const char pass[] = "";
HardwareSerial Serial_SIM_Module(1);

TinyGsm sim808(Serial_SIM_Module);

bool networkConnect()
{
  Serial.print("Waiting for network...");
  if (!sim808.waitForNetwork()) 
  {
    Serial.println(" fail");
    return false;
  }
  
  Serial.println(" OK");
  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!sim808.gprsConnect(apn, user, pass)) 
  {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" OK");
  return true;

}

//Configura o modem GPRS
bool modemConfig(HardwareSerial mySerial)
{
  Serial_SIM_Module = mySerial;
  //Inicia modem
  Serial.println("Setting modem...");  
  
  if(!sim808.restart()) 
    return false;

  //Conecta na rede
  return networkConnect();
}


void loop() 
{
  String distance, la, lo;
  float lat, lon;  
   
  //Tenta obter valores de GPS
  if(sim808.getGPS(&lat, &lon)) //if(getGSMLocation(mySerial, &la, &lo))  Caso o sinal GPS esteja ruim, teste utilizando a rede GSM substituindo este if  
  {    
    lat = la.toFloat();
    lon = lo.toFloat();
    Serial.println("GPS signal ok. Values obtained successfully.");
 
    //Verifica se o dispositivo ultrapassa a distância limite
   /* if(deviceIsTooFar(lat, lon, &distance))
    {
      //Flag que permite o envio de só uma mensagem SMS
      if(!smsSent)
      {          
        //Tenta enviar SMS
        if(sim808.sendSMS(CELLNUMBER, "Device is too far!"))
          Serial.println("Device is too far - SMS sent");        
        else
          Serial.println("Device is too far - fail to send SMS");    
 
        showDisplay(0, "Device is too far!", true);
        digitalWrite(pinLed, HIGH); //acende led sinalizando que o dispositivo está muito longe
        smsSent = true;
        delay(5000);
      }      
    }*/

//Se foi possível obter os valores de GPS, exibe no display
//    showDisplay(0, "Latitude: " + String(lat, 6), true);
 //   showDisplay(1, "Longitude: " + String(lon, 6), false);
  //  showDisplay(2, "Distance: " + distance + "m", false); 
  }
  else 
  {
    //Se não foi possível obter os valores, exibe vazio
    Serial.println("GPS signal is down. Values not obtained.");
 //   showDisplay(0, "Latitude: -", true);
  //  showDisplay(1, "Longitude: -", false);
   // showDisplay(2, "Distance: -", false); 
    /*
    showDisplay(0, "Latitude: " + String(LATITUDE_REFERENCE, 6), true);
    showDisplay(1, "Longitude: " + String(LONGITUDE_REFERENCE, 6), false);
    showDisplay(2, "Distance: 328m", false); */
  }
 
//  flag = !flag;
 // if(flag)
  //  Serial.println("Refresh.");
  delay(3000);
 
}
