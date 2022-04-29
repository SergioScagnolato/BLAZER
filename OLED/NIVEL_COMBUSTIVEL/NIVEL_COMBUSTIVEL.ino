/* MARCADOR COMBUSTIVEL BLAZER
 *  COMPILAR COM ARDUINO NANO AT 328P
 *  
 */
 
// 04-03-22 - correcao da esquacao para motor ligado e desligado (diferenca de tensao no aterramento)


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)' 

// Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// mudar a conf na lib para display de 128 x 64 em home/sergio/Arduino/libraries/Adafruit_SSD1306/Adafruit_SSD1306.h (linha 28)
Adafruit_SSD1306 display1(OLED_RESET);
Adafruit_SSD1306 display2(OLED_RESET);

// A5 = SCL
// A4 = SCA


float Rc2=1006;   // combustivel
float Rc1=2150;

float R3=1480;   // temperatura

// voltimetro
float Vo = 0.0, Vi = 0.0;
float value_aux = 0, value = 0;
//Valores dos Resistores//
float Rv1 = 49000; //Resistência R1 (220) 
float Rv2 = 14000; //Resistência R2 (boia) 

int primeira_leitura=1;

int tempo = 2000;
int tempo_leituras=tempo;

float nivel_comb=0;

void setup() {
  pinMode(A0,INPUT);           // TEMPERATURA
  pinMode(A1,INPUT);           // SENSOR BOIA
  pinMode(A6,INPUT);           // TENSAO BATERIA
//  pinMode(A7,INPUT_PULLUP);    // SINAL REMOTO
  pinMode(6,OUTPUT);          // LED STBY
  
  display1.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // conf 0x78 no display 1
  display2.begin(SSD1306_SWITCHCAPVCC, 0x3D);   // conf 0x7A no display 2
  
  
  display1.setTextColor(WHITE); //DEFINE A COR DO TEXTO
  display1.invertDisplay(false);

  display2.setTextColor(WHITE); //DEFINE A COR DO TEXTO
  display2.invertDisplay(false);


  // teste do led
  for(int x=0;x<2;x++) {
   digitalWrite(6,HIGH);
   delay(100);
   digitalWrite(6,LOW);
   delay(100);
  } 
  digitalWrite(6,LOW);
}

