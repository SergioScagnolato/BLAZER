

// ********* VERSAO PARA LCD - OLD *************


// 10/12/2020
// 14/12/2020 - acrescentado opcao de scan wifi
// 15/12/2020 - correcao no relogio
// 15/12/2020 - botao soft-reset no webserver
// 15/12/2020 - pino remoto sobrepoe wifi para colocar em standby
// 15/12/2020 - substituir funcao delay no loop por millis
// 25/10/2021 - opcao de relogio decimal ou binario
// 22/11/2021 - modo 'normal' e 'manutencao'
// 01/12/2021 - desliga o wifi quando entra em stdby (desabilitado opcao - nao faz diferenca no consumo)
// 06/12/2021 - temperatura atual, max e min no webserver


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
//#include <ThreeWire.h>  
//#include <RtcDS1302.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

IPAddress ip(192, 168, 4, 1);
//IPAddress gateway(192, 168, 4, 1);
//IPAddress subnet(255, 255, 255, 0);


#define MODOWIFI WIFI_PHY_MODE_11B     // maior alcance, 11 Mbps  - celular encontra na pesquisa de redes wifi
//#define MODOWIFI WIFI_PHY_MODE_11G   // menor alcance, 54 Mbps - celular nao encontra na pesquisa de wifi, so o notebook
//#define MODOWIFI WIFI_PHY_MODE_11N   


//LISTA DE WIFI CONHECIDAS
char *wifi[]={"BUDA2",
              "PIRAFORD",
              "MotOgbuda",
              "DESKTOP_F8A92914",
              "Sscagnolato",
              "CLARO_2G57483F"};
              
char *wifipass[]={"saicapeta666",
                  "soeusei3-2",
                  "soeusei1",
                  "23960040",
                  "saicapeta666",
                  "3857483F"};
                  
byte TOTAL_WIFI_CONHECIDA=6;

String a_conectar="NENHUM WIFI";
String a_conectar_senha="NENHUMA SENHA DISPONIVEL";

// portas do controlador
const int RELE_IGNICAO      = D5;
const int RELE_PARTIDA      = D6;
//const int LED_VERDE       = D6;
//const int LED_VERMELHO    = D7;
const int PORTA1_FG         = D1;
const int PORTA2_FG         = D2; 
const int PORTA1_DSP        = 1;
const int PORTA2_DSP        = 3;
const int SINAL_RPM         = D7;
const int SINAL_REMOTO      = D0;
const int RELE_TRAVA_PORTA  = D3;
const int LED_STDBY         = 10;
//const int TX_ARDUINO        = 13;
//const int RX_ARDUINO        = 11;


const float MEDIA_ERRO_POR_MINUTO=0.00266;      // ERRO MEDIO POR MINUTO em ms

int TIPORELOGIO = 0;     // 0 = RELOGIO DECIMAL, 1 = RELOGIO BINARIO

int MODOFUNCIONAMENTO = 0;  // 0 = NORMAL, 1 = MANUTENCAO (IGNORA LEITOR BIOMETRICO)

bool flag_segundo=true;

int wifion=1;

int ESPACO_LINHAS=1;           // linha a pular na inicializacao;
int linha_init=0;
int xdelay;

ESP8266WebServer server(80); //Server on port 80

SoftwareSerial mySerial(PORTA1_FG, PORTA2_FG);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//SoftwareSerial arduino(RX_ARDUINO,TX_ARDUINO);   // RX / TX

//Pinos do NodeMCU
// SDA => RX (3)
// SCL => TX  (1)
// Inicializa o display Oled
//SSD1306Wire  display(0x3c, PORTA1_DSP, PORTA2_DSP);

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 20, 4); //FUNÇÃO DO TIPO "LiquidCrystal_I2C"


byte acesso,acessoweb=0;
byte estadoIgnicao=0;
byte leitorok=1;
byte porta_travada=0;
uint8_t p,p1;

int hora,minuto,status_stdy,rpm=0;
long umsegundo=1000;    
long intervalo_rpm=250;
unsigned long previousMillis,previousstd=0;
float segundo=0;
int dia,mes,ano=0;

float tempmax = 0;
float tempmin = 999;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

int DEBUG=0;         // para debug

