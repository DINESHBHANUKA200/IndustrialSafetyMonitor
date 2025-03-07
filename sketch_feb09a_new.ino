#include <Wire.h>
#include <Adafruit_GFX.h>  //draw shapes,image in display
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define sensor pins
#define MQ4_PIN 34        
#define MQ135_PIN 35      
#define DHT_PIN 32        
#define DHT_TYPE DHT22    

// Define LED pins for MQ-4
#define MQ4_LED_GREEN 2   
#define MQ4_LED_YELLOW 4  
#define MQ4_LED_RED 16    

// Define LED pins for MQ-135
#define MQ135_LED_GREEN 5   
#define MQ135_LED_YELLOW 18 
#define MQ135_LED_RED 19    

#define RL_VALUE 10.0     
#define R0_MQ4 9.83       
#define R0_MQ135 9.83     

DHT dht(DHT_PIN, DHT_TYPE);

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
// initialize DHT Sensor
void setup() {
    Serial.begin(115200);
    dht.begin();

    //OLED Display initialization
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {//check connection and clear display
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();

    // Set LED pins as output
    pinMode(MQ4_LED_GREEN, OUTPUT);
    pinMode(MQ4_LED_YELLOW, OUTPUT);
    pinMode(MQ4_LED_RED, OUTPUT);
    pinMode(MQ135_LED_GREEN, OUTPUT);
    pinMode(MQ135_LED_YELLOW, OUTPUT);
    pinMode(MQ135_LED_RED, OUTPUT);
}

//after setup loop will run
void loop() {
    // Read and calculate MQ-4 values
    int analogValue_MQ4 = analogRead(MQ4_PIN);
    float voltage_MQ4 = (analogValue_MQ4 / 4095.0) * 5.0;
    float Rs_MQ4 = RL_VALUE * ((5.0 / voltage_MQ4) - 1);  //conver to voltage
    float ppm_MQ4 = calculatePPM_MQ4(Rs_MQ4);   //get ppm concentraion
    String airQuality_MQ4 = getAirQualityCategory(ppm_MQ4, 300, 600);

    // Read and calculate MQ-135 values
    int analogValue_MQ135 = analogRead(MQ135_PIN);
    float voltage_MQ135 = (analogValue_MQ135 / 4095.0) * 5.0;
    float Rs_MQ135 = RL_VALUE * ((5.0 / voltage_MQ135) - 1);
    float ppm_MQ135 = calculatePPM_MQ135(Rs_MQ135);
    String airQuality_MQ135 = getAirQualityCategory(ppm_MQ135, 400, 1000);

    // Read temperature and humidity from DHT sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Update LED status for MQ-4
    digitalWrite(MQ4_LED_GREEN, airQuality_MQ4 == "Good");
    digitalWrite(MQ4_LED_YELLOW, airQuality_MQ4 == "Moderate");
    digitalWrite(MQ4_LED_RED, airQuality_MQ4 == "Poor");

    // Update LED status for MQ-135
    digitalWrite(MQ135_LED_GREEN, airQuality_MQ135 == "Good");
    digitalWrite(MQ135_LED_YELLOW, airQuality_MQ135 == "Moderate");
    digitalWrite(MQ135_LED_RED, airQuality_MQ135 == "Poor");

    // Display data on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("MQ-4: "); display.println(airQuality_MQ4);
    display.print("MQ-135: "); display.println(airQuality_MQ135);
    display.print("Temp: "); display.print(temperature); display.println(" C");
    display.print("Humidity: "); display.print(humidity); display.println(" %");
    display.display();

    // Print data to Serial Monitor
    Serial.print("MQ-4 PPM: "); Serial.print(ppm_MQ4);
    Serial.print(" | Air Quality: "); Serial.println(airQuality_MQ4);
    Serial.print("MQ-135 PPM: "); Serial.print(ppm_MQ135);
    Serial.print(" | Air Quality: "); Serial.println(airQuality_MQ135);
    Serial.print("Temperature: "); Serial.print(temperature);
    Serial.print(" C | Humidity: "); Serial.print(humidity);
    Serial.println(" %");

    delay(1000);
}
