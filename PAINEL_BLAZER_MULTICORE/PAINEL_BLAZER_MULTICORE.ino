
// 10/12/2020
// 14/12/2020 - acrescentado opcao de scan wifi
// 15/12/2020 - correcao no relogio
// 15/12/2020 - botao soft-reset no webserver
// 15/12/2020 - pino remoto sobrepoe wifi para colocar em standby
// 15/12/2020 - relogio no core 0

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <WiFiClient.h>
#include <ESP8266mDNS.h>


//SSID e Password do ESP
const char* ssid = "BLAZER";
const char* password = "soeusei1";
const byte canal_wifi = 2;
const byte max_connection=4;
const char* host_ota = "ESP_Blazer";
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress ST_IP(192,168,1,96); 
IPAddress ST_gateway(192,168,1,1); 
IPAddress ST_subnet(255,255,255,0);


#define MODOWIFI WIFI_PHY_MODE_11B     // maior alcance, 11 Mbps  - celular encontra na pesquisa de redes wifi
//#define MODOWIFI WIFI_PHY_MODE_11G   // menor alcance, 54 Mbps - celular nao encontra na pesquisa de wifi, so o notebook
//#define MODOWIFI WIFI_PHY_MODE_11N   


//LISTA DE WIFI CONHECIDAS
char *wifi[]={"BUDA2","PIRAFORD"};
char *wifipass[]={"saicapeta666","soeusei"};
byte TOTAL_WIFI_CONHECIDA=2;
String a_conectar="NENHUMA REDE A CONECTAR AQUI";
String a_conectar_senha="NENHUMA SENHA DISPONIVEL AQUI PRA CONECTAR";

// portas do controlador
const int RELE_IGNICAO   = D6;
const int RELE_PARTIDA   = D5;
//const int LED_VERDE      = D6;
//const int LED_VERMELHO   = D7;
const int PORTA1_FG      = D1;
const int PORTA2_FG      = D2; 
const int PORTA1_DSP     = 1;
const int PORTA2_DSP     = 3;
const int SINAL_RPM      = D7;
const int SINAL_REMOTO   = D0;

int ESPACO_LINHAS=11;           // linha a pular na inicializacao;
int linha_init=13;
int xdelay;

ESP8266WebServer server(80); //Server on port 80

SoftwareSerial mySerial(PORTA1_FG, PORTA2_FG);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//Pinos do NodeMCU
// SDA => RX (3)
// SCL => TX  (1)
// Inicializa o display Oled
SSD1306Wire  display(0x3c, PORTA1_DSP, PORTA2_DSP);

byte acesso,acessoweb=0;
byte estadoIgnicao=0;
byte leitorok=1;
uint8_t p,p1;

int hora,minuto,ano,status_stdy,rpm=0;
long umsegundo=1000;    
long intervalo_rpm=250;
unsigned long previousMillis,previousstd=0;
float segundo=0;

int DEBUG=0;         // para debug


// NTP
String formattedDate;
String timeStamp;
const long utcOffsetInSeconds = -10800;      // fuso horario gmt-3 (gmt-1 = -3600)
// Cliente NTP 
WiFiUDP ntpUDP;
// pool de ntp a conectar
//NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utcOffsetInSeconds);
NTPClient timeClient(ntpUDP, "200.160.7.186", utcOffsetInSeconds);

