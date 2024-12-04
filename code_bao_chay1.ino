// Define the Blynk template details (for app integration)
#define BLYNK_TEMPLATE_ID "TMPL69odiICtd"   // Template ID from Blynk
#define BLYNK_TEMPLATE_NAME "test"          // Template name
#define BLYNK_AUTH_TOKEN "v1pQHXX8Zxk6K9rd1yQz8kzjtanzz4QQ" // Authentication token for the Blynk project

#include <ESP8266WiFi.h>         // ESP8266 WiFi library
#include <BlynkSimpleEsp8266.h>  // Blynk library for ESP8266

// Wi-Fi credentials for connecting to the network
char ssid[] = "Iphone";         // Wi-Fi SSID
char pass[] = "12343210";       // Wi-Fi password

// Pin assignments
const int sensorPin = D6;          // Fire sensor pin
const int buzzerPin = D8;          // Buzzer pin
const int pumpIN3 = D7;            // Pump control pin
const int physicalButtonPin = D5;  // Physical button pin

// Variables to store system states
bool autoMode = false;            // Auto mode status (controlled via virtual pin V1)
bool buttonState = false;         // Manual button state (controlled via virtual pin V2)
int sensorValue = 0;              // Fire sensor value (0 = safe, 1 = fire detected)
bool lastPhysicalButtonState = HIGH; // Stores the last state of the physical button

BlynkTimer timer;                 // Timer object for scheduled tasks

// Timing variables for non-blocking delays
unsigned long previousMillis = 0;
const unsigned long interval = 1000; // Interval of 1 second

// Function to control the pump and buzzer
void controlPumpAndBuzzer(bool state) {
  digitalWrite(pumpIN3, state);  // Set pump state (ON/OFF)
  digitalWrite(buzzerPin, state); // Set buzzer state (ON/OFF)
}

// Function to check the fire sensor and control devices accordingly
void checkSensor() {
  sensorValue = digitalRead(sensorPin);  // Read fire sensor value (1 = fire, 0 = safe)

  // Send sensor status to Blynk virtual pin V3
  if (sensorValue == 0) {
    Blynk.virtualWrite(V3, "Safe");           // If safe, send "Safe" to Blynk
  } else {
    Blynk.virtualWrite(V3, "Fire Detection"); // If fire detected, send "Fire Detection" to Blynk
  }

  if (autoMode) {
    // In auto mode, control pump and buzzer based on sensor value
    controlPumpAndBuzzer(sensorValue == 0); // Activate if fire detected
  } else {
    // In manual mode, control pump and buzzer based on manual button state
    controlPumpAndBuzzer(buttonState);
  }

  // Notify users in case of fire
  notifyOnFireAlert();
}

// Blynk virtual pin V1 handler (for auto mode toggle)
BLYNK_WRITE(V1) {
  autoMode = param.asInt();  // Read the value of V1 (1 = ON, 0 = OFF)
}

// Blynk virtual pin V2 handler (for manual button toggle)
BLYNK_WRITE(V2) {
  buttonState = param.asInt();  // Read the value of V2 (1 = ON, 0 = OFF)
}

// Function to handle the physical button
static void handlePhysicalButton() {
  bool physicalButtonState = digitalRead(physicalButtonPin); // Read the current button state

  // Check for button press (state changes from HIGH to LOW)
  if (physicalButtonState == LOW && lastPhysicalButtonState == HIGH) {
    autoMode = false;             // Disable auto mode
    Blynk.virtualWrite(V1, 0);    // Turn off auto mode on Blynk
    buttonState = !buttonState;   // Toggle the manual button state
    controlPumpAndBuzzer(buttonState); // Update pump and buzzer based on the button state
  }

  // Save the current button state for the next loop
  lastPhysicalButtonState = physicalButtonState;
}

// Function to send notifications in case of fire
void notifyOnFireAlert() {
  if (sensorValue == 0) {
    // If fire detected
    Serial.println("Warning your area is on fire!!!");

    // Send notification and event log to Blynk
    Blynk.virtualWrite(V4, "Fire Alert in Home"); // Show "Fire Alert" on Blynk
    Blynk.logEvent("fire_warning", "Warning your area is on fire!!!"); // Log the fire warning
  } else {
    // If no fire
    Blynk.virtualWrite(V4, "Home is Safe"); // Show "Home is Safe" on Blynk
  }
}

// Setup function (runs once at the beginning)
void setup() {
  Serial.begin(9600);           // Initialize serial communication for debugging

  // Configure pin modes
  pinMode(sensorPin, INPUT);           // Fire sensor pin as input
  pinMode(buzzerPin, OUTPUT);          // Buzzer pin as output
  pinMode(pumpIN3, OUTPUT);            // Pump pin as output
  pinMode(physicalButtonPin, INPUT_PULLUP); // Physical button pin with internal pull-up resistor

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Set up a timer to check the fire sensor every 1 second
  timer.setInterval(1000L, checkSensor);
}

// Loop function (runs continuously)
void loop() {
  // Run Blynk if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }

  // Run the timer for scheduled tasks
  timer.run();

  // Check the physical button
  handlePhysicalButton();
}
