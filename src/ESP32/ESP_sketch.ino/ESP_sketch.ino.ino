#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <FastLED.h>


// WiFi
const char* ssid = "RUC-IOT";
const char* password = "GiHa5934La";

WiFiClient espClient;
PubSubClient client(espClient);

// MQTT
const char* mqttServer = "public.cloud.shiftr.io";
const int mqttPort = 1883;
const char* mqttUser = "public";
const char* mqttPassword = "public";
const char* mqttTopic = "esp32IDS/vandkvalitet";

// NeoPixel
#define LED_PIN 17
#define NUM_LEDS 40

CRGB leds[NUM_LEDS];


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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);


  // Control NeoPixel
  if (msg == "LED_ON") {
    for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
    }
    FastLED.show();
  } else if (msg == "LED_OFF") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
  }
  else if (msg == "party") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(random(0, 255), 255, 255);
    }
    FastLED.show();
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

client.setCallback(callback);

if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

if (!client.connected()) {
    reconnect();
}

client.loop();

}