/***************** SETUP *********************/
void setup(void) {
 if(DEBUG==1) {
  Serial.begin(9600);
  Serial.println("");
  Serial.println("");
 }

 pinMode(RELE_IGNICAO, OUTPUT);
 pinMode(RELE_PARTIDA, OUTPUT);
 //pinMode(LED_VERDE, OUTPUT);
 //pinMode(LED_VERMELHO, OUTPUT);
 pinMode(SINAL_REMOTO,INPUT);
 pinMode(SINAL_RPM,INPUT);
 
 digitalWrite(RELE_IGNICAO, HIGH);
 digitalWrite(RELE_PARTIDA, HIGH);
 //digitalWrite(LED_VERDE, LOW);
 //digitalWrite(LED_VERMELHO, HIGH);

 
  display.init();
  display.flipScreenVertically();

 //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //Seleciona a fonte
  display.setFont(ArialMT_Plain_16);
  display.setColor(WHITE);
//  display.drawString(63, 13, "BLAZER");
//  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 17, "Sergio");
  display.drawString(60, 39, "Scagnolato");
  display.display();
  delay(1500);

  display.setFont(ArialMT_Plain_10);
  display.clear();
 //atualizador OTA
 ArduinoOTA.setPort(8266);     // Porta
 ArduinoOTA.setHostname(host_ota);  // hostname
 ArduinoOTA.begin(); 
 display.drawString(44, linha_init, "OTA.....");
 linha_init+=ESPACO_LINHAS;
 
 finger.begin(57600);
 delay(10);

// verifica se o leitor bio esta conectado
 if (finger.verifyPassword()) {
  leitorok=1;
  if(DEBUG==1) Serial.println("Leitor conectado");
  display.drawString(44, linha_init, "Leitor conectado");
  
 } else {
  leitorok=0;
  if(DEBUG==1) Serial.println("Leitor nao encontrado");
  display.drawString(44, linha_init, "** Leitor nao encontrado **");
 }

 linha_init+=ESPACO_LINHAS;
 finger.getTemplateCount();
 int cadastros=finger.templateCount;
 if(DEBUG==1) {
  Serial.println(" ");
  Serial.print(cadastros);
  Serial.print(" cadastros no sensor");
  Serial.println(" ");
 }
 display.drawString(44, linha_init, String(cadastros));
 display.drawString(50, linha_init, "Cadastros");
 display.display();

 linha_init+=ESPACO_LINHAS;
 
 finger.LEDcontrol(false);
 delay(500);

 display.clear();
 display.display();
 linha_init=13;
 acerta_relogio(1);

  WiFi.mode(WIFI_AP);           // Apenas AP
//  WiFi.mode(WIFI_AP_STA);         // AP + ST
// WiFi.setPhyMode(MODOWIFI);
 IPAddress ip(192, 168, 4, 1);
 IPAddress gateway(192, 168, 4, 1);
 IPAddress subnet(255, 255, 255, 0);
 WiFi.config(ip, gateway, subnet);
 boolean result=WiFi.softAP(ssid, password,canal_wifi,false,max_connection);  // Inicia HOTspot

 display.setFont(ArialMT_Plain_10);
 if(result==true) {
  if(DEBUG==1) Serial.println("SoftAP OK");
 } else {
  if(DEBUG==1) Serial.println("Erro no SoftAP");
 } 
 display.display(); 
 
//  wifi_softap_dhcps_start();    //inicia DHCP sercer
   
   IPAddress myIP = WiFi.softAPIP(); //Get IP address
   if(DEBUG==1) {
    Serial.println(" ");
    Serial.print("HotSpt IP:");
    Serial.println(myIP);
    Serial.println(ssid);
    Serial.println(password);
   }
 
 server.on("/", handleRoot);                              // pagina principal
 server.on("/ignicao/on", ligarIgnicao);                  // liga ign
 server.on("/ignicao/off", desligarIgnicao);              // desliga ign
 server.on("/start", partida); 
 server.on("/reset", reset_esp); 

 server.onNotFound(handle_NotFound);

 server.begin();  
 display.drawString(44, linha_init, "WebServer iniciado");
 display.display();
 linha_init+=ESPACO_LINHAS;

 if(DEBUG>0) {
  Serial.print("Horario ");
  Serial.print(timeStamp);
  Serial.println("");
  Serial.print(hora);
  Serial.print(":");
  Serial.print(minuto);
  Serial.print(":");
  Serial.print(segundo);
  Serial.println("");
 }
 delay(xdelay);
 display.clear();
 display.setFont(ArialMT_Plain_24);

//Cria uma nova tarefa no core 0
    xTaskCreatePinnedToCore(
        atualiza_relogio,     //Função que será executada
        "atualiza relogio",   //Nome da tarefa
        10000,                  //Tamanho da memória disponível (em WORDs)
        NULL,                   //Não vamos passar nenhum parametro
        2,                      //prioridade
        NULL,                   //Não precisamos de referência para a tarefa
        0);                     //Núm
 
}


