#include <Arduino.h>

#define BLUE_LED 2 // PD2
//#define BLUE_SWITCH PIN_WIRE_SDA
#define BLUE_SWITCH 3 // PD3
#define RED_LED 4
#define RED_SWITCH 5 // PD5

//#define IN_RELAY 7  // PD5
//#define OUT_RELAY 8 // PB0

#define IN_RELAY SDA
#define OUT_RELAY SCL // PB0


void loop2();

enum arm_state
{
  stopped,
  moving_in,
  moving_out,
  limit_in,
  limit_out
} state;

//#define LED_PIN 13

const char* getStateName(enum arm_state state) 
{                                                                                                                
   switch (state) 
   {
      case stopped: return "Stopped";
      case moving_in: return "Moving In";
      case moving_out: return "Moving Out";
      case limit_out: return "Limit Out";
      default: return "";
   }
}


void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(IN_RELAY, OUTPUT);
  pinMode(OUT_RELAY, OUTPUT);
  pinMode(BLUE_SWITCH, INPUT);
  pinMode(RED_SWITCH, INPUT);

  // Start serial port
  Serial.begin(115200);
}

void loop()
{
  switch (state)
  {
  case stopped:
    digitalWrite(IN_RELAY, LOW);
    digitalWrite(OUT_RELAY, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, LOW);
    break;
  case moving_in:
    digitalWrite(IN_RELAY, HIGH);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    break;
  case moving_out:
    digitalWrite(OUT_RELAY, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    break;
  default:;
  }

  //Serial.print(state);
  //Serial.print(getStateName(state));

/* Then, later in main: */
//printf("%s", getDayName(TheDay));
  //Serial.println(" end");
  // turn the LED on (HIGH isthe voltage level)
  //digitalWrite(LED_BUILTIN, HIGH);

  if (!digitalRead(RED_SWITCH))
    state = moving_in;
  else if (!digitalRead(BLUE_SWITCH))
    state = moving_out;
  //digitalWrite(IN_RELAY, HIGH);
  else
    state = stopped;

  //  digitalWrite(OUT_RELAY, HIGH);

  //digitalWrite(OUT_RELAY, HIGH);
  //digitalWrite(IN_RELAY, HIGH);
  //  else
  //    digitalWrite(BLUE_LED, LOW);

  // wait for a second
  //delay(1000);
  // turn the LED off by making the voltage LOW
  //digitalWrite(BLUE_LED, LOW);
  //digitalWrite(RED_LED, LOW);

  //digitalWrite(IN_RELAY, LOW);
  //digitalWrite(OUT_RELAY, LOW);

  // wait for a second
  //delay(500);
  loop2();
}


void loop2() {

  static unsigned long lastTime = 0;
  const long interval = 500;
  static bool led_state = 0;

  unsigned long now = millis();

  if ( now - lastTime > interval ) {
    //state = 1;
    lastTime = now;
     //Serial.print(state);
    Serial.print(getStateName(state));

   /* Then, later in main: */
  //printf("%s", getDayName(TheDay));
   Serial.println(" end");
     if (led_state ) {
         digitalWrite(LED_BUILTIN, LOW);
         led_state = 0;
     }
    else {
         digitalWrite(LED_BUILTIN, HIGH);
         led_state = 1;
    }
  }

  }

