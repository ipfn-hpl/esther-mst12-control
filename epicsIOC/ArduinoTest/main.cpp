#include <Arduino.h>

bool ledon=false;
long randNumber;
char in_char='x';

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i=5; i-->0;){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  Serial.begin(115200);
  Serial.println();
}

void loop() {
  /*
    if (Serial.available()){
      in_char = Serial.read();
    } 
    */
    while (Serial.available() > 0) {
    // read incoming bytes, store last:
         in_char = Serial.read();
    }

    
//    Serial.print("1, O  pen, Char:x, SwIN:1, SwOUT:0, LimIN:0, LimOUT:0, Now:");
    if(in_char == 'x')
      Serial.print("1, Open, Char:x");
    else
        Serial.print("1, Open, Char:O");
    //Serial.print(in_char);
    Serial.print(", SwIN:1, SwOUT:0, LimIN:0, LimOUT:0, Now:");
    in_char = 'x';
    randNumber = random(100000);
    Serial.print(randNumber,DEC);
    Serial.println(", Holding:0");
    //Serial.println("A=6343");
    delay(1000);
    digitalWrite(LED_BUILTIN, ledon);
    ledon = !ledon;

  // put your main code here, to run repeatedly:
}