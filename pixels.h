#include <Adafruit_NeoPixel.h>

void setPixels(Adafruit_NeoPixel* pixels, int pixelBegin, int pixelEnd, int r, int g, int b) {
  for (int i = pixelBegin; i < pixelEnd; ++i) {
    pixels->setPixelColor(i, pixels->Color(r, g, b));
  }
}

float bound(float val, float min, float max) {
  if (val < min) {
    return min;
  } 
  if (val > max) { 
    return max;
  }

  return val;
}

void updatePixelsGradient(Adafruit_NeoPixel* pixels, int pixelBegin, int pixelEnd, float currentVal, float optimalVal, float maxVal, float minVal) {
  const float diff = (currentVal - optimalVal);

  const float blueDist = optimalVal - minVal; 
  const float redDist = maxVal - optimalVal;
  
  const int r = diff >= 0 ? bound((abs(diff) / redDist) * 255.0, 0, 255) : 0;
  const int g = diff >= 0 ? 255 - bound((abs(diff) / redDist) * 255.0, 0, 255) : 255 - bound((abs(diff) / blueDist) * 255.0, 0, 255);
  const int b = diff >= 0 ? 0 : bound((abs(diff) / blueDist) * 255.0, 0, 255);

  setPixels(pixels, pixelBegin, pixelEnd, r, g, b);
}

void updateTemperaturePixels(Adafruit_NeoPixel* pixels, float currentTemperature) {
  const float minTemperature = 5;
  const float optimalTemperature = 23;
  const float maxTemperature = 35;

  updatePixelsGradient(pixels, 0, 8, currentTemperature, optimalTemperature, maxTemperature, minTemperature);
}

void updatePhPixels(Adafruit_NeoPixel* pixels, float currentPh) {
  const float minPh = 4;
  const float optimalPh = 5.8;
  const float maxPh = 8.5;
  
  updatePixelsGradient(pixels, 8, 16, currentPh, optimalPh, maxPh, minPh);
}