// NTP
String formattedDate;
String timeStamp,dayStamp;
const long utcOffsetInSeconds = -10800;      // fuso horario gmt-3 (gmt-1 = -3600)


// Cliente NTP 
WiFiUDP ntpUDP;
// ****************** pool de ntp a conectar
//NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utcOffsetInSeconds);
NTPClient timeClient(ntpUDP, "200.160.7.186", utcOffsetInSeconds);         // a.st1.ntp.bt
//NTPClient timeClient(ntpUDP, "201.49.148.135", utcOffsetInSeconds);          // b.st1.ntp.br   --> trava ?
//NTPClient timeClient(ntpUDP, "200.186.125.195", utcOffsetInSeconds);       // c.st1.ntp.br
//NTPClient timeClient(ntpUDP, "200.20.186.76", utcOffsetInSeconds);         // d.st1.ntp.br
//NTPClient timeClient(ntpUDP, "200.160.0.8", utcOffsetInSeconds);           // a.ntp.br
//NTPClient timeClient(ntpUDP, "200.189.40.8", utcOffsetInSeconds);          // b.ntp.br
//NTPClient timeClient(ntpUDP, "200.192.232.8", utcOffsetInSeconds);         // c.ntp.br


//INICIA MODULO RTC
//ThreeWire myWire(RTC_DAT,RTC_CLK,RTC_RST); // IO, SCLK, CE
//RtcDS1302<ThreeWire> Rtc(myWire)

unsigned long previous_vivo=millis();
unsigned long atual_vivo;


// TEMPERATURA
const float R2=2300;   // resistor no A0


byte barra[8]={ B01111110,
                B01111110,               
                B01111110,
                B11111110,
                B01111110,
                B01111110,
                B01111110,
                B01111110,};

byte grau[8] ={ B00001100,
                B00010010,
                B00010010,
                B00001100,
                B00000000,
                B00000000,
                B00000000,
                B00000000,};       

byte vazio[8] ={B00000000,
                B00000110,
                B00001001,
                B00001001,
                B00001001,
                B00001001,
                B00000110,
                B00000000,};

byte cheio[8] ={B00000000,
                B00000110,
                B00001111,
                B00001111,
                B00001111,
                B00001111,
                B00000110,
                B00000000, };
                

// converte de inteiro para str com base
char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}


// formata a string com zeros a esquerda
 char* formatar(char* txt,char *result) { 
  char* retorno=result;
  int indiceret=0;
  int tamasec=6-strlen(txt);
  while(tamasec>0) {
   *retorno++=(byte)2;
   tamasec--;
  }
  tamasec=strlen(txt);
  int indice=0;
  while(indice< tamasec) {
   if(txt[indice++] =='1') *retorno++=(byte)3;
   else *retorno++=(byte)2;
  }
  *retorno++='\0';
  return retorno;
 }