/********************* LOOP ********************/
void loop() {
 server.handleClient();  //Handle client requests
 MDNS.update();

 int remoto=digitalRead(SINAL_REMOTO);
 
 // relogio 

 //atualiza_relogio();
 
 if(remoto==1 or acessoweb==1) {    
  status_stdy=0;
  
  if(remoto==1) acessoweb=0;
  
  mostra_hora();

  if(acesso==1) {
   int tick=0;
   int x=0;

   unsigned long currtime_rpm=millis();
   while(millis() -currtime_rpm <= intervalo_rpm) {
    if(digitalRead(SINAL_RPM)==LOW and x==0) {tick++; x=1;}
    if(digitalRead(SINAL_RPM)==HIGH) x=0;
   }
   rpm=(tick/4)*15;    // 4 pulsos no neg da bobina a cada volta

   int porc=100*rpm/6500;   // max rpm=6500;

   display.drawProgressBar(0, 42, 126, 10, porc);
   //Atualiza a porcentagem completa
   display.setTextAlignment(TEXT_ALIGN_CENTER);
   display.setFont(ArialMT_Plain_10);
   display.drawString(64, 53, String(rpm));
   display.setFont(ArialMT_Plain_24);
//   display.drawString(66, 41, String(rpm));
   display.display();
  }

 //  server.handleClient();  //Handle client requests
 
 if(leitorok==1) { 
  if(acesso==0 and acessoweb==0) {
    
   display.setFont(ArialMT_Plain_10);
   display.drawString(64, 45, "Aguardando Biometria");
   display.display();

   finger.LEDcontrol(true);
   p = finger.getImage();
   if(p==FINGERPRINT_OK) {
    p1 = finger.image2Tz();
    while(p1!=FINGERPRINT_OK) {
     p1 = finger.image2Tz();
     delay(10);
    }
    p1 = finger.fingerSearch();
    if(p1==FINGERPRINT_NOTFOUND) {    // nao encontrou digital
     if(DEBUG==1) Serial.println("NAO AUTORIZADO");
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.drawString(60, 20, "NAO");
      display.drawString(60, 44, "AUTORIZADO");
      display.display();
      delay(1200);
    }
    if(p1==FINGERPRINT_OK) { 
      if(DEBUG==1) Serial.println("ACESSO AUTORIZADO");
      
      finger.LEDcontrol(false);
      acesso=1;
    }
   }
//   delay(100);
  } else {    // cadastro reconhecido
   estadoIgnicao=1;
   //digitalWrite(LED_VERDE,HIGH);
   //digitalWrite(LED_VERMELHO,LOW);
   digitalWrite(RELE_IGNICAO,LOW);
 
  }
 }
} else {
  estadoIgnicao=0;
  acesso=0;
  if(status_stdy==0) {
   finger.LEDcontrol(false);
   digitalWrite(RELE_IGNICAO,HIGH);
   display.clear();
   display.setFont(ArialMT_Plain_16);
   display.drawString(64, 22, "Stand By");
   display.display();
   status_stdy=1;
  }
}
 
ArduinoOTA.handle(); 
delay(10);
}



