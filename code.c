#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define RL_VALUE 10.0
#define R0_MQ4 9.83
#define R0_MQ135 9.83
#define SEA_LEVEL_PRESSURE 1013.25

float calculatePPM_MQ4(float Rs) {
    return 1021 * pow((Rs / R0_MQ4), -2.7887);
}

float calculatePPM_MQ135(float Rs) {
    return 116.602 * pow((Rs / R0_MQ135), -2.769);
}

const char* getAirQualityCategory(float ppm, float goodThreshold, float moderateThreshold) {
    if (ppm < goodThreshold) {
        return "Good";
    } else if (ppm < moderateThreshold) {
        return "Moderate";
    } else {
        return "Poor";
    }
}

float getVoltage(int analogValue, float maxVoltage, int resolution) {
    return (analogValue / (float)resolution) * maxVoltage;
}

float getRs(float voltage, float RL) {
    return RL * ((5.0 / voltage) - 1);
}

float getVPP(int *values, int size, float maxVoltage, int resolution) {
    int maxValue = 0;
    int minValue = resolution;
    for (int i = 0; i < size; i++) {
        if (values[i] > maxValue) maxValue = values[i];
        if (values[i] < minValue) minValue = values[i];
    }
    return ((maxValue - minValue) * maxVoltage) / resolution;
}

int main() {
    srand(time(0));

    int analogValue_MQ4 = rand() % 4096;
    int analogValue_MQ135 = rand() % 4096;
    int currentSamples[1000];

    for (int i = 0; i < 1000; i++) {
        currentSamples[i] = rand() % 4096;
    }

    float voltage_MQ4 = getVoltage(analogValue_MQ4, 5.0, 4096);
    float Rs_MQ4 = getRs(voltage_MQ4, RL_VALUE);
    float ppm_MQ4 = calculatePPM_MQ4(Rs_MQ4);
    const char* airQuality_MQ4 = getAirQualityCategory(ppm_MQ4, 300, 600);

    float voltage_MQ135 = getVoltage(analogValue_MQ135, 5.0, 4096);
    float Rs_MQ135 = getRs(voltage_MQ135, RL_VALUE);
    float ppm_MQ135 = calculatePPM_MQ135(Rs_MQ135);
    const char* airQuality_MQ135 = getAirQualityCategory(ppm_MQ135, 400, 1000);

    float Voltage = getVPP(currentSamples, 1000, 3.3, 4096);
    float VRMS = (Voltage / 2.0) * 0.707;
    float AmpsRMS = ((VRMS * 1000) / 185.0) - 0.3;
    float Watt = (AmpsRMS * 240 / 1.2);

    printf("MQ-4 PPM: %.2f | Air Quality: %s\n", ppm_MQ4, airQuality_MQ4);
    printf("MQ-135 PPM: %.2f | Air Quality: %s\n", ppm_MQ135, airQuality_MQ135);
    printf("Voltage: %.2fV | VRMS: %.2fV | Amps RMS: %.2fA | Watt: %.2fW\n", Voltage, VRMS, AmpsRMS, Watt);

    return 0;
}
