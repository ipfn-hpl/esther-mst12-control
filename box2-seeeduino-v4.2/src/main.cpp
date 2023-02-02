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
 * @details
 *  version Seeduino v4.2  + Baseshield
 * 
 *  Limit Sensor wires of the ARM: 
 * Blue:  VDD 5V
 * Brown: GND
 * Black: Sensor:
 * Sensor Normaly LED on
 *    On limit ->Led OFF, -> black Wire 5V
 * 
 *
 *  RS232 Connector (Outer plug, Box side) (soldering side)  
 *     
 *        M-            LI     S-
 *         1    2   3    4     5
 *     -----------------------------
 *      \  |   |    |    |    |   /
 *       \                       /
 *        \  |    |    |    |   /
 *          -------------------
 *           6    7    8    9 
 *          M+         LO     S+
 * 

 *
 * M-  Blue / motor connections
 * M+: Brown
 * S+: Sensor VDD 5v 
 * S-: Sensor GND 0v 
 * LI: Limit sensor IN
 * LO: Limit sensor OUT
 * 
 *********** Vacuum male-male connector is mirrored!!! ************
 *  RS232 Connector (Inner plug, motor side) (soldering side)  
 *     
 *        S-  LO              M- (1)
 *         1    2   3    4     5
 *      -----------------------------
 *       \  |   |    |    |     |  /
 *        \   |    |    |    |    / 
 *         \  6    7    8    9   /
 *          -------------------
 *          S+    LI       M+ (2)

 * M- wire "1";  wire "2" / motor connections
 * S+: Blue Wires Sensor VDD 5v 
 * S-: Brown wires Sensor GND 0v 
 * LI: Black wire Limit sensor IN
 * LO: Black wire Limit sensor OUT
 * 
 */


#include <Arduino.h>

/** Seeeduino v4.2 Specific **/
#define LED_OFF LOW
#define LED_ON HIGH

#define LED_OUT 2 // PD2  // e.g. blue LED
//#define BLUE_SWITCH PIN_WIRE_SDA
#define BLUE_SWITCH 3 // PD3

#define LED_IN 4   // e.g. Red LED
#define RED_SWITCH 5 // PD5

#define LIMIT_IN 7 // White Grove connector PD6
#define LIMIT_OUT  6  // Grove Yellow

#define RELAY_IN SDA
#define RELAY_OUT SCL // PB0

#define RELAY_OFF LOW
#define RELAY_ON HIGH
#define ADDRESS 1
/** end Seeeduino v4.2 Specific **/


bool sensorLimIn, sensorLimOut;
unsigned long holding, hold_stop;
// Commands
bool switchIn, switchOut, stop;
char in_char;

void led_loop();
void print_loop();


enum arm_state
{
    stopped,
    moving_in,
    moving_out,
    fully_closed,
    fully_open,
    error
} state;

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

    pinMode(BLUE_SWITCH, INPUT_PULLUP);
    pinMode(RED_SWITCH, INPUT_PULLUP);

    pinMode(LIMIT_OUT, INPUT_PULLUP);
    pinMode(LIMIT_IN,  INPUT_PULLUP);

    state = stopped;
    // Start serial port
    Serial.begin(115200);
    in_char = 'x';
}

void fast_loop()     //fast Loop
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

}

void loop() {

    fast_loop();
    print_loop();  // Slow print loop
    led_loop();  // Slow blink loop
}

// Slow blink loop
void led_loop() {

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

void print_loop() {

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
