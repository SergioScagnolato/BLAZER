
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int PORTA1_DSP        = 1;
const int PORTA2_DSP        = 3;

//LiquidCrystal_I2C lcd(0x3F, 20, 4); //FUNÇÃO DO TIPO "LiquidCrystal_I2C"
LiquidCrystal_I2C lcd(0x27, 20, 4); //FUNÇÃO DO TIPO "LiquidCrystal_I2C"

void setup() {
  Wire.begin(PORTA1_DSP,PORTA2_DSP);

lcd.begin();   // INICIALIZA O DISPLAY LCD

  lcd.backlight();
  lcd.print("hello wotttttdd"); //ESCREVE O TEXTO NA PRIMEIRA LINHA DO DISPLAY LCD
  
  lcd.setCursor(0, 3);

}

void loop() {

  bool blinking = true;
  lcd.cursor();

  while (1) {
    if (blinking) {
      lcd.clear();
        lcd.setCursor(0, 3);

      lcd.print("No cursor blink");
      lcd.noBlink();
      blinking = false;
    } else {
      lcd.clear();
        lcd.setCursor(0, 3);

      lcd.print("Cursor blink");
      lcd.blink();
      blinking = true;
    }
    delay(4000);
  }
  // put your main code here, to run repeatedly:

}