//==============================================================
//     PAGINA PRINCIPAL
//==============================================================
void handleRoot() {
 String MAIN_page="<!DOCTYPE html><html>";
 MAIN_page+="<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
 //MAIN_page+="<link rel=icon href=data:,>";
 MAIN_page+="<style>";
 MAIN_page+=" html { ";
 MAIN_page+="font-family: Sans-serif; ";
 MAIN_page+="    display: inline-block; ";
 MAIN_page+="margin: 0px auto; ";
 MAIN_page+="text-align: ";
 MAIN_page+="center;";
 MAIN_page+="   }";
 MAIN_page+=" .button {";
 MAIN_page+="    background-color: #195B6A; ";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 26px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".button:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+=" .buttonpequeno {";
 MAIN_page+="    background-color: #195B6A; ";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 14px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".buttonpequeno:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+=" .mais {";
 MAIN_page+="   background-color: #77878A;";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 18px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".mais:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+="</style></head><body>";
 MAIN_page+="\n\n<script>\n";
 MAIN_page+="function butaoon() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/ignicao/on';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";
 MAIN_page+="\n\n<script>\n";
 MAIN_page+="function butaooff() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/ignicao/off';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n";
 MAIN_page+="\n<script>\n";
 MAIN_page+="function butaostart() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/start';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n";
 MAIN_page+="\n<script>\n";
 MAIN_page+="function butaoreset() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/reset';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";
 
 MAIN_page+="<h3>Blazer - Versao - 2.1</h3>";

 if(estadoIgnicao==0)
  MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaoon()'>LIGAR IGNICAO</button></p>";
 else
  MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaooff()'>DESLIGAR IGNICAO</button></p>";

  if(estadoIgnicao==1)
    MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaostart()'>PARTIDA</button></p>";
  

    MAIN_page+="\n<br><br><br><br>\n<p><button class=button id=botaoabrir onclick='butaoreset()'>Reset</button></p>";
 

 MAIN_page+="\n<br>\n</body>\n</html>";

 server.send(200, "text/html", MAIN_page);
}

void ligarIgnicao() {
 estadoIgnicao=1;
 acesso=1;
 acessoweb=1;
 digitalWrite(RELE_IGNICAO,LOW);
 if(leitorok==1) {
//  leitorok=0;
  finger.LEDcontrol(false);
 }
 handleRoot();
}

void desligarIgnicao() {
 estadoIgnicao=0;
 acessoweb=0;
 digitalWrite(RELE_IGNICAO,HIGH);
 if(leitorok==1) {
//  leitorok=0;
  acesso=0;
  finger.LEDcontrol(true);
 }
 handleRoot();
}

void reset_esp() {
 ESP.restart(); 
 handleRoot();
}



void partida() {
 digitalWrite(RELE_PARTIDA,LOW);
 delay(1500);
 digitalWrite(RELE_PARTIDA,HIGH);
 handleRoot();
}

void handle_NotFound(){
  server.send(404, "text/plain", "Pagina non ecxiste");
}


void acerta_relogio(int status) {
 int x;
 byte ja_achou=0;
 WiFi.mode(WIFI_STA);
 WiFi.disconnect();
 delay(100);
 if(status==1) {
  display.drawString(44, linha_init, "Buscando WIFI");
  display.display();
  linha_init+=ESPACO_LINHAS;
 }
 
 // scan nos wifi proximos
 int nro_wifi = WiFi.scanNetworks();
 if (nro_wifi == 0) {
  if(status==1) {
   display.drawString(44, linha_init, "*** Nenhum WIFI ***");
   display.display();
   linha_init+=ESPACO_LINHAS;
  }
  if(DEBUG==1) Serial.println("-----> NENHUM WIFI ENCONTRADO");
  hora=minuto=segundo=0;
 } else {
  
  for(x=0;x<TOTAL_WIFI_CONHECIDA;x++) {
   for (int i = 0; i < nro_wifi; ++i) {
    // Print SSID and RSSI for each network found
    if(WiFi.encryptionType(i)==ENC_TYPE_NONE and ja_achou==0) {a_conectar=WiFi.SSID(i); a_conectar_senha=""; ja_achou=1;}
    if(WiFi.SSID(i)==wifi[x]) {
      a_conectar=WiFi.SSID(i); 
      a_conectar_senha=wifipass[x];
      x=TOTAL_WIFI_CONHECIDA+1;       
      break;
    }
   }
  }
 }
 if(nro_wifi>0) {
  WiFi.begin(a_conectar, a_conectar_senha);
//  WiFi.config(ST_IP, ST_gateway, ST_subnet,ST_gateway);  
  if(DEBUG==1) Serial.println("Conectando wifi");
  if(status==1) {
   display.drawString(44, linha_init, a_conectar);
   display.display();
   //linha_init+=ESPACO_LINHAS;
  }
  int vezes=0;
  int coluna=100;
  while ( WiFi.status() != WL_CONNECTED and vezes<=10) {
   if(DEBUG==1) Serial.print(".");
   delay(120);
   if(status==1) {
    display.drawString(coluna, linha_init, ".");
    coluna++;
    display.display();
   }
   vezes++;
  }
  if(status==1)  linha_init+=ESPACO_LINHAS;
  vezes=10;
  while(!timeClient.update() and vezes>0) {
    timeClient.forceUpdate();
    vezes--;
  }
  if(vezes>0) {   
   formattedDate = timeClient.getFormattedDate();
   int splitT = formattedDate.indexOf("T");
   timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
   hora    = timeStamp.substring(0,2).toInt();
   minuto  = timeStamp.substring(3,5).toInt();
   segundo = timeStamp.substring(6,8).toInt();
  } else {
    hora=minuto=segundo=0;
  }
  WiFi.disconnect();

  display.setFont(ArialMT_Plain_10);
  if(vezes>0) {
   display.drawString(44, linha_init, "NTP OK");
   xdelay=300;
  } else {
   display.drawString(44, linha_init, "** ERRO NTP **");
   xdelay=1100;
  }
 } else {
  hora=minuto=segundo=0;
 }
 display.display();
 delay(xdelay);
 if(status==1) linha_init += ESPACO_LINHAS;
} 


void atualiza_relogio() {
  // relogio 
 unsigned long currentMillis = millis();    //Tempo atual em ms
 if (currentMillis - previousMillis > umsegundo) {
  segundo += ((currentMillis - previousMillis)/1000.0);
//  segundo++;
  previousMillis=millis();
  
  if(segundo>=60.0) {
    segundo=0;
    minuto++;
  }
  if(minuto>59) {
   minuto=0;
   hora++;
  }
  if(hora>23){
   hora=0;
   minuto=0;
  }
 }
}

void mostra_hora() {
    display.clear();
  char horario[5];
  sprintf(horario,"%02d:%02d",hora,minuto);
  display.setFont(ArialMT_Plain_24);
  display.drawString(65, 11, horario);
 // display.display();
}