/*********************************************/
/***************** SETUP *********************/
void setup(void) {
 if(DEBUG==1) {
  Serial.begin(9600);
  Serial.println("");
  Serial.println("");
 }

 pinMode(RELE_IGNICAO, OUTPUT);
 pinMode(RELE_PARTIDA, OUTPUT);
 pinMode(RELE_TRAVA_PORTA, OUTPUT);
 pinMode(LED_STDBY, OUTPUT);

 pinMode(SINAL_REMOTO,INPUT);
 pinMode(SINAL_RPM,INPUT);
 
 digitalWrite(RELE_IGNICAO, HIGH);
 digitalWrite(RELE_PARTIDA, HIGH);
 digitalWrite(RELE_TRAVA_PORTA, HIGH);
 digitalWrite(LED_STDBY, HIGH);


 //digitalWrite(LED_VERDE, LOW);
 //digitalWrite(LED_VERMELHO, HIGH);

  Wire.begin(PORTA1_DSP,PORTA2_DSP);
  lcd.begin();   // INICIALIZA O DISPLAY LCD

 //Apaga o display
  lcd.clear();
 lcd.setCursor(7,0);
 lcd.print("Blazer");
 lcd.setCursor(6,1);
 lcd.print("CCY 5030");
 lcd.setCursor(4,2);
 lcd.print("Painel V1.01");
 lcd.setCursor(1,3);        // <-- AQUI POSICIONA O NOME PARA SAIR CENTRALIZADO (COLUNA, LINHA)
 lcd.print("Sergio Scagnolato"); 
 delay(3000);

  lcd.clear();
 //atualizador OTA
 ArduinoOTA.setPort(8266);     // Porta
 ArduinoOTA.setHostname(host_ota);  // hostname
 ArduinoOTA.setPassword("teresa3-2");
 ArduinoOTA.begin(); 
 lcd.setCursor(0,linha_init);
 lcd.print("OTA.....");
 linha_init+=ESPACO_LINHAS;
 
 finger.begin(57600);
 delay(10);

// verifica se o leitor bio esta conectado
 if (finger.verifyPassword()) {
  leitorok=1;
  if(DEBUG==1) Serial.println("Leitor conectado");
  lcd.setCursor(0,linha_init);
  lcd.print("Leitor conectado");
  
 } else {
  leitorok=0;
  if(DEBUG==1) Serial.println("Leitor nao encontrado");
  lcd.print("** Leitor nao encontrado **");
 }

 linha_init+=ESPACO_LINHAS;
 finger.getTemplateCount();
 int cadastros=finger.templateCount;
 if(DEBUG==1) {
  Serial.println(" ");
  Serial.print(cadastros);
  Serial.print(" Biometrias");
  Serial.println(" ");
 }
 lcd.setCursor(0,linha_init);
 lcd.print(String(cadastros));
 lcd.setCursor(4,linha_init);
 lcd.print("Cadastros");
 linha_init+=ESPACO_LINHAS;
 lcd.setCursor(0,linha_init);
 
 
 finger.LEDcontrol(false);
 delay(500);

 lcd.clear();
 linha_init=0;
 
 acerta_relogio(1);

  WiFi.mode(WIFI_AP);           // Apenas AP
//  WiFi.mode(WIFI_AP_STA);         // AP + ST
// WiFi.setPhyMode(MODOWIFI);
 WiFi.config(ip, gateway, subnet);
 boolean result=WiFi.softAP(ssid, password,canal_wifi,false,max_connection);  // Inicia HOTspot

 if(result==true) {
  if(DEBUG==1) Serial.println("SoftAP OK");
 } else {
  if(DEBUG==1) Serial.println("Erro no SoftAP");
 } 
// display.display(); 
 
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
 server.on("/travaportas", travaporta); 
 server.on("/destravaportas", destravaporta); 
 server.on("/tiporelogiodec", tiporelogiodec); 
 server.on("/tiporelogiobin", tiporelogiobin); 
 server.on("/modonormal", modonormal); 
 server.on("/modomanutencao", modomanutencao); 

 server.onNotFound(handle_NotFound);

 server.begin();  
 lcd.print("WebServer iniciado");
 //linha_init+=ESPACO_LINHAS;
 //lcd.setCursor(0,linha_init);

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
 lcd.clear();

 lcd.createChar(0, grau);
 lcd.createChar(1, barra);
 lcd.createChar(2, vazio);
 lcd.createChar(3, cheio);
}


