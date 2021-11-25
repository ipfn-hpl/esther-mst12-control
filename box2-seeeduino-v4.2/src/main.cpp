/**
 * @file main.c
 * @brief Arduino software to control MST12 Arm 
 * @author Bernardo Carvalho / IPFN
 * @date 30/09/2021
 *
 * @copyright Copyright 2016 - 2021 IPFN-Instituto Superior Tecnico, Portugal
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence, available in 23 official languages of
 * the European Union, at:
 * https://joinup.ec.europa.eu/community/eupl/og_page/eupl-text-11-12
 *
 * @warning Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing permissions and
 * limitations under the Licence.
 *
 * @details version Seeduino v4.2
 */


#include <Arduino.h>

#define LED_OUT 2 // PD2  // e.g. blue LED
//#define BLUE_SWITCH PIN_WIRE_SDA
#define BLUE_SWITCH 3 // PD3

#define LED_IN 4   // e.g. Red LED
#define RED_SWITCH 5 // PD5

#define LIMIT_IN 9 // PD6
#define LIMIT_OUT  8

#define RELAY_IN SDA
#define RELAY_OUT SCL // PB0

bool sensorLimIn, sensorLimOut;
bool switchIn, switchOut;
unsigned long holding ;

void loop2();
void loop3();

enum arm_state
{
  stopped,
  moving_in,
  moving_out,
  fully_closed,
  fully_open
} state;

//#define LED_PIN 13

const char* getStateName(enum arm_state state) 
{                                                                                                                
   switch (state) 
   {
      case stopped: return    "Stopped";
      case moving_in: return  "Moving In";
      case moving_out: return "Moving Out";
      case fully_closed: return "Closed";
      case fully_open: return  "Open";
      default: return "";
   }
}


void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_OUT, OUTPUT);
  pinMode(LED_IN, OUTPUT);
  pinMode(RELAY_IN, OUTPUT);
  pinMode(RELAY_OUT, OUTPUT);

  pinMode(BLUE_SWITCH, INPUT); // INPUT_PULLUP  /Grove switch has pull up
  pinMode(RED_SWITCH, INPUT);

  pinMode(LIMIT_OUT, INPUT_PULLUP);
  pinMode(LIMIT_IN,  INPUT_PULLUP);

  state = stopped;
  // Start serial port
  Serial.begin(115200);
}

void loop()     //fast Loop
{


  const long debounce = 1000; // 0.2 s
  const long debounce2 = 500; // 0.2 s


  sensorLimIn = digitalRead(LIMIT_IN);
  sensorLimOut = digitalRead(LIMIT_OUT);
  switchIn = !digitalRead(RED_SWITCH); // red
  switchOut = !digitalRead(BLUE_SWITCH);  // blue

  unsigned long now = millis();

  switch (state)
  {
  case stopped:
    digitalWrite(RELAY_IN, LOW);
    digitalWrite(RELAY_OUT, LOW);
    digitalWrite(LED_OUT, LOW);
    digitalWrite(LED_IN, LOW);
    if (now > holding) {
      if (switchIn && !sensorLimIn ){
          holding = now + debounce;
          state = moving_in;
          Serial.println(F("STOP->MOV_IN"));

      }
      else if (switchOut && !sensorLimOut ){
        holding = now + debounce;
        state = moving_out;
      }
    }
  //digitalWrite(RELAY_IN, HIGH);
    break;
  case moving_in:
    digitalWrite(RELAY_IN, HIGH);
    digitalWrite(RELAY_OUT, LOW);
    digitalWrite(LED_OUT, LOW); //
    digitalWrite(LED_IN, HIGH);
    if (sensorLimIn){
      state = fully_closed;
    }
    else if (( switchIn && (now > holding)) || switchOut){
      holding = now + debounce2;
      state = stopped;
      Serial.println(F("MOV_IN->STOP")) ;
    }
    break;
  case moving_out:
      digitalWrite(RELAY_OUT, HIGH);
      digitalWrite(RELAY_IN, LOW);
      digitalWrite(LED_OUT, HIGH);
      digitalWrite(LED_IN, LOW);
      if (sensorLimOut){
      //Serial.println(F("Moving->fully_open"));
        state = fully_open;
      }
    
      if ((switchOut && (now > holding)) || switchIn){
        holding = now + debounce2;
        state = stopped;
        //Serial.println(F("OUT->STOP"));
      }
    break;    
  case fully_closed:
      digitalWrite(RELAY_IN, LOW);
      digitalWrite(RELAY_OUT, LOW);
      if (switchOut) {
        //holding = now + debounce2;
        state = moving_out;
      }
    break;
  case fully_open:
    digitalWrite(RELAY_IN, LOW);
    digitalWrite(RELAY_OUT, LOW);
    // Blinking on loop2
    if (switchIn){
      //holding = now + debounce2;
      state = moving_in;
    }
    break;
  default:;
  }

  loop3();  // Slow print loop
  loop2();  // Slow blink loop
}

// Slow blink loop
void loop2() {

  static unsigned long nextTime = 0;
  const long interval = 200;
  static bool led_state = LOW;

  unsigned long now = millis();

  if ( now > nextTime ) {  
    nextTime = now + interval; 
    led_state = !led_state;   
    switch (state)
    {
      case fully_closed:  
          digitalWrite(LED_IN, led_state);
          digitalWrite(LED_OUT, LED_OFF);
        break;          
      case fully_open:  
          digitalWrite(LED_IN, LED_OFF);
          digitalWrite(LED_OUT, led_state); 
      default:;
    }
  }
}

void loop3() {

  static unsigned long lastTime = 0;
  const long interval = 2000;
  static bool led_state = 0;

  unsigned long now = millis();

  if ( now - lastTime > interval ) {
    //state = 1;
    lastTime = now;
     //Serial.print(state);
    Serial.print(getStateName(state));
    Serial.print(F(", SwIN: "));
    Serial.print(switchIn, DEC);
    Serial.print(F(", SwOUT: "));
    Serial.print(switchOut, DEC);
    Serial.print(F(", LimIN: "));
    Serial.print(sensorLimIn, DEC);
    Serial.print(F(", LimOUT: "));
    Serial.print(sensorLimOut, DEC);
    Serial.print(F(", Now "));
    Serial.print(now, DEC);
    Serial.print(F(", holding "));
    Serial.println(holding, DEC);
   /* Then, later in main: */
  //printf("%s", getDayName(TheDay));
   //Serial.println(F(" end"));
     if (led_state ) {
         digitalWrite(LED_BUILTIN, LOW);
         led_state = 0;
     }
    else {
         digitalWrite(LED_BUILTIN, HIGH);
         led_state = 1;
    }
  }
  // printf("%s", getDayName(TheDay));
}
