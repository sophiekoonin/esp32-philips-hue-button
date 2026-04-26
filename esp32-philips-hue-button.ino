/**
 * See README.md for details
 *
 * Modify Config.h for your Wifi and Hue setup
 */
#include "Config.h"
#include <Arduino.h>
#include "PinButton.h"
#include "HueClient.h"
#include "HueGroup.h"
#include <FastLED.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#define USE_SERIAL Serial
#define NUM_LEDS 18
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

bool buttonHeld = false;
unsigned long scrollDelayCount = 0;
unsigned long ledDelayCount = 0;
int sceneIndex = 0;
bool longClick = false;
bool ledOn = false;
PinButton button(BUTTON_PIN);

HueClient conn(HUE_IP, HUE_API_USERNAME);
HueGroup group(conn, HUE_GROUP);

void setup() {
  USE_SERIAL.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Connect to wifi
  WiFiMulti wifiMulti;
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
  }
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  if ((wifiMulti.run() != WL_CONNECTED)) {
    USE_SERIAL.println("Unable to connect");
  }
}

void loop() {

  button.update();
  if (button.isClick()) {
    fill_solid(leds + 5, NUM_LEDS, CRGB::White);
    FastLED.show();
    ledOn = true;
  }

  if (button.isSingleClick()) {
    group.toggle();
    USE_SERIAL.printf("Single click");
  }
  if (button.isDoubleClick()) {
    USE_SERIAL.printf("Double click");
    if (sceneIndex >= (sizeof scenes / sizeof scenes[0])) {
      sceneIndex = 0;
    }

    group.setScene(scenes[sceneIndex]);

    sceneIndex++;
  }

  if (button.isLongClick()) {
    USE_SERIAL.printf("Long click");
    scrollDelayCount = 0;
    group.scrollBrightnessStart();
    buttonHeld = true;
    longClick = true;
  }

  if (button.isReleased()) {
    USE_SERIAL.printf("release");
    buttonHeld = false;
    if (longClick) {
      FastLED.clear();
      FastLED.show();
      ledOn = false;
      ledDelayCount = 0;
      longClick = false;
    }
  }

  if (buttonHeld) {
    scrollDelayCount++;
    if (scrollDelayCount > 200000) {
      group.scrollBrightnessTick();
      scrollDelayCount = 0;
    }
  }

  // Keeps LED on for a short amount of time if button is pressed, but not long pressed
  if (ledOn && !longClick) {
    ledDelayCount++;
    if (ledDelayCount > 1500000) {
      FastLED.clear();
      FastLED.show();
      ledOn = false;
      ledDelayCount = 0;
      longClick = false;
    }
  }
}
