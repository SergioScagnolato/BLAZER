
/********** VERSAO LCD ***************/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <Adafruit_Fingerprint.h>
//#include "SSD1306Wire.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// portas do controlador
const int RELE_IGNICAO      = D6;
const int RELE_PARTIDA      = D5;
//const int LED_VERDE       = D6;
//const int LED_VERMELHO    = D7;
const int PORTA1_FG         = D1;
const int PORTA2_FG         = D2; 
const int PORTA1_DSP        = 1;
const int PORTA2_DSP        = 3;
const int SINAL_RPM         = D7;
const int SINAL_REMOTO      = D0;
const int RELE_TRAVA_PORTA  = D3;


//SSID e Password do ESP
const char* ssid = "BLAZER";
const char* password = "soeusei1";
const byte canal_wifi = 2;
const byte max_connection=4;
const char* host_ota = "ESP_Blazer";
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80); //Server on port 80


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..

SoftwareSerial mySerial(PORTA1_FG,PORTA2_FG);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

//Pinos do NodeMCU
// SDA => RX (3)
// SCL => TX  (1)
// Inicializa o display Oled
//SSD1306Wire  display(0x3c, PORTA1_DSP, PORTA2_DSP);

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 20, 4); //FUNÇÃO DO TIPO "LiquidCrystal_I2C"


void setup()
{
//  Serial.begin(9600);
//  while (!Serial);  // For Yun/Leo/Micro/Zero/...
//  delay(100);

  lcd.begin();
  lcd.clear();
  lcd.setCursor(0,0);

 //Apaga o display
  
  // Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    // Serial.println("Found fingerprint sensor!");
    lcd.print("Sensor OK");
  } else {
    // Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
//  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
//  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
//  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  finger.getTemplateCount();
  uint8_t cadastros=finger.templateCount;
  lcd.setCursor(1,0);
  lcd.print(String(cadastros));  
  lcd.print("/");  
  lcd.print(String(finger.capacity));  
//  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
//  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
//  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
//  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  delay(2000);

 id=cadastros;

 WiFi.mode(WIFI_AP);           // Apenas AP
//  WiFi.mode(WIFI_AP_STA);         // AP + ST
// WiFi.setPhyMode(MODOWIFI);
 IPAddress ip(192, 168, 4, 1);
 IPAddress gateway(192, 168, 4, 1);
 IPAddress subnet(255, 255, 255, 0);
 WiFi.config(ip, gateway, subnet);
 boolean result=WiFi.softAP(ssid, password,canal_wifi,false,max_connection);  // Inicia HOTspot

  
 //atualizador OTA
 const char* host_ota = "ESP_Blazer";
 ArduinoOTA.setPort(8266);     // Porta
 ArduinoOTA.setHostname(host_ota);  // hostname
 ArduinoOTA.begin(); 
}

/*
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}
*/
void loop()                     // run over and over again
{
//  Serial.println("Ready to enroll a fingerprint!");
//  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
//  id = readnumber();
//  if (id == 0) {// ID #0 not allowed, try again!
//     return;
//  }
//  Serial.print("Enrolling ID #");
//  Serial.println(id);

//finger.getTemplateCount();
//uint8_t cadastros=finger.templateCount;
id=id+1;
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Cadastro ID ");  
lcd.print(String(id));  
lcd.setCursor(1,0);

  while (!  getFingerprintEnroll() );

  ArduinoOTA.handle(); 
  delay(15);
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  // Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    ArduinoOTA.handle(); 
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image taken");
      lcd.print("Imagem OK ");  
      delay(3000);
      break;
    case FINGERPRINT_NOFINGER:
      // Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      // Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      // Serial.println("Imaging error");
      break;
    default:
      // Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image converted");
      lcd.setCursor(2,0);
      lcd.print("Imagem convertida");  
      break;
    case FINGERPRINT_IMAGEMESS:
      // Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      // Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      // Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      // Serial.println("Could not find fingerprint features");
      return p;
    default:
      // Serial.println("Unknown error");
      return p;
  }

  // Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remova o dedo");  
  delay(3000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  // Serial.print("ID "); Serial.println(id);
  p = -1;
  // Serial.println("Place same finger again");
  lcd.setCursor(1,0);
  lcd.print("Coloque mesmo dedo");  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image taken");
      lcd.setCursor(2,0);
      lcd.print("Imagem ok");  
      delay(3000);
      break;
    case FINGERPRINT_NOFINGER:
      // Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      // Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      // Serial.println("Imaging error");
      break;
    default:
      // Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      // Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      // Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      // Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      // Serial.println("Could not find fingerprint features");
      return p;
    default:
      // Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  // Serial.print("Creating model for #");  Serial.println(id);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Modelo ");  
  lcd.print(String(id));  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    // Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    // Serial.println("Fingerprints did not match");
    return p;
  } else {
    // Serial.println("Unknown error");
    return p;
  }

  // Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
   // Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ARMAZENADO !");  
    delay(4000);
    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    // Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    // Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    // Serial.println("Error writing to flash");
    return p;
  } else {
    // Serial.println("Unknown error");
    return p;
  }

  return true;
}
