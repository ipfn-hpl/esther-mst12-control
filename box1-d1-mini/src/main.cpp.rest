// Relay control using the ESP8266 WiFi chip
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

enum State_enum { OFF_LIMITS, IN_LIMIT, OUT_LIMIT, ERROR };
// enum Sensors_enum { NONE, SENSOR_RIGHT, SENSOR_LEFT, BOTH };

uint8_t state = OFF_LIMITS;

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80
#define LED_WEMOOS_D1_MINI 2 // Pin D4 GPIO2
#define LIMIT_OUT 14         // D5
#define LIMIT_IN 12          // D6
#define SWITCH_3_IN 5        // D1
#define SWITCH_3_OUT 4       // D2
#define RELAY_IN 2           // D4  Relay Output HIGH = OFF (NC)
#define RELAY_OUT 0          // D3  Relay Output
#define LED_IN 16            // D0
#define LED_OUT 13           // D7

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
void state_machine_run() // uint8_t sensors)
{
  int sensorIn = digitalRead(SWITCH_3_IN);
  int sensorOut = digitalRead(SWITCH_3_OUT);
  int sensorLimIn = digitalRead(LIMIT_IN);
  int sensorLimOut = digitalRead(LIMIT_OUT);
  // int sensorLimOut = 0; // No Sensor yet  digitalRead(LIMIT_OUT);

  switch (state) {
  case OFF_LIMITS:
    if (sensorLimIn) {
      state = IN_LIMIT;
    } else if (sensorLimOut) {
      state = OUT_LIMIT;
    } else if (!sensorIn) {
      digitalWrite(LED_IN, HIGH);
      digitalWrite(LED_OUT, LOW);
      digitalWrite(RELAY_IN, HIGH);
      digitalWrite(RELAY_OUT, LOW);
    } else if (!sensorOut) {
      digitalWrite(LED_IN, LOW);
      digitalWrite(LED_OUT, HIGH);
      digitalWrite(RELAY_IN, LOW);
      digitalWrite(RELAY_OUT, HIGH);
    } else {
      digitalWrite(LED_IN, LOW);
      digitalWrite(LED_OUT, LOW);
      digitalWrite(RELAY_IN, HIGH);
      digitalWrite(RELAY_OUT, HIGH);
    }

    break;

  case IN_LIMIT:
    if (!sensorLimIn) {
      state = OFF_LIMITS;
    } else if (sensorLimOut) {
      state = ERROR;
    } else if (!sensorOut) {
      digitalWrite(RELAY_IN, LOW);
      digitalWrite(RELAY_OUT, HIGH);
    } else {
      digitalWrite(RELAY_IN, HIGH);
      digitalWrite(RELAY_OUT, HIGH);
    }
    break;
  case OUT_LIMIT:
    if (!sensorLimOut) {
      state = OFF_LIMITS;
    } else if (sensorLimIn) {
      state = ERROR;
    } else if (!sensorIn) {
      digitalWrite(RELAY_IN, HIGH);
      digitalWrite(RELAY_OUT, LOW);
    } else {
      digitalWrite(RELAY_IN, HIGH);
      digitalWrite(RELAY_OUT, HIGH);
    }
    break;
  case ERROR:
    digitalWrite(RELAY_IN, HIGH);
    digitalWrite(RELAY_OUT, HIGH);
    if (!sensorLimIn && !sensorLimOut) {
      state = OFF_LIMITS;
    }
    break;
  }
}
void setup(void) {
  // pinMode(15, LIMIT_O); // D8
  // pinMode(16, OUTPUT); // D0
  pinMode(LED_OUT, OUTPUT);   // D
  pinMode(LED_IN, OUTPUT);    // D
  pinMode(RELAY_OUT, OUTPUT); // D
  pinMode(RELAY_IN, OUTPUT);  // D
  digitalWrite(RELAY_IN, HIGH);
  digitalWrite(RELAY_OUT, HIGH);
  // pinMode(4, INPUT_PULLUP); // D2  3-state SWITCH
  pinMode(SWITCH_3_OUT, INPUT_PULLUP); // D
  pinMode(SWITCH_3_IN, INPUT_PULLUP);  // D
  pinMode(LIMIT_OUT, INPUT_PULLUP);    // D
  pinMode(LIMIT_IN, INPUT_PULLUP);     // D

  // Start Serial
  int i = 0;
  Serial.begin(115200);
  Serial.println("Trying WiFi ....");

  // Create UI
  rest.title("Relay Control");
  rest.button(4); // LED
  rest.variable("state", &state);
  rest.label("state");

  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");

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
}

void loop() {
  static const unsigned long REFRESH_INTERVAL = 1000; // ms
  static unsigned long lastRefreshTime = 0;
  static bool ledOn = true;
  // unsigned long mills;
  // int sensorVal = digitalRead(5);
  // digitalWrite(13,sensorVal);

  if (millis() - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime += REFRESH_INTERVAL;
    Serial.print("S- ");
    Serial.print(state);
    Serial.print(": ");
    Serial.println(lastRefreshTime);
    // digitalWrite(0, ledOn); // D3
    // digitalWrite(12, ledOn);
    // digitalWrite(16, ledOn);
    //// digitalWrite(13, ledOn);
    //  digitalWrite(15, ledOn);
    // if (ledOn) {
    // Serial.print("+ ");
    //} else {
    // Serial.print("/ ");
    //}
    ledOn = not ledOn;
  }
  state_machine_run();
  handle_rest_client();
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
