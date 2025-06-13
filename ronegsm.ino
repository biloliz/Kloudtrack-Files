#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>
#define SerialMon Serial
#define SerialAT  Serial1

#define MODEM_RST     5
#define MODEM_PWRKEY  4
#define MODEM_POWERON 23
#define MODEM_TX      26
#define MODEM_RX      27

#define APN       "internet"     
#define GPRS_USER " " 
#define GPRS_PASS " " 

TinyGsm modem(SerialAT);

void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialMon.println("GSM Module Test");

  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWERON, OUTPUT);

  digitalWrite(MODEM_PWRKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWERON, HIGH);
  delay(1000);

  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(1000);

  SerialMon.print("Initializing...");
  delay(1000);
  SerialMon.println(" done");

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    return;
  }
  SerialMon.println(" success");
  SerialMon.print("Connecting to APN");

  if (!modem.gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
    SerialMon.println(" fail");
    return;
 
 
  }ss
  SerialMon.println(" success");
}

void loop() {
  if (modem.isGprsConnected()) {
    SerialMon.print("GPRS is connected.");
    int16_t rssi = modem.getSignalQuality();
      SerialMon.print("Signal strength (RSSI): ");
      SerialMon.println(rssi);
  } else {
    SerialMon.println("GPRS is disconnected.");
    modem.restart();
  }
  delay(1000);
}
