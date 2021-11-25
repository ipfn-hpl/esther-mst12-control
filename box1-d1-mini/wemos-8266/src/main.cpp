// Relay control using the ESP8266 WiFi chip
// Box number 2
// https://www.norwegiancreations.com/2017/03/state-machines-and-arduino-implementation
//
// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <aREST_UI.h>

// Create aREST instance
aREST_UI rest = aREST_UI();

// WiFi parameters
// const char *ssid = "Cabovisao-E30F";
// const char *password = "e0cec31ae30f";
// const char *ssid = "Vodafone-EDB45A";
// const char *password = "71F62F063E";
const char *ssid = "Lab. Plasmas Hipersonicos";
const char *password = "";

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80

// #define LED_WEMOOS_D1_MINI 2 // Pin D4 GPIO2 LED_BUILTIN
#define SEN_LIM_OUT 14         // D5
#define SEN_LIM_IN 12          // D6
#define SWITCH_IN 5        // D1  aka RED_SWITCH
#define SWITCH_OUT 4       // D2 aka BLUE_SWITCH
#define RELAY_IN 2           // D4  Relay Output HIGH = OFF (NC)
#define RELAY_OUT 0          // D3  Relay Output
#define LED_IN 16            // D0  Red LED?
#define LED_OUT 13           // D7



//#define BLUE_SWITCH PIN_WIRE_SDA
//#define BLUE_SWITCH 3 // PD3
//#define LED_IN 4   // e.g. Red LED
//#define RED_SWITCH 5 // PD5

//#define LIMIT_IN 6 // PD6
//#define SEN_LIM_OUT  7 // PD7

//#define RELAY_IN SDA
//#define RELAY_OUT SCL // PB0

#define RELAY_ON   LOW
#define RELAY_OFF HIGH

#define LED_ON   LOW
#define LED_OFF HIGH

bool sensorLimIn, sensorLimOut;
bool switchIn, switchOut;

unsigned long holding ;



void loop2();
void loop3();

enum arm_state
{
  STOPPED,
  MOVING_IN,
  MOVING_OUT,
  LIMIT_IN,
  LIMIT_OUT,
  ERROR  
} state;

//#define LED_PIN 13

const char* getStateName(enum arm_state state) 
{                                                                                                                
   switch (state) 
   {
      case STOPPED: return    "Stopped";
      case MOVING_IN: return  "Moving In";
      case MOVING_OUT: return "Moving Out";
      case LIMIT_IN: return   "Limit In";
      case LIMIT_OUT: return  "Limit Out";
      default: return "";
   }
}

// Create an instance of the server
WiFiServer server(LISTEN_PORT);
void printWifiStatus();

int handle_rest_client() {
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return 0;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);
  return 0;
  // Serial.print("~");
}

void print_serial() {
    Serial.print("S- ");
    Serial.print(state);
    Serial.print(" switchIn: ");  Serial.print(switchIn);
    Serial.print(" switchOut: ");  Serial.print(switchOut);
    Serial.print(" sensorLimIn: ");  Serial.print(sensorLimIn);
    Serial.print(" sensorLimOut: ");  Serial.print(sensorLimOut);
    Serial.print(": ");
    //Serial.println(lastRefreshTime);
    // if (ledOn) {
    // Serial.print("+ ");
    //} else {
    // Serial.print("/ ");
    //}

}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}



void setup()
{
  int i = 0;

  char rest_label[8] = "state";
  char rest_name[10] = "esp8266";
  // initialize LED digital pin as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_OUT, OUTPUT);
  pinMode(LED_IN, OUTPUT);
  pinMode(RELAY_IN, OUTPUT);
  pinMode(RELAY_OUT, OUTPUT);

  pinMode(SWITCH_IN, INPUT_PULLUP);
  pinMode(SWITCH_OUT, INPUT_PULLUP);

  pinMode(SEN_LIM_OUT, INPUT_PULLUP);
  pinMode(SEN_LIM_IN,  INPUT_PULLUP);

  digitalWrite(RELAY_OUT, RELAY_OFF);
  digitalWrite(RELAY_IN, RELAY_OFF);

  // Start serial port
  Serial.begin(115200);
/*
  Serial.println("Trying WiFi ....");

  // Create UI
  rest.title("Relay Control");
  rest.button(SWITCH_IN); // 
  rest.variable("state", &state);
  rest.label(rest_label);

  // Give name and ID to device
  rest.set_id("1");
  rest.set_name(rest_name);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (i++ > 20) {
      Serial.println("Can't connect, Quitting...");
      break;
    }
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // digitalWrite(LED_YELLOW, HIGH);
  // delay(500);
  // digitalWrite(LED_YELLOW, LOW);

  // Start the server
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  // Serial.println(WiFi.localIP());
  // Print out the status
  printWifiStatus();
*/
}

