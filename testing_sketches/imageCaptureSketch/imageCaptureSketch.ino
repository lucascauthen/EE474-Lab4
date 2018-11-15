#include "optfft.h"
#include "tables.c"
#define IMAGE_CAPTURE_SAMPLES 256
#define IMAGE_CAPTURE_PIN 12

signed int* samples;
signed int* zeros;

#define SAMPLING_FREQUENCY  7500.0f
#define SAMPLING_DELAY  (1.0f/SAMPLING_FREQUENCY)*1000000
int sampleIndex = 0;



void setup() {
  // put your setup code here, to run once:
  samples = malloc (IMAGE_CAPTURE_SAMPLES * sizeof *samples);
  zeros = malloc (IMAGE_CAPTURE_SAMPLES * sizeof *samples);
  Serial.begin(9600);
  Serial.print("Start!: ");
  Serial.println(sampleIndex);
}

long systemTime() {
  return micros();
}

long nextRunTime = 0;
void recordMeasurement() {
  if(nextRunTime == 0 || systemTime() >= nextRunTime) {
    nextRunTime = systemTime() + SAMPLING_DELAY;
    //Check if we need to record smaples
    if(sampleIndex < IMAGE_CAPTURE_SAMPLES) {
      samples[sampleIndex] = analogRead(IMAGE_CAPTURE_PIN);
      zeros[sampleIndex] = 0;
      sampleIndex++;
    } else {
      Serial.print("Running Operation: ");
      signed int maxFrequency = optfft(u2, zeros);
      Serial.print("Max: ");
      Serial.println(samples[maxFrequency]);
      nextRunTime = 0;
      sampleIndex = 0;
    }
  }
}

void loop() {
  recordMeasurement();
}
