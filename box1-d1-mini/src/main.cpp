// Relay control using the ESP8266 WiFi chip


//uint8_t state = OFF_LIMITS;

// The port to listen for incoming TCP connections
//#define LISTEN_PORT 80

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
 * @details version ESP8266 WiFi d1_mini module
 * https://escapequotes.net/esp8266-wemos-d1-mini-pins-and-diagram/
 *
 * Limit Sensor wires
 * Blue: positive 5V
 * Brown: GND
 * Black: Sensor
 * Sensor Normally LED on
 *    On limit ->Led OFF, ->black 5V
 *
 *  RS232 Connector  (soldering side)
 *
 *         5    4   3    2     1
 *         |   |    |    |     |
 *
 *           |    |    |    |
 *           9    8    7    6
 *
 *
 */

#include <Arduino.h>
/*
#define LED_WEMOOS_D1_MINI 2  LED_BUILTIN // Pin D4 GPIO2
#define LIMIT_OUT 12         // D5
#define LIMIT_IN 14          // D6
#define SWITCH_3_IN 5        // D1
#define SWITCH_3_OUT 4       // D2
#define RELAY_IN 2           // D4  Relay Output HIGH = OFF (NC)
#define RELAY_OUT 0          // D3  Relay Output
#define LED_IN 16            // D0
#define LED_OUT 13           // D7
*/

/** esp8266 d1_specific **/
#define LED_OFF HIGH
#define LED_ON LOW

#define LED_OUT 13 //  // e.g. blue LED
#define BLUE_SWITCH 4 // PD4

#define LED_IN 16   // e.g. Red LED
#define RED_SWITCH 5 // PD5

#define LIMIT_OUT  12
#define LIMIT_IN   14 // PD9

//#define RELAY_IN 2
//#define RELAY_OUT 0 // PB0

#define RELAY_IN 0
#define RELAY_OUT 2 // PB0

#define RELAY_OFF HIGH
#define RELAY_ON LOW
#define ADDRESS 2
/** End esp8266 d1_specific **/

bool sensorLimIn, sensorLimOut;
unsigned long holding, hold_stop;
// Commands
bool switchIn, switchOut, stop;
char in_char;

void loop2();
void loop3();

enum arm_state
{
    stopped,
    moving_in,
    moving_out,
    fully_closed,
    fully_open,
    error
} state;

//#define LED_PIN 13

