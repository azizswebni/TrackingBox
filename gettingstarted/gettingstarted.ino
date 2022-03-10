#include <SIM808.h>
#include <SoftwareSerial.h>

#define SIM_RST    5 ///< SIM808 RESET
#define SIM_RX    6 ///< SIM808 RXD
#define SIM_TX    7 ///< SIM808 TXD
#define SIM_PWR   9 ///< SIM808 PWRKEY
#define SIM_STATUS  8 ///< SIM808 STATUS

#define SIM808_BAUDRATE 4800    ///< Control the baudrate use to communicate with the SIM808 module

SoftwareSerial simSerial = SoftwareSerial(SIM_TX, SIM_RX);
SIM808 sim808 = SIM808(SIM_RST, SIM_PWR, SIM_STATUS);
// SIM808 sim808 = SIM808(SIM_RST); // if you only have the RESET pin wired
// SIM808 sim808 = SIM808(SIM_RST, SIM_PWR); // if you only have the RESET and PWRKEY pins wired

void setup() {
   simSerial.begin(SIM808_BAUDRATE);
   sim808.begin(simSerial);

   sim808.powerOnOff(true);    //power on the SIM808. Unavailable without the PWRKEY pin wired
   sim808.init();
}

void loop() {
   // whatever you need to do
}
