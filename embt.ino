#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_BMP085_U.h>
#include "DHT.h"
#include <math.h>
#include <WiFi.h>
#include <ThingSpeak.h>

// WiFi credentials
const char* ssid = "DINESHROG";
const char* password = "Rtx4090@";

// ThingSpeak credentials
unsigned long channelID = 2906776;
const char* writeAPIKey = "3VYSPLHNY7MKWNSS";

// Create WiFi client
WiFiClient client;

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BMP180 Setup
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
#define seaLevelPressure_hPa 1013.25

// DHT Setup
#define DHT_PIN 32
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// MQ Sensor Setup
#define MQ4_PIN 34
#define MQ135_PIN 35
#define MQ4_LED_GREEN 2
#define MQ4_LED_YELLOW 4
#define MQ4_LED_RED 16
#define MQ135_LED_GREEN 5
#define MQ135_LED_YELLOW 18
#define MQ135_LED_RED 19
#define RL_VALUE 10.0
#define R0_MQ4 9.83
#define R0_MQ135 9.83

// Voltage Sensor Setup
#define VOLTAGE_SENSOR_PIN 33
#define REF_VOLTAGE    3.3
#define ADC_RESOLUTION 4095.0
#define R1             30000.0
#define R2             7500.0

// Vibration Sensor Setup
int Vibration_signal = 14; // Define the Digital Input on the ESP32 for the sensor signal
int Sensor_State = 1;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  if (!bmp.begin()) {
    Serial.println("BMP180 not found!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (1);
  }

  display.clearDisplay();
  analogSetAttenuation(ADC_11db);

  // LED Pins
  pinMode(MQ4_LED_GREEN, OUTPUT);
  pinMode(MQ4_LED_YELLOW, OUTPUT);
  pinMode(MQ4_LED_RED, OUTPUT);
  pinMode(MQ135_LED_GREEN, OUTPUT);
  pinMode(MQ135_LED_YELLOW, OUTPUT);
  pinMode(MQ135_LED_RED, OUTPUT);
  pinMode(Vibration_signal, INPUT);  // Set vibration sensor pin as input
}

float calculatePPM_MQ4(float Rs) {
  return 1021 * pow((Rs / R0_MQ4), -2.7887);
}

float calculatePPM_MQ135(float Rs) {
  return 116.602 * pow((Rs / R0_MQ135), -2.769);
}

String getAirQualityCategory(float ppm, float goodThreshold, float moderateThreshold) {
  if (ppm < goodThreshold) return "Good";
  else if (ppm < moderateThreshold) return "Moderate";
  else return "Poor";
}

void sendToThingSpeak(float dhtTemp, float humidity, float bmpTemp, float pressure, 
                     float altitude, float voltage, float mq4, float mq135) {
  // Set the fields with the values
  ThingSpeak.setField(1, dhtTemp);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, bmpTemp);
  ThingSpeak.setField(4, pressure);
  ThingSpeak.setField(5, altitude);
  ThingSpeak.setField(6, voltage);
  ThingSpeak.setField(7, mq4);
  ThingSpeak.setField(8, mq135);
  
  // Write to ThingSpeak
  int response = ThingSpeak.writeFields(channelID, writeAPIKey);
  
  if (response == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(response));
  }
}

void loop() {
  // Read Vibration Sensor
  Sensor_State = digitalRead(Vibration_signal);

  // Read DHT22
  float temperature_dht = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read MQ-4
  int val_MQ4 = analogRead(MQ4_PIN);
  float volt_MQ4 = (val_MQ4 / ADC_RESOLUTION) * 5.0;
  float Rs_MQ4 = RL_VALUE * ((5.0 / volt_MQ4) - 1);
  float ppm_MQ4 = calculatePPM_MQ4(Rs_MQ4);
  String aq_MQ4 = getAirQualityCategory(ppm_MQ4, 300, 600);

  // Read MQ-135
  int val_MQ135 = analogRead(MQ135_PIN);
  float volt_MQ135 = (val_MQ135 / ADC_RESOLUTION) * 5.0;
  float Rs_MQ135 = RL_VALUE * ((5.0 / volt_MQ135) - 1);
  float ppm_MQ135 = calculatePPM_MQ135(Rs_MQ135);
  String aq_MQ135 = getAirQualityCategory(ppm_MQ135, 400, 1000);

  // Update LEDs
  digitalWrite(MQ4_LED_GREEN, aq_MQ4 == "Good");
  digitalWrite(MQ4_LED_YELLOW, aq_MQ4 == "Moderate");
  digitalWrite(MQ4_LED_RED, aq_MQ4 == "Poor");

  digitalWrite(MQ135_LED_GREEN, aq_MQ135 == "Good");
  digitalWrite(MQ135_LED_YELLOW, aq_MQ135 == "Moderate");
  digitalWrite(MQ135_LED_RED, aq_MQ135 == "Poor");

  // Read BMP180
  sensors_event_t event;
  bmp.getEvent(&event);
  float pressure = event.pressure;
  float temperature_bmp;
  bmp.getTemperature(&temperature_bmp);
  float altitude = 44330 * (1 - pow((pressure / seaLevelPressure_hPa), 1.0 / 5.255));

  // Read Voltage Sensor
  int adc_val = analogRead(VOLTAGE_SENSOR_PIN);
  float volt_adc = ((float)adc_val * REF_VOLTAGE) / ADC_RESOLUTION;
  float voltage_in = volt_adc * (R1 + R2) / R2;

  // Serial Output
  Serial.println("---------------");
  Serial.printf("Vibration Status: %s\n", (Sensor_State == 1) ? "Vibration Detected" : "No Vibration");
  Serial.printf("MQ-4: %.2f ppm (%s)\n", ppm_MQ4, aq_MQ4.c_str());
  Serial.printf("MQ-135: %.2f ppm (%s)\n", ppm_MQ135, aq_MQ135.c_str());
  Serial.printf("DHT Temp: %.2f C | Humidity: %.2f%%\n", temperature_dht, humidity);
  Serial.printf("BMP Temp: %.2f C | Pressure: %.2f hPa | Altitude: %.2f m\n", temperature_bmp, pressure, altitude);
  Serial.printf("Voltage: %.2f V\n", voltage_in);

  // OLED Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("Vibration: %s\n", (Sensor_State == 1) ? "Detected" : "None");
  display.printf("MQ4: %.2f ppm (%s)\n", ppm_MQ4, aq_MQ4.c_str());
  display.printf("MQ135: %.2f ppm (%s)\n", ppm_MQ135, aq_MQ135.c_str());
  display.printf("T:%.1fC H:%.0f%%\n", temperature_dht, humidity);
  display.printf("P:%.0fhPa A:%.0fm\n", pressure, altitude);
  display.printf("V:%.2fV\n", voltage_in);
  display.display();

  // Send data to ThingSpeak
  sendToThingSpeak(temperature_dht, humidity, temperature_bmp, pressure, 
                  altitude, voltage_in, ppm_MQ4, ppm_MQ135);

  // ThingSpeak requires at least 15 seconds between updates
  delay(20000); // Wait 20 seconds before next update
}