void loop()     //fast Loop
{


  const long debounce = 1000; // 0.2 s
  const long debounce2 = 500; // 0.2 s

  sensorLimIn = digitalRead(SEN_LIM_IN);
  sensorLimOut = digitalRead(SEN_LIM_OUT);
  switchIn = (digitalRead(SWITCH_IN))? false : true;
  switchOut = (digitalRead(SWITCH_OUT))? false : true;
  //switchIn = digitalRead(SWITCH_IN);
  //switchOut =  digitalRead(SWITCH_OUT);

  unsigned long now = millis();

  switch (state)
  {
  case STOPPED:
    digitalWrite(RELAY_IN, RELAY_OFF);
    digitalWrite(RELAY_OUT, RELAY_OFF);
    digitalWrite(LED_OUT, LED_OFF);
    digitalWrite(LED_IN, LED_OFF);
    if (sensorLimIn){
      state = LIMIT_IN;
    }
    else if (sensorLimOut){
      state = LIMIT_OUT;
    }
    else if (sensorLimOut){
      state = LIMIT_OUT;
    }
    else if (now > holding) {
      if (switchIn && !sensorLimIn ){
          holding = now + debounce;
          state = MOVING_IN;
          Serial.println(F("STOP->IN"));

      }
      else if ( switchOut && !sensorLimOut ){
        holding = now + debounce;
        state = MOVING_OUT;
      }
    }
  //digitalWrite(RELAY_IN, HIGH);
    break;
  case MOVING_IN:
    digitalWrite(RELAY_IN, RELAY_ON);
    digitalWrite(RELAY_OUT, RELAY_OFF);
    digitalWrite(LED_OUT, LED_OFF); //
    digitalWrite(LED_IN, LED_ON);
    if (sensorLimIn){
      state = LIMIT_IN;
    }
    else if ((switchIn  && (now > holding)) || switchOut){
      holding = now + debounce2;
      state = STOPPED;
      //Serial.println(F("IN->STOP")) ;
    }
    break;
  case MOVING_OUT:
      digitalWrite(RELAY_OUT, RELAY_ON);
      digitalWrite(RELAY_IN, RELAY_OFF);
      digitalWrite(LED_OUT, LED_ON);
      digitalWrite(LED_IN, LED_OFF);
      if (sensorLimOut){
      //Serial.println(F("Moving->LIMIT_OUT"));
        state = LIMIT_OUT;
      }
    
      if ((switchOut && (now > holding)) || switchIn ){
        holding = now + debounce2;
        state = STOPPED;
        //Serial.println(F("OUT->STOP"));
      }
    break;    
  case LIMIT_IN:
      digitalWrite(RELAY_IN, RELAY_OFF);
      digitalWrite(RELAY_OUT, RELAY_OFF);
      if (sensorLimOut){
        state = ERROR;
      }
      // Blinking on loop2
      else if (switchOut) {
        //holding = now + debounce2;
        state = MOVING_OUT;
      }
    break;
  case LIMIT_OUT:
    digitalWrite(RELAY_IN, RELAY_OFF);
    digitalWrite(RELAY_OUT, RELAY_OFF);
    if (sensorLimIn){
      state = ERROR;
    }
    // Blinking on loop2
    else if (switchIn){
      //holding = now + debounce2;
      state = MOVING_IN;
    }
    break;
  case ERROR:   
     if (!sensorLimIn && !sensorLimOut){
      state = STOPPED;
    } 
    break;
  default:;
  }

  loop3();  // Slow print loop
  loop2();  // Slow blink loop
}


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
      case LIMIT_IN:  
          digitalWrite(LED_IN, led_state);
          digitalWrite(LED_OUT, LED_OFF);
        break;          
      case LIMIT_OUT:  
          digitalWrite(LED_IN, LED_OFF);
          digitalWrite(LED_OUT, led_state); 
      case ERROR:   
          digitalWrite(LED_IN, led_state);
          digitalWrite(LED_OUT, led_state);
        break;          
      default:;
    }
  }
}

void loop3() {

  static unsigned long nextTime = 0;
  const long interval = 2000;
  static bool led_state = 0;

  unsigned long now = millis();

  if ( now > nextTime ) {  
    nextTime = now + interval; 
    led_state = not led_state;
     //Serial.print(state);
    Serial.print(getStateName(state));
    Serial.print(F(", Limit IN: "));
    Serial.print(sensorLimIn, DEC);
    Serial.print(F(", Limit OUT: "));
    Serial.print(sensorLimOut, DEC);
    Serial.print(F(", Now "));
    Serial.print(now, DEC);
    Serial.print(F(", holding "));
    Serial.print(holding, DEC);
   /* Then, later in main: */
  //printf("%s", getDayName(TheDay));
   Serial.println(F(" end"));
/*   
     if (led_state ) {
         digitalWrite(RELAY_OUT, LOW);
         digitalWrite(RELAY_IN, LOW);
         
     }
    else {
         digitalWrite(RELAY_OUT, HIGH);
         digitalWrite(RELAY_IN, HIGH);         
    }
    */
    print_serial();
  }

  // printf("%s", getDayName(TheDay));
}
