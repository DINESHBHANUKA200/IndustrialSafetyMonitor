#include <Wire.h>
#include <Adafruit_BMP085_U.h>
#include "DHT.h"

// BMP180 setup
Adafruit_BMP085_Unified bmp;
#define seaLevelPressure_hPa 1013.25 // Adjust according to your location

// DHT22 setup
#define DHT_PIN 32        
#define DHT_TYPE DHT22    
DHT dht(DHT_PIN, DHT_TYPE);

// MQ-4 and MQ-135 setup
#define MQ4_PIN 34        
#define MQ135_PIN 35      
#define RL_VALUE 10.0     
#define R0_MQ4 9.83       
#define R0_MQ135 9.83     

// LED pins for MQ-4
#define MQ4_LED_GREEN 2   
#define MQ4_LED_YELLOW 4  
#define MQ4_LED_RED 16    

// LED pins for MQ-135
#define MQ135_LED_GREEN 5   
#define MQ135_LED_YELLOW 18 
#define MQ135_LED_RED 19    

// ACS712 setup
const int sensorIn = 34;      // pin where the OUT pin from sensor is connected on Arduino
int mVperAmp = 185;           // this the 5A version of the ACS712 -use 100 for 20A Module and 66 for 30A Module
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

// Function to calculate PPM from Rs and R0 for MQ-4
float calculatePPM_MQ4(float Rs) {
    return 1021 * pow((Rs / R0_MQ4), -2.7887);
}

// Function to calculate PPM from Rs and R0 for MQ-135
float calculatePPM_MQ135(float Rs) {
    return 116.602 * pow((Rs / R0_MQ135), -2.769);
}

// Function to categorize air quality based on PPM
String getAirQualityCategory(float ppm, float goodThreshold, float moderateThreshold) {
    if (ppm < goodThreshold) {
        return "Good";
    } else if (ppm < moderateThreshold) {
        return "Moderate";
    } else {
        return "Poor";
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize BMP180
    if (!bmp.begin()) {
        Serial.println("Could not find a valid BMP180 sensor, check wiring!");
        while (1) {}
    }

    // Initialize DHT22
    dht.begin();

    // Set LED pins as output
    pinMode(MQ4_LED_GREEN, OUTPUT);
    pinMode(MQ4_LED_YELLOW, OUTPUT);
    pinMode(MQ4_LED_RED, OUTPUT);
    pinMode(MQ135_LED_GREEN, OUTPUT);
    pinMode(MQ135_LED_YELLOW, OUTPUT);
    pinMode(MQ135_LED_RED, OUTPUT);
}

void loop() {
    // Read BMP180 data
    float temperature_bmp;
    bmp.getTemperature(&temperature_bmp);
    float pressure;
    bmp.getPressure(&pressure);
    float altitude = bmp.pressureToAltitude(seaLevelPressure_hPa, pressure);

    // Read and calculate MQ-4 values
    int analogValue_MQ4 = analogRead(MQ4_PIN);
    float voltage_MQ4 = (analogValue_MQ4 / 4095.0) * 5.0;
    float Rs_MQ4 = RL_VALUE * ((5.0 / voltage_MQ4) - 1);
    float ppm_MQ4 = calculatePPM_MQ4(Rs_MQ4);
    String airQuality_MQ4 = getAirQualityCategory(ppm_MQ4, 300, 600);

    // Read and calculate MQ-135 values
    int analogValue_MQ135 = analogRead(MQ135_PIN);
    float voltage_MQ135 = (analogValue_MQ135 / 4095.0) * 5.0;
    float Rs_MQ135 = RL_VALUE * ((5.0 / voltage_MQ135) - 1);
    float ppm_MQ135 = calculatePPM_MQ135(Rs_MQ135);
    String airQuality_MQ135 = getAirQualityCategory(ppm_MQ135, 400, 1000);

    // Read temperature and humidity from DHT sensor
    float humidity = dht.readHumidity();
    float temperature_dht = dht.readTemperature();

    // Update LED status for MQ-4
    digitalWrite(MQ4_LED_GREEN, airQuality_MQ4 == "Good");
    digitalWrite(MQ4_LED_YELLOW, airQuality_MQ4 == "Moderate");
    digitalWrite(MQ4_LED_RED, airQuality_MQ4 == "Poor");

    // Update LED status for MQ-135
    digitalWrite(MQ135_LED_GREEN, airQuality_MQ135 == "Good");
    digitalWrite(MQ135_LED_YELLOW, airQuality_MQ135 == "Moderate");
    digitalWrite(MQ135_LED_RED, airQuality_MQ135 == "Poor");

    // Print data to Serial Monitor
    Serial.print("MQ-4 PPM: "); Serial.print(ppm_MQ4);
    Serial.print(" | Air Quality: "); Serial.println(airQuality_MQ4);
    Serial.print("MQ-135 PPM: "); Serial.print(ppm_MQ135);
    Serial.print(" | Air Quality: "); Serial.println(airQuality_MQ135);
    Serial.print("Temperature: "); Serial.print(temperature_dht);
    Serial.print(" C | Humidity: "); Serial.print(humidity);
    Serial.println(" %");

    // Measure AC current
    Voltage = getVPP();
    VRMS = (Voltage / 2.0) * 0.707;   //root 2 is 0.707
    AmpsRMS = ((VRMS * 1000) / mVperAmp) - 0.3; //0.3 is the error I got for my sensor
    Watt = (AmpsRMS * 240 / 1.2);

    delay(1000);
}

// Function to measure AC voltage
float getVPP() {
    float result;
    int readValue;                // value read from the sensor
    int maxValue = 0;             // store max value here
    int minValue = 4096;          // store min value here ESP32 ADC resolution

    uint32_t start_time = millis();
    while ((millis() - start_time) < 1000) //sample for 1 Sec
    {
        readValue = analogRead(sensorIn);
        if (readValue > maxValue) {
            maxValue = readValue;
        }
        if (readValue < minValue) {
            minValue = readValue;
        }
    }

    result = ((maxValue - minValue) * 3.3) / 4096.0;

    return result;
}
