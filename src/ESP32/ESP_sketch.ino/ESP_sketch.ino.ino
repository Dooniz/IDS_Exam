#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <FastLED.h>


// WiFi
const char* ssid = "RUC-IOT"; //replace with your WiFi SSID
const char* password = "GiHa5934La"; //replace with your WiFi password

WiFiClient espClient;
PubSubClient client(espClient);

// MQTT
const char* mqttServer = "public.cloud.shiftr.io";
const int mqttPort = 1883;
const char* mqttUser = "public";
const char* mqttPassword = "public";
const char* mqttTopic = "esp32/Wind_Speed";

// NeoPixel
#define LED_PIN 17
#define NUM_LEDS 21

CRGB leds[NUM_LEDS];


// Function to parse wind speed from MQTT message
//allow non decimal numbers as well, but return -1 if the format is invalid
float parseWindSpeed(String msg) {
  msg.trim(); // Remove any leading/trailing whitespace
  if (msg.length() == 0) return -1.0; // Empty message

  // Check if the message is a valid number (integer or float)
  bool isValid = true;
  bool hasDecimal = false;
  for (size_t i = 0; i < msg.length(); i++) {
    char c = msg.charAt(i);
    if (c == '.') {
      if (hasDecimal) {
        isValid = false; // More than one decimal point
        break;
      }
      hasDecimal = true;
    } else if (!isDigit(c)) {
      isValid = false; // Non-digit character
      break;
    }
  }

  if (!isValid) {
    return -1.0;
  }
  return msg.toFloat();
}

//Setup wifi function
void setup_wifi() {
  if (WiFi.status() == WL_CONNECTED) return; // Already connected


  Serial.print("Connecting to WiFi...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi reconnected!");
  } else {
    Serial.println("\nWiFi reconnect failed!");
  }
}


//Reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

//callback function for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  //parse windspeed to float
  float windSpeed = parseWindSpeed(msg);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);


  if (windSpeed == -1.0) {
    Serial.println("Failed to parse wind speed");
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
    }
    FastLED.show();
    return;
  } 
  //show wind speed by moving a white light across the strip with delay based on the speed
  else {
    int delayTime = map(windSpeed, 0, 20, 100, 10); // Map wind speed to delay time (0-20 m/s to 100-10 ms)
    for (int i = 0; i < 4; i++) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::White;
        FastLED.show();
        delay(delayTime);
        leds[i] = CRGB::Black;
    }
  }
  }
}



void setup() {
  Serial.begin(115200);

  // Initialize NeoPixel
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Black;
  FastLED.show();

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }

client.loop();

}