/********************* LOOP ********************/
void loop() {
 server.handleClient();  //Handle client requests
 MDNS.update();

 int remoto=digitalRead(SINAL_REMOTO);
 
 // relogio 

 atualiza_relogio();
 
 if(remoto==1 or acessoweb==1) { 
//  if (wifion==0) {
//    wifi_on();   
//    wifion=1;
//  }
  status_stdy=0;
  lcd.backlight();
  digitalWrite(LED_STDBY,LOW);
  
  if(remoto==1) acessoweb=0;
  
  mostra_hora();

  if(acesso==1 or MODOFUNCIONAMENTO==1) {
  
   le_rpm();
   mostra_temperatura();
   mostra_barra_rpm();
  }

 //  server.handleClient();  //Handle client requests
 
 if(leitorok==1 or MODOFUNCIONAMENTO==1) { 
  if(acesso==0 and acessoweb==0) {
   digitalWrite(LED_STDBY,LOW);
   lcd.setCursor(4,2); 
   lcd.print("Aguardando");
   lcd.setCursor(5,3); 
   lcd.print("biometria");

   if(MODOFUNCIONAMENTO==0)
    acesso=le_biometria();
   else
    acesso=1;

  } else {    // cadastro reconhecido
   if(estadoIgnicao==0){
    digitalWrite(RELE_IGNICAO,LOW);
    estadoIgnicao=1;
   }
 
  }
 }
} else {    //  entra no stdby
  estadoIgnicao=0;
  acesso=0;
  if(status_stdy==0) {
//   wifi_off();
//   wifion=0;
   finger.LEDcontrol(false);
   digitalWrite(RELE_IGNICAO,HIGH);
   lcd.clear();
   lcd.setCursor(5,1);
   lcd.print("Stand By");
   unsigned long stdby = millis();
   while( millis() - stdby < 3200){ // 10 ms
    yield();
   }
   
//   delay(3500);
   lcd.noBacklight();
   lcd.clear();
   status_stdy=1;
  }
//  char horario[5];
//  sprintf(horario,"%02d:%02d",hora,minuto);
//  display.drawString(65, 43, horario);  
//  display.display();

 int delaypisca=700;
 if(status_stdy==1) {

  float temperatura=le_temperatura();
  if(temperatura>30) delaypisca = 600;
  if(temperatura>40) delaypisca = 500;
  if(temperatura>50) delaypisca = 300;
  if(temperatura>60) delaypisca = 150;
  if(temperatura>70) delaypisca = 40;
 
  digitalWrite(LED_STDBY,LOW);
  unsigned long startled = millis();
  while( millis() - startled < delaypisca){ // 10 ms
   yield();
  }
  digitalWrite(LED_STDBY,HIGH);
  startled = millis();
  while( millis() - startled < 90){ // 10 ms
   yield();
  }
 } 

}
 
ArduinoOTA.handle(); 
 delay(10);
// unsigned long start = millis();
// while( millis() - start < 10){ // 10 ms
//  yield();
// }
}