void loop() {
   float boia,temp_motor;
   tempo_leituras=tempo;
   nivel_comb=nivel_combustivel(nivel_comb);
   
  // Serial.println(x);
   display1.clearDisplay(); //LIMPA AS INFORMAÇÕES DO DISPLAY
   display1.setTextSize(1); //DEFINE O TAMANHO DA FONTE DO TEXTO
   display2.clearDisplay(); //LIMPA AS INFORMAÇÕES DO DISPLAY
   display2.setTextSize(1); //DEFINE O TAMANHO DA FONTE DO TEXTO
   display1.setCursor(14,0); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
   display1.print("COMB (Lt)"); //ESCREVE O TEXTO NO DISPLAY
   display1.setCursor(86,0); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
   display1.print("BAT (V)"); //ESCREVE O TEXTO NO DISPLAY
   
   
   display2.setCursor(26,0); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
   display2.print("TEMPERATURA"); //ESCREVE O TEXTO NO DISPLAY
   
 //  display1.setCursor(104,2); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
 //  display1.print("BAT"); //ESCREVE O TEXTO NO DISPLAY
  
 
   // leitura tanque cheio = 88 a 93 ohms
   // leitura medio tanque = 41 a 44 ohms
   // leitura vazio        = 0 a 2 ohms


    
   boia=nivel_comb;
    if (boia>1.42) boia=1.42;
/*    
     int tamanhobarra=(boia*60)/1.42;   
     display1.setCursor(1,60); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
     display1.print("R");
     display1.setCursor(1,40); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
     display1.print("1/4");
     display1.setCursor(1,21); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
     display1.print("1/2");
     display1.setCursor(1,5); //POSIÇÃO EM QUE O CURSOR IRÁ FAZER A ESCRITA
     display1.print("1");
*/
  
     
     if(boia<0.023) {
      display1.invertDisplay(true);
      digitalWrite(6,HIGH);
     } else {
      float litros=boia;

/* IMPLEMENTACAO DA FUNCAO MAP ORIGINAL - TRABALHA COM LONG INT
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/

  //    litros= (litros - 0.005) * (70.0 - 1) / (0.4 - 0.005) + 1.0;       // funcao map para operar com float ao inves de long int 
      
      digitalWrite(6,LOW);
      display1.invertDisplay(false);
//      display1.fillRect(0, 30, tamanhobarra, 15, SSD1306_WHITE);

     float batt=bateria();
     display1.setTextSize(2);
     display1.setCursor(96,10);
     int bater=(int)batt;
     display1.print(bater); //ESCREVE O TEXTO NO DISPLAY


      if(bater<13) {
       litros=(49.30908826 * boia) + -2.17245852;    // equacao da reta para motor parado
      } else {
       litros=(51.2637526 * boia) + -10.91168599;    // equacao da reta para motor ligado
        
      }
      int litro=(int)litros;
//      display1.setTextSize(1);

      display1.setCursor(21,12);
      display1.setTextSize(3);
      display1.print(litro);
 //     display1.display();


// referencia em v do divisor de tensao da boia do tanque
//      display1.setTextSize(1);
//      display1.setCursor(100,26);
//      display1.print(boia);



/*****************************************************/

      float temp_motor=temperatura();

      // equacao da reta :
      // (b*x**1) + intercept
      // b = -32.21904264
      // intercept = 162.12568931

      float temp_graus=(-32.21904264 * temp_motor) + 162.12568931;
      int temp_grau=(int)temp_graus;

      display2.setTextSize(3);
      display2.setCursor(30,13);
      display2.print(temp_grau);
     
      if(temp_motor<1.7) {        // ventoinha liga a 2.06, se a leitura for menor motor quente demais
          digitalWrite(6,HIGH);
          delay(100);
          digitalWrite(6,LOW);
          delay(100);
      } else {
          digitalWrite(6,LOW);
      }

/*******************************************************/

     }

    display1.display();
    display2.display();
    
   delay(1000);
}




/**********************************************/



float nivel_combustivel(float leitura) {
  // retorna a leitura em ohms da boia de combustivel
  int xxx;
//  float leitura=0;
   float vez;
   if(leitura>0) vez=11.0; else vez=10.0;
   for(xxx=0;xxx<vez;xxx++) {
     leitura+=analogRead(A1);
     delay(2);
   }
   leitura=leitura/vez;

   Vo = (leitura * 5.0) / 1024.0;
 //  Vi = Vo / (Rc2/(Rc1+Rc2));
 //  if (Vi<0.09) {Vi=0.0;}
   return(Vo);

/*  
   float Vo=(leitura * 5.0) / 1024;
   float Rx = ((5.0 * Rc2) / Vo) - Rc2;     //  lei de Kirchoff
   Rx=Rx/10.0;
   return(Rx);
   */
}


float temperatura() {
  // retorna a leitura em ohms do sensor de temperatura
  int xxx;
  float leitura=0;
   for(xxx=0;xxx<30;xxx++) {
     leitura+=analogRead(A0);
   }
   leitura=leitura/30.0;
   leitura=(leitura*5.0)/1024;
   
 /* 
   float Vo=(leitura * 5.0) / 1024;
   float Rx = ((5.0 * R3) / Vo) - R3;     //  lei de Kirchoff
   Rx=Rx/10.0;
   char temp[10];
   return(Rx);
   */
   return(leitura);
}

float bateria() {
   // tensao bateria
   value=0;
   for(int x=0;x<10;x++){
    value_aux = analogRead(A6);
    value += value_aux;
    delay(2);
   }
   value = value/10.0;
   Vo = (value * 5.0) / 1023.0;
   Vi = Vo / (Rv2/(Rv2+Rv1));
   if (Vi<0.09) {Vi=0.0;}
   return(Vi);
}
