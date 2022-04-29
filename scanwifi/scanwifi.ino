/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "ESP8266WiFi.h"

//LISTA DE WIFI CONHECIDAS
char *wifi[]={"BUDA1","PIRAFORD"};
char *wifipass[]={"saicapeta666","soeusei"};
byte TOTAL_WIFI_CONHECIDA=2;


void setup() {
  Serial.begin(9600);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
}

void loop() {
  Serial.println("scan start");
  int x;             
  String a_conectar="NENHUMA REDE A CONECTAR AQUI";
  String a_conectar_senha="NAO TEM SENHA NENHUMA NESSA COISA";

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
//  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
//    Serial.print(n);
//    Serial.println(" networks found");

    for(x=0;x<TOTAL_WIFI_CONHECIDA;x++) {
     for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      if(WiFi.encryptionType(i)==ENC_TYPE_NONE) a_conectar=WiFi.SSID(i);
      if(WiFi.SSID(i)==wifi[x]) {
        a_conectar=WiFi.SSID(i); 
        a_conectar_senha=wifipass[x];
        x=TOTAL_WIFI_CONHECIDA+1;       
        break;
      }
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
   }
  }
  Serial.println("");
  Serial.print("SSID=");
  Serial.println(a_conectar);
  Serial.print(a_conectar_senha);
  
  Serial.println("");

  // Wait a bit before scanning again
  delay(5000);
}