//==============================================================
//     PAGINA PRINCIPAL
//==============================================================
void handleRoot() {
   String PAGINA_PRINCIPAL=
          "\n"
          "html {"
          "font-family: Sans-serif;"
          "display: inline-block;"
          "margin: 0px auto;"
          "text-align: center;}\n"
          ".switch {\n"
          "position: relative;\n"
          "display: inline-block;\n"
          "width: 60px;\n"
          "height: 34px;\n"
          "}\n"
          ".switch input {\n"
          "opacity: 0;\n"
          "width: 0;\n"
          "height: 0;\n"
          "}\n"
          ".slider {\n"
          "position: absolute;\n"
          "cursor: pointer;\n"
          "top: 0;\n"
          "left: 0;\n"
          "right: 0;\n"
          "bottom: 0;\n"
          "background-color: #ccc;\n"
          "-webkit-transition: .4s;\n"
          "transition: .4s;\n"
          "}\n"
          ".slider:before {\n"
          "position: absolute;\n"
          "content: \"\";\n"
          "height: 26px;\n"
          "width: 26px;\n"
          "left: 4px;\n"
          "bottom: 4px;\n"
          "background-color: white;\n"
          "-webkit-transition: .4s;\n"
          "transition: .4s;\n"
          "}\n"
          "input:checked + .slider {\n"
          "background-color: #2196F3;\n"
          "}\n"

          "input:focus + .slider {\n"
          "box-shadow: 0 0 1px #2196F3;\n"
          "}\n"

          "input:checked + .slider:before {\n"
          "-webkit-transform: translateX(26px);\n"
          "-ms-transform: translateX(26px);\n"
          "transform: translateX(26px);\n"
          "}\n"
          ".slider.round {\n"
          "border-radius: 34px;\n"
          "}\n"
          ".slider.round:before {\n"
          "border-radius: 50%;\n"
          "}\n";
          

  
 String MAIN_page="<!DOCTYPE html><html>";
 MAIN_page+="<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
 //MAIN_page+="<link rel=icon href=data:,>";
 MAIN_page+="<style>";
 MAIN_page+=PAGINA_PRINCIPAL;
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
 MAIN_page+="    font-size: 16px; ";
 MAIN_page+="    margin: 1px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".mais:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+="table{font-size:10;font-family:sans-serif;}";
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

 MAIN_page+="\n<script>\n";
 MAIN_page+="function butaotrava() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/travaportas';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";

 MAIN_page+="\n<script>\n";
 MAIN_page+="function butaodestrava() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/destravaportas';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";

 MAIN_page+="\n<script>\n";
 MAIN_page+="function tiporelogiodec() {\n";
// MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/tiporelogiodec';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";

 MAIN_page+="\n<script>\n";
 MAIN_page+="function tiporelogiobin() {\n";
// MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/tiporelogiobin';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";

 MAIN_page+="\n<script>\n";
 MAIN_page+="function modonormal() {\n";
// MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/modonormal';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";

 MAIN_page+="\n<script>\n";
 MAIN_page+="function modomanutencao() {\n";
// MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/modomanutencao';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";


 
 MAIN_page+="<h3>Blazer - Versao - 2.11</h3>";


 if(porta_travada==0)
  MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaotrava()'>TRAVAR PORTAS</button></p>";
 else
  MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaodestrava()'>DESTRAVAR PORTAS</button></p>";

 if(estadoIgnicao==0)
  MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaoon()'>LIGAR IGNICAO</button></p>";
 else
  MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaooff()'>DESLIGAR IGNICAO</button></p>";

  MAIN_page+="<table border=0 width=70% align=center>";
  MAIN_page+="\n<tr><td>RELOGIO DECIMAL</td><td>";
 if(TIPORELOGIO==1) {  // 0 = DECIMAL, 1= BINARIO
       MAIN_page+="\n<label class=switch>";
       MAIN_page+="\n<input type=checkbox onclick='window.location.href=\"/tiporelogiodec\"' checked>";
       MAIN_page+="\n<span class='slider round'></span>";
       MAIN_page+="\n</label>\n";
 } else {
      MAIN_page+="\n<label class=switch>";
      MAIN_page+="\n<input type=checkbox onclick='window.location.href=\"/tiporelogiobin\"'>";
      MAIN_page+="\n<span class='slider round'></span>";
      MAIN_page+="\n</label>\n";
 }
 MAIN_page+= "</td><td>RELOGIO BINARIO</td></tr></table>\n";
 
 /* 
  MAIN_page+="<input type=radio name=xrelogio value=0 id=tiporelogiodec onclick='tiporelogiodec()'>DECIMAL";
  MAIN_page+="<input type=radio name=xrelogio value=1 id=tiporelogiobin checked> BINARIO</p>";
 } else { 
  MAIN_page+="<input type=radio name=xrelogio value=0 id=tiporelogiodec checked>DECIMAL";
  MAIN_page+="<input type=radio name=xrelogio value=1 id=tiporelogiobin onclick='tiporelogiobin()'> BINARIO</p>";
 }
*/

 MAIN_page+="\n<p>MODO ";
 if(MODOFUNCIONAMENTO==1) {  // 0 = NORMAL, 1= MANUTENCAO
  MAIN_page+="<input type=radio name=xmodo value=0 id=tipomodo onclick='modonormal()'>NORMAL";
  MAIN_page+="<input type=radio name=xmodo value=1 id=tipomodo checked> MANUTENCAO</p>";
 } else { 
  MAIN_page+="<input type=radio name=xmodo value=0 id=tipomodo checked>NORMAL";
  MAIN_page+="<input type=radio name=xmodo value=1 id=tipomodo onclick='modomanutencao()'> MANUTENCAO</p>";
 }

 MAIN_page+="\n<br><br><br>\n<p><button class=button id=botaoabrir onclick='butaoreset()'>Reset</button></p>";
 
 
 unsigned long current_vivo=millis();
 char horario[20];
 sprintf(horario,"%02d:%02d:%02.4f",hora,minuto,segundo);
 MAIN_page+="<br><br>";
 MAIN_page+=horario;

 int cadastros=finger.templateCount;
 MAIN_page+="<br>";
 MAIN_page+=String(cadastros);
 MAIN_page+=" Biometrias ativas<br>";
 
 MAIN_page+="<br>Vivo a ";
 atual_vivo = current_vivo - previous_vivo;
 int vivo=(atual_vivo/1000/60);
 String msg=" minutos";
 if(vivo>=60){
  vivo=vivo/60;
  msg=" hora";
  if(vivo>1) msg+="s";
  if(vivo>=24) {
   vivo=vivo/24;
    msg=" dia";
    if(vivo>1) msg+="s";
    if(vivo>=30) {
     vivo=vivo/30;
     msg=" mes";
     if(vivo>1) msg+="s";
    }
  }
 }
 MAIN_page+=String(vivo);
 MAIN_page+=msg;

 // le sensor de temperatura
  float temperatura=le_temperatura();
 if(temperatura>tempmax) tempmax=temperatura;
 if(temperatura<tempmin) tempmin=temperatura;
 char temp[30];

 sprintf(temp,"%3.1f / %3.1f / %3.1f",temperatura,tempmax,tempmin);
  
 MAIN_page+="<br><p>Temperatura ";
 MAIN_page+=temp;
 MAIN_page+="</p>";
 
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
// if(leitorok==1) {
//  leitorok=0;
  acesso=0;
  finger.LEDcontrol(true);
// }
 handleRoot();
}

