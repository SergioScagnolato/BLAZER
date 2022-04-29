void setup() {
 pinMode(A7,INPUT_PULLUP);   // recebe o sinal que ativa o rele da ignicao vindo no nodemcu
 Serial.begin(9600);

}

void loop() {
 int x=analogRead(A7);
 Serial.println(x);
 delay(700);

}
