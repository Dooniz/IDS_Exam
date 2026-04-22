
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>


// WiFi
const char* ssid = "RUC-IOT";
const char* password = "GiHa5934La";

// MQTT
const char* mqttServer = "public.cloud.shiftr.io";
const int mqttPort = 1883;
const char* mqttUser = "public";
const char* mqttPassword = "public";
const char* mqttTopic = "esp32IDS/vandkvalitet";
//^^Topic like in p5

// Button
const int buttonPin = 27;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// NeoPixel
#define LED_PIN 17
#define NUM_LEDS 40



CRGB leds[NUM_LEDS];

// LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  if (WiFi.status() == WL_CONNECTED) return; // Already connected

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected to");
    lcd.setCursor(0, 1);
    lcd.print("Internet");
  } else {
    Serial.println("\nWiFi reconnect failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi failed");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);

  // Display MQTT message on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQTT Msg:");
  lcd.setCursor(0, 1);
  lcd.print(msg.substring(0, 16));

  int red = 50;
  int grn = 168;
  int blu = 70;

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
  } else if (msg == "bad") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
    }
    FastLED.show();
  } else if (msg == "good") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
  } else if (msg == "medium") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Yellow;
    }
    FastLED.show();
  } else if (msg == "party") {
      while (msg == "party") {
        FastLED.setBrightness(150);
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(random(255), random(255), random(255));
    } 
    FastLED.show(); 
    delay(500);
    if (msg != "party") {
      break;
    }
    }
  } 
}

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

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialize NeoPixel
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Black;
  FastLED.show();

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  // WiFi status handling
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Button press to publish message
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    if (millis() - lastDebounceTime > debounceDelay) {
      Serial.println("Button pressed! Sending MQTT message...");
      client.publish(mqttTopic, "Button Pressed");

      // Optional: Show on LCD when sending
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sent:");
      lcd.setCursor(0, 1);
      lcd.print("Button Pressed");

      lastDebounceTime = millis();
    }
  }
  lastButtonState = buttonState;
}