void reset_esp() {
 ESP.restart(); 
 handleRoot();
}

void travaporta() {
 porta_travada=1;
 digitalWrite(RELE_TRAVA_PORTA,LOW);
 digitalWrite(RELE_PARTIDA,LOW);
 delay(250);
 digitalWrite(RELE_TRAVA_PORTA,HIGH);
 digitalWrite(RELE_PARTIDA,HIGH);
 handleRoot();
}

void destravaporta() {
 porta_travada=0;
 digitalWrite(RELE_TRAVA_PORTA,LOW);
 delay(250);
 digitalWrite(RELE_TRAVA_PORTA,HIGH);
 handleRoot();
}

void partida() {
 digitalWrite(RELE_PARTIDA,LOW);
 delay(250);
 digitalWrite(RELE_PARTIDA,HIGH);
 handleRoot();
}

void tiporelogiodec() {
 TIPORELOGIO=0;
 lcd.setCursor(0,0);
 lcd.print("                    ");
 handleRoot();
}

void tiporelogiobin() {
 TIPORELOGIO=1;
 lcd.setCursor(0,0);
 lcd.print("                    ");
 handleRoot();
}

void modonormal() {
 MODOFUNCIONAMENTO=0;
 handleRoot();
}

void modomanutencao() {
 MODOFUNCIONAMENTO=1;
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
  lcd.print("Buscando WIFI");
  linha_init+=ESPACO_LINHAS;
  lcd.setCursor(0,linha_init);
 }
 
 // scan nos wifi proximos
 int nro_wifi = WiFi.scanNetworks();
 if (nro_wifi == 0) {
  if(status==1) {
   lcd.print("** Nenhum WIFI **");
   linha_init+=ESPACO_LINHAS;
   lcd.setCursor(0,linha_init);
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
   lcd.print(a_conectar);
   //linha_init+=ESPACO_LINHAS;
  }
  int vezes=0;
  int coluna=11;
  lcd.setCursor(coluna,linha_init);

  while ( WiFi.status() != WL_CONNECTED and vezes<=10) {
   if(DEBUG==1) Serial.print(".");
   digitalWrite(LED_STDBY,LOW);
   delay(120);
   digitalWrite(LED_STDBY,HIGH);
   if(status==1) {
    lcd.print(".");
    if(coluna<9) coluna++;
    lcd.setCursor(coluna,linha_init);
   }
   
   vezes++;
  }
  if(status==1)  linha_init+=ESPACO_LINHAS;
  vezes=10;
  while(!timeClient.update() and vezes>0) {
    timeClient.forceUpdate();
    vezes--;
   digitalWrite(LED_STDBY,LOW);
   delay(200);
   digitalWrite(LED_STDBY,HIGH);
  }
  if(vezes>0) {   
   formattedDate = timeClient.getFormattedDate();
  // O NTP RETONA A DATA NO FORMATO :
  // 2018-05-28T16:00:13Z
   int splitT = formattedDate.indexOf("T");
   dayStamp = formattedDate.substring(0, splitT);
   timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
   hora    = timeStamp.substring(0,2).toInt();
   minuto  = timeStamp.substring(3,5).toInt();
   segundo = timeStamp.substring(6,8).toFloat();
   ano=dayStamp.substring(0,4).toInt();
   mes=dayStamp.substring(5,2).toInt();
   dia=dayStamp.substring(8,2).toInt();
      
  } else {
    hora=minuto=segundo=0;
    dia=mes=ano=1;
  }
  WiFi.disconnect();

  if(vezes>0) {
   lcd.setCursor(0,linha_init);
   lcd.print("NTP OK");
   xdelay=200;
  } else {
   lcd.setCursor(0,linha_init);
   lcd.print("** ERRO NTP **");
   xdelay=200;
  }
 } else {
  hora=minuto=segundo=0;
  dia=mes=ano=1;
 }
 delay(xdelay);
 if(status==1) {
   linha_init += ESPACO_LINHAS;
   if(linha_init>3) {linha_init=0;lcd.clear();}
 }
 lcd.setCursor(0,linha_init);