const char* getStateName(enum arm_state state)
{
    switch (state)
    {
        case stopped: return    "Stopped";
        case moving_in: return  "MovingIn";
        case moving_out: return "MovingOut";
        case fully_closed: return "Closed";
        case fully_open: return  "Open";
        case error: return  "Error";
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

    digitalWrite(RELAY_IN, RELAY_OFF);
    digitalWrite(RELAY_OUT, RELAY_OFF);

    pinMode(BLUE_SWITCH, INPUT_PULLUP); //
    pinMode(RED_SWITCH, INPUT_PULLUP);

    pinMode(LIMIT_OUT, INPUT_PULLUP);
    pinMode(LIMIT_IN,  INPUT_PULLUP);

    state = stopped;
    // Start serial port
    Serial.begin(115200);
    in_char = 'x';
}

void loop()     //fast Loop
{

    int iByte = 0; // for incoming serial data
    const long debounce = 1000;
    //const long debounce2 = 500; // 0.5 s
    const long in_extend = 1000;

    //in_char = 'x';
    //if ((in_char == 'x') ){
    while (Serial.available() > 0) {
        // read incoming bytes, store last:
        iByte = Serial.read();
        if (iByte == 'i' || iByte == 'I')
            in_char = 'i';
        else if (iByte == 'o' || iByte == 'O')
            in_char = 'o';
        else if (iByte == 's' || iByte == 'S')
            in_char = 's';
        //         else
        //             in_char = 'x';
    }

    unsigned long now = millis();
    sensorLimIn = digitalRead(LIMIT_IN);
    sensorLimOut = digitalRead(LIMIT_OUT);
    switchIn  = 0;
    switchOut = 0;
    if (in_char=='o'){
        in_char   = 'x';
        switchOut = 1;
    }
    else if ((now > holding) && !digitalRead(BLUE_SWITCH) ){
        holding = now + debounce;
        switchOut = 1;
    }
    else if (in_char=='i'){
        in_char  = 'x';
        switchIn = 1;
    }
    else if ((now > holding) && !digitalRead(RED_SWITCH) ){
        holding = now + debounce;
        switchIn = 1;
    }

    //!digitalRead(RED_SWITCH) || (in_char=='i'); // red
    //switchIn = !digitalRead(RED_SWITCH) || (in_char=='i'); // red
    //switchOut = !digitalRead(BLUE_SWITCH) || (in_char=='o');  // blue
    if (in_char=='s') {
        in_char = 'x';
        stop = 1;
    }
    //switchIn = !digitalRead(RED_SWITCH); // red
    //switchOut = !digitalRead(BLUE_SWITCH);  // blue


    if (sensorLimIn && sensorLimOut )
        state = error;

    switch (state) {
        case stopped:
            digitalWrite(RELAY_IN, RELAY_OFF);
            digitalWrite(RELAY_OUT, RELAY_OFF);
            digitalWrite(LED_OUT, LED_OFF);
            digitalWrite(LED_IN, LED_OFF);
            if (sensorLimIn && !sensorLimOut )
                state = fully_closed;
            else if (!sensorLimIn && sensorLimOut )
                state = fully_open;
            else if ( switchIn && !sensorLimIn ) {
                state = moving_in;
                Serial.println(F("STOP->MOV_IN"));
            }
            else if (switchOut && !sensorLimOut)
                state = moving_out;
            break;
        case moving_in:
            digitalWrite(RELAY_IN, RELAY_ON);
            digitalWrite(RELAY_OUT, RELAY_OFF);
            digitalWrite(LED_OUT, LED_OFF); //
            digitalWrite(LED_IN, LED_ON);
            if (sensorLimIn) {
                hold_stop = now + in_extend;
                state = fully_closed;
            }
            else if ( switchIn || switchOut) {
                state = stopped;
                Serial.println(F("MOV_IN->STOP")) ;
            }
            break;
        case moving_out:
            digitalWrite(RELAY_IN, RELAY_OFF);
            digitalWrite(RELAY_OUT, RELAY_ON);
            digitalWrite(LED_OUT, LED_ON);
            digitalWrite(LED_IN, LED_OFF);
            if (sensorLimOut)
                state = fully_open;
            //Serial.println(F("Moving->fully_open"));

            if (switchOut || switchIn)
                state = stopped;
            //Serial.println(F("OUT->STOP"));
            break;
        case fully_closed:
            if(now > hold_stop) { // Extended Inbound travel
                digitalWrite(RELAY_IN, RELAY_OFF);
                digitalWrite(RELAY_OUT, RELAY_OFF);
            }
            if (switchOut)
                state = moving_out;
            break;
        case fully_open:
            digitalWrite(RELAY_IN, RELAY_OFF);
            digitalWrite(RELAY_OUT, RELAY_OFF);
            if (switchIn)
                state = moving_in;
            break;
        case error:
            digitalWrite(RELAY_IN, RELAY_OFF);
            digitalWrite(RELAY_OUT, RELAY_OFF);
            if (sensorLimIn && !sensorLimOut )
                state = fully_closed;
            else if ( !sensorLimIn && sensorLimOut )
                state = fully_open;
            else if ( !sensorLimIn && !sensorLimOut )
                state = stopped;
            break;
        default:
            state = stopped;
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
                break;
            case error:
                digitalWrite(LED_IN, led_state);
                digitalWrite(LED_OUT, led_state);
            default:;
        }
    }
}

void loop3() {

    static unsigned long lastTime = 0;
    const long print_interval = 500;
    static bool led_state = 0;

    unsigned long now = millis();

    if ( now - lastTime > print_interval ) {
        //state = 1;
        lastTime = now;
        //Serial.print(state);
        Serial.print(ADDRESS);
        Serial.print(F(", "));
        Serial.print(getStateName(state));
        Serial.print(F(", Char:"));
        Serial.print(in_char);
        Serial.print(F(", SwIN:"));
        Serial.print(switchIn, DEC);
        Serial.print(F(", SwOUT:"));
        Serial.print(switchOut, DEC);
        Serial.print(F(", LimIN:"));
        Serial.print(sensorLimIn, DEC);
        Serial.print(F(", LimOUT:"));
        Serial.print(sensorLimOut, DEC);
        Serial.print(F(", Millis:"));
        Serial.print(now, DEC);
        Serial.print(F(", Holding:"));
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

// vim: syntax=cpp ts=4 sw=4 sts=4 sr et
