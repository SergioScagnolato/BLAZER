#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#ifndef STASSID
#define STASSID "Sscagnolato"
#define STAPSK  "saicapeta666"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host_ota = "ESP_BLAZER";   //Nome que o NodeMCU tera na rede OTA


ESP8266WebServer server(80);

const int led = LED_BUILTIN;
String incomming="c";

void handleRoot() {
  String pagina="Leitura = ";
  pagina +=  incomming;  
  server.send(200, "text/plain", pagina);
}


void leitura() {
  // envia uma string para o arduino via serial, requisitando os dados dos sensores
  Serial.println("0");   // 0 = comando de requisicao de leitura
   server.send(200, "text/plain", "solicitacao de leitura enviada");
}

void leituraz() {
  // envia uma string para o arduino via serial, requisitando os dados dos sensores
  Serial.println("z");   // 0 = comando de requisicao de leitura
   server.send(200, "text/plain", "solicitacao de leitura enviada com z");
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
   
  //DEFINIÇÃO DE IP FIXO PARA O ESP
  IPAddress ip(192,168,1,110); 
  IPAddress gateway(192,168,1,1); 
  IPAddress subnet(255,255,255,0);
  WiFi.config(ip, gateway, subnet, gateway);


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }


  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "ESP 8266 FUNCIONANDO OK");
  });

 // requisita leitura
 server.on("/leitura",leitura);
 server.on("/leituraz",leituraz);
 
 server.begin();

 //atualizador OTA
 ArduinoOTA.setPort(8266);     // Porta
 ArduinoOTA.setHostname(host_ota);  // hostname
 ArduinoOTA.begin(); 

 /**** WATCHDOG *****/
 ESP.wdtEnable(8000);
}

void loop(void) {
  server.handleClient();
  while (Serial.available()) {
   incomming=Serial.readString();
 }
 delay(500);
 ArduinoOTA.handle();   
 ESP.wdtFeed();  // RESETA O WATCHDOG
}