/*
 Rtc.SetIsRunning(true); //INICIALIZA O RTC
 Rtc.SetIsWriteProtected(false); //HABILITA GRAVAÇÃO NO RTC

 //monta as variaveis no formato
 //data=MON dd yyy e TIME=hh:mm:ss
 char datahoje[12];
 char horahoje[8];
 char mese[4];
 mesext(mes).toCharArray(mese,4);
 sprintf(datahoje, "%s %02d %04d" , mese,dia,ano);
 sprintf(horahoje,"%02d:%02d:%02d",hora,minuto,segundo);
 //atualiza o RTC
 RtcDateTime compiled = RtcDateTime(datahoje, horahoje);
*/ 
} 


void atualiza_relogio() {
 // relogio 
 timeNow = millis()/1000; 
 if(flag_segundo) {
  segundo = timeNow - timeLast;
 } else {
  segundo = timeNow - timeLast;
  segundo=segundo+MEDIA_ERRO_POR_MINUTO;
  flag_segundo=true;
 }
 if (segundo > 59) {
  timeLast = timeNow;
  minuto = minuto + 1;
  flag_segundo=false;
 }
 if (minuto == 60){ 
  minuto = 0;
  hora = hora + 1;
 }
 if (hora == 24){
  hora = 0;
 }
}



void mostra_hora() {
//  lcd.clear();
  if(TIPORELOGIO==0) {
   lcd.setCursor(6,0);
   char horario[12];
   sprintf(horario,"%02d:%02d",hora,minuto);
   lcd.print(horario);
  } else {
   char binasec[7],binamin[7],binahor[7];
//   itoa(segundo,binasec,2);
   itoa(minuto,binamin,2);
   itoa(hora,binahor,2);
   char horafinal[7],minutofinal[7],segundofinal[7];
   formatar(binahor,horafinal);
   formatar(binamin,minutofinal);
// formatar(binasec,segundofinal);
   lcd.setCursor(3,0);
   lcd.print(horafinal);
   lcd.print(":");
   lcd.print(minutofinal);
//   lcd.print(":");
//   lcd.print(segundofinal);
  }
 
 // display.display();
}


//void mostra_hora() {
//  myRTC.updateTime();
//  char horario[5];
//  sprintf(horario,"%02d:%02d:%02d",myRTC.hours,myRTC.minutes,myRTC.seconds);
//  display.setFont(ArialMT_Plain_24);
//  display.drawString(65, 11, horario);
//}
  




void le_rpm(void) {
  int tick=0;
  int x=0;
  unsigned long currtime_rpm=millis();
   while(millis() -currtime_rpm <= intervalo_rpm) {
    if(digitalRead(SINAL_RPM)==LOW and x==0) {tick++; x=1;}
    if(digitalRead(SINAL_RPM)==HIGH) x=0;
   }
   rpm=(tick/2)*15;    // 4 pulsos no neg da bobina a cada volta
}

void mostra_barra_rpm(void) {
  int sinal,x;
  char buff[6];
  lcd.setCursor(0,1);
  sprintf(buff,"%04d",(int)rpm);
  lcd.print(buff);
  lcd.print(" RPM");
  lcd.print("            ");
  lcd.setCursor(9,1);
  x=(int)rpm/500;
  for(sinal=0;sinal<x;sinal++) lcd.write((byte)1);

/*
   int porc=100*rpm/6500;   // max rpm=6500;
   display.drawProgressBar(0, 45, 126, 10, porc);
   //Atualiza a porcentagem completa
   display.setTextAlignment(TEXT_ALIGN_CENTER);
   display.setFont(ArialMT_Plain_10);
   display.drawString(64, 54, String(rpm));
   display.setFont(ArialMT_Plain_24);
   display.display();
*/   
}


