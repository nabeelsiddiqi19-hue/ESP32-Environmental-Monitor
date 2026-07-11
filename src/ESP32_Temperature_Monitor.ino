// =====================================================
//  WiFi Environmental Monitor — Freenove ESP32-S3
//  Sensors: Thermistor, Potentiometer, Joystick
//  Outputs: LCD, Servo, RGB LED, Buzzer, Adafruit IO
// =====================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiS3.h>
#include <Servo.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//  ───────────────────────────────
#define WIFI_SSID    "Wifi_Name"      
#define WIFI_PASS    "Wifi_Password"    
#define AIO_USERNAME "Username"  
#define AIO_KEY      "AdaFruit_Key"

// ─────────────────────────────────────────────────────

// Pins
#define THERM_PIN   A0
#define POT_PIN     A2
#define SERVO_PIN   5
#define BUZZER_PIN  9
#define RGB_R       6
#define RGB_G       8
#define RGB_B       3
#define JOY_X       A1
#define JOY_Y       A3
#define JOY_SW      4

// LCD screens
#define SCREEN_TEMP   0
#define SCREEN_STATUS 1
#define SCREEN_MINMAX 2
#define TOTAL_SCREENS 3

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

WiFiClient wifiClient;
Adafruit_MQTT_Client mqtt(&wifiClient, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish tempFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

float tempMin = 999;
float tempMax = -999;
int currentScreen = 0;
int lastScreen = -1;
bool lastJoyPress = false;

float smoothedTemp = 23.0;
const float filterFactor = 0.08;

unsigned long lastPublishTime = 0;
#define PUBLISH_INTERVAL 10000

unsigned long lastLCDUpdateTime = 0;
#define LCD_UPDATE_INTERVAL 300

void setRGB(int r, int g, int b) {
  analogWrite(RGB_R, 255 - r);
  analogWrite(RGB_G, 255 - g);
  analogWrite(RGB_B, 255 - b);
}

void updateRGB(float temp) {
  if (temp < 20) {
    setRGB(0, 0, 255);
  } else if (temp < 27) {
    setRGB(0, 255, 0);
  } else if (temp < 33) {
    setRGB(255, 100, 0);
  } else {
    setRGB(255, 0, 0);
  }
}

float readTemperature() {
  int raw = analogRead(THERM_PIN);
  if (raw == 0 || raw >= 1023) return smoothedTemp;
  float voltage = raw * (3.3 / 1023.0);
  float resistance = (10000.0 * (3.3 - voltage)) / voltage;
  float tempK = 1.0 / (0.001129 + 0.000234 * log(resistance)
              + 0.0000000877 * pow(log(resistance), 3));
  return tempK - 273.15;
}

float readThreshold() {
  return map(analogRead(POT_PIN), 0, 1023, 20, 45);
}

void checkJoystick() {
  int joyY = analogRead(JOY_Y);
  bool joyPressed = (digitalRead(JOY_SW) == LOW);

  if (joyY < 100) {
    currentScreen = (currentScreen + 1) % TOTAL_SCREENS;
    delay(300);
  } else if (joyY > 900) {
    currentScreen = (currentScreen - 1 + TOTAL_SCREENS) % TOTAL_SCREENS;
    delay(300);
  }

  if (joyPressed && !lastJoyPress) {
    tempMin = 999;
    tempMax = -999;
    lcd.clear();
    lcd.print("Min/Max reset!");
    delay(800);
    lastScreen = -1;
  }
  lastJoyPress = joyPressed;
}

void updateLCD(float temp, float threshold) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastLCDUpdateTime >= LCD_UPDATE_INTERVAL || currentScreen != lastScreen) {
    lastLCDUpdateTime = currentMillis;
    lastScreen = currentScreen;
    lcd.clear();
    switch (currentScreen) {
      case SCREEN_TEMP:
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temp, 1);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Alert at: ");
        lcd.print((int)threshold);
        lcd.print(" C");
        break;
      case SCREEN_STATUS:
        lcd.setCursor(0, 0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.status() == WL_CONNECTED ? "OK" : "OFF");
        lcd.setCursor(0, 1);
        lcd.print("IO: ");
        lcd.print(mqtt.connected() ? "Connected" : "Offline");
        break;
      case SCREEN_MINMAX:
        lcd.setCursor(0, 0);
        lcd.print("Max: ");
        lcd.print(tempMax, 1);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Min: ");
        lcd.print(tempMin, 1);
        lcd.print(" C");
        break;
    }
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void connectMQTT() {
  if (mqtt.connected()) return;
  Serial.print("Connecting to Adafruit IO... ");
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("Adafruit IO connected!");
}

void setup() {
  Serial.begin(115200);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  pinMode(JOY_SW, INPUT_PULLUP);
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  lcd.init();
  lcd.backlight();
  lcd.print("Starting up...");
  setRGB(0, 0, 0);
  connectWiFi();
  connectMQTT();
  lcd.clear();
  lcd.print("Monitor Ready!");
  delay(1000);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.processPackets(10);

  float rawTemp   = readTemperature();
  float threshold = readThreshold();

  smoothedTemp = (rawTemp * filterFactor) + (smoothedTemp * (1.0 - filterFactor));

  if (smoothedTemp > tempMax) tempMax = smoothedTemp;
  if (smoothedTemp < tempMin) tempMin = smoothedTemp;

  myServo.write(constrain(map((int)smoothedTemp, 15, 40, 0, 180), 0, 180));

  updateRGB(smoothedTemp);
  checkJoystick();
  updateLCD(smoothedTemp, threshold);

  // Police siren alert
  if (smoothedTemp > threshold) {
    for (int freq = 800; freq <= 1600; freq += 20) {
      tone(BUZZER_PIN, freq);
      delay(2);
    }
    for (int freq = 1600; freq >= 800; freq -= 20) {
      tone(BUZZER_PIN, freq);
      delay(2);
    }
  } else {
    noTone(BUZZER_PIN);
  }

  Serial.print("Temp: "); Serial.print(smoothedTemp);
  Serial.print(" C | Threshold: "); Serial.print(threshold);
  Serial.print(" | Screen: "); Serial.println(currentScreen);

  unsigned long now = millis();
  if (now - lastPublishTime >= PUBLISH_INTERVAL) {
    if (tempFeed.publish(smoothedTemp)) {
      Serial.println("Published to Adafruit IO!");
    } else {
      Serial.println("Publish failed.");
    }
    lastPublishTime = now;
  }

  delay(50);
}