void mostra_temperatura() {
  float temperatura = le_temperatura();
  char temp[10];
  sprintf(temp,"%3.1f",temperatura);
  lcd.setCursor(0,3);
  lcd.print("Temp. externa ");
  lcd.print(temp);
  lcd.write((byte)0);
}

float le_temperatura() {
  int xxx;
  float leitura=0;
  for(xxx=0;xxx<100;xxx++) {
    leitura+=analogRead(A0);
   }
   leitura=leitura/100.0;
  
   float Vo=(leitura * 3.3) / 1024;
   float Rx = ((3.3 * R2) / Vo) - R2;     //  lei de Kirchoff

  // ler a temperatura com ponta NTC
  // constantes do NTC
   float A=0.0014056;
   float B=0.000240762;
   float C=7.48e-07;
   Rx=Rx/10;

   float x0=log(Rx);
   float x1=B * x0;
   float x2=C * pow(x0,3);
   float temperatur = ( 1 / ( A + x1 + x2) );     
   temperatur=temperatur - 273.15;               // converte Kelvin para Celius
   return(temperatur);
}



int le_biometria(void) {
   int permite=0;
   finger.LEDcontrol(true);
/* 
  uint8_t p2 = finger.getImage();
  while(p2 != FINGERPRINT_OK) {
    if(p2==FINGERPRINT_IMAGEFAIL) {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("DEDO NAO");
      lcd.setCursor(4,3);
      lcd.print("POSICIONADO");
      unsigned long start = millis();
      while( millis() - start < 2*1000){ // 2 segundos
       yield();
      }
      lcd.clear();
    }
  }
*/
   p = finger.getImage();
   if(p==FINGERPRINT_OK) {
    p1 = finger.image2Tz();
    while(p1!=FINGERPRINT_OK) {
     p1 = finger.image2Tz();
     unsigned long start = millis();
     while( millis() - start < 25){ // 25 ms
      yield();
     }
    }
    p1 = finger.fingerSearch();
    if(p1==FINGERPRINT_NOTFOUND) {    // nao encontrou digital
     if(DEBUG==1) Serial.println("NAO AUTORIZADO");
      nao_autorizado();
      permite=0;
    }
    if(p1==FINGERPRINT_OK) { 
      if(DEBUG==1) Serial.println("ACESSO AUTORIZADO");
      finger.LEDcontrol(false);
      lcd.clear();
      permite=1;
    }
   }
 return(permite);
}

void nao_autorizado() {
      lcd.clear();
      lcd.setCursor(7,1);
      lcd.print("NAO");
      lcd.setCursor(4,3);
      lcd.print("AUTORIZADO");
      unsigned long start = millis();
      while( millis() - start < 2*1000){ // 2 segundos
       yield();
      }
      lcd.clear();
}

/*
void erro_de_leitura() {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("DEDO NAO");
      lcd.setCursor(4,3);
      lcd.print("POSICIONADO");
      unsigned long start = millis();
      while( millis() - start < 2*1000){ // 2 segundos
       yield();
      }
      lcd.clear();
}
*/

/*
String mesext(int mes) {
  String retorno;
  if(mes==1) retono="Jan";
  if(mes==2) retono="Feb";
  if(mes==3) retono="Mar";
  if(mes==4) retono="Apr";
  if(mes==5) retono="May";
  if(mes==6) retono="Jun";
  if(mes==7) retono="Jul";
  if(mes==8) retono="Aug";
  if(mes==9) retono="Sep";
  if(mes==10) retono="Oct";
  if(mes==11) retono="Nov";
  if(mes==12) retono="Dec";
  return(retorno);
}
*/   



void wifi_on() {
  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode(WIFI_AP);
  WiFi.config(ip, gateway, subnet);
  WiFi.softAP(ssid, password,canal_wifi,false,max_connection);  // Inicia HOTspot
  server.begin();
}

void wifi_off() {
  WiFi.mode(WIFI_OFF);
}
