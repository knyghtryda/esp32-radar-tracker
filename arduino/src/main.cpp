/*
 * LilyGO TTGO T-Display HLK-LD2450 Radar Tracker
 *
 * A radar motion tracker inspired by the Alien Motion Tracker
 * Features a 180-degree radar view with a logarithmic 10 m range and
 * simultaneous display of up to three targets
 *
 * Hardware:
 * - LilyGO TTGO T-Display (original ESP32, ST7789V 240x135 color LCD)
 * - HLK-LD2450 24GHz Radar Sensor
 * - Onboard ST7789V TFT LCD
 *
 * Connections:
 * Radar: TX->GPIO26, RX->GPIO27, VCC->5V, GND->GND
 * (UART pins are selected to avoid the TFT's GPIO18 SPI clock.)
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "HLK_LD2450.h"
#include "RadarDisplay.h"

// UART pins for HLK-LD2450
#define RADAR_RX_PIN 26
#define RADAR_TX_PIN 27

// Create objects
TFT_eSPI display;
TFT_eSprite radarFrame(&display);
HLK_LD2450 radar(Serial1);
RadarDisplay radarDisplay(&radarFrame);
Target pulseTargets[NUM_TARGETS] = {};
Target visibleTargets[NUM_TARGETS] = {};
bool targetHitThisPulse[NUM_TARGETS] = {};
bool visibleTarget[NUM_TARGETS] = {};
unsigned long targetHitAt[NUM_TARGETS] = {};

unsigned long lastUpdate = 0;
unsigned long lastDiagnostics = 0;
unsigned long displayedPulseCycle = 0xFFFFFFFFUL;
const unsigned long UPDATE_INTERVAL = 50; // Update every 50ms (20 FPS)
const unsigned long DIAGNOSTICS_INTERVAL = 1000;
const unsigned long PULSE_INTERVAL = 1000;

void setup() {
  // Initialize USB Serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("LilyGO T-Display Radar Tracker Starting...");

  // Initialize the onboard ST7789V TFT and its backlight.
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);
  display.setTextSize(1);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setCursor(10, 20);
  display.println(F("Radar Tracker"));
  display.setCursor(10, 35);
  display.println(F("Initializing..."));

  // Render each frame off-screen, then transfer it as a single image to
  // avoid the visible flash caused by clearing and redrawing the LCD in place.
  radarFrame.setColorDepth(16);
  if (radarFrame.createSprite(240, 135) == nullptr) {
    Serial.println("Could not allocate radar frame buffer");
    while (true) {
      delay(1000);
    }
  }
  delay(1000);

  // Initialize UART for HLK-LD2450
  Serial1.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  Serial.println("UART initialized at 256000 baud");

  // Initialize radar display
  radarDisplay.begin();

  Serial.println("Setup complete!");
  delay(500);
}

void loop() {
  unsigned long currentMillis = millis();

  // Read radar data
  radar.update();

  // Latch the latest sensor frame once at the start of each expanding pulse.
  const unsigned long pulseCycle = currentMillis / PULSE_INTERVAL;
  if (pulseCycle != displayedPulseCycle) {
    displayedPulseCycle = pulseCycle;
    for (int i = 0; i < NUM_TARGETS; i++) {
      pulseTargets[i] = radar.targets[i];
      targetHitThisPulse[i] = false;
    }
  }

  const float pulseProgress =
    (currentMillis % PULSE_INTERVAL) / static_cast<float>(PULSE_INTERVAL);
  const int currentPulseRadius = radarDisplay.pulseScreenRadius(pulseProgress);

  // Latch each target at the moment the expanding wave reaches its range.
  for (int i = 0; i < NUM_TARGETS; i++) {
    if (pulseTargets[i].valid && pulseTargets[i].distance_m > 0.0f &&
        !targetHitThisPulse[i] &&
        currentPulseRadius >= radarDisplay.targetScreenRadius(
                                pulseTargets[i].distance_m)) {
      visibleTargets[i] = pulseTargets[i];
      visibleTarget[i] = true;
      targetHitAt[i] = currentMillis;
      targetHitThisPulse[i] = true;
    }
  }

  // Update display at regular intervals
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = currentMillis;

    // Clear display
    radarFrame.fillSprite(RADAR_BACKGROUND_COLOR);

    // Draw radar screen
    radarDisplay.drawRadarScreen();

    // Draw the expanding semicircular pulse and its once-per-cycle snapshot.
    radarDisplay.drawPulse(pulseProgress);
    radarDisplay.drawRangeLabels();
    for (int i = 0; i < NUM_TARGETS; i++) {
      if (visibleTarget[i]) {
        const unsigned long markerAge = currentMillis - targetHitAt[i];
        if (markerAge >= PULSE_INTERVAL) {
          visibleTarget[i] = false;
        } else {
          const float brightness = 1.0f -
            markerAge / static_cast<float>(PULSE_INTERVAL);
          radarDisplay.plotTarget(visibleTargets[i].distance_m,
                                  visibleTargets[i].angle_deg, i, brightness);
        }
      }
    }

    // Range readouts stay in fixed columns and never fade.
    radarDisplay.drawTargetInfo(pulseTargets);

    // Transfer the completed frame once, rather than drawing on the LCD
    // while it is being cleared and reconstructed.
    radarFrame.pushSprite(0, 0);

  }

  if (currentMillis - lastDiagnostics >= DIAGNOSTICS_INTERVAL) {
    lastDiagnostics = currentMillis;
    Serial.printf("Radar RX: %lu bytes, %lu valid frames, %lu bad frames | ",
                  static_cast<unsigned long>(radar.bytesReceived()),
                  static_cast<unsigned long>(radar.validFrames()),
                  static_cast<unsigned long>(radar.invalidFrames()));
    for (int i = 0; i < 3; i++) {
      if (radar.targets[i].valid) {
        Serial.printf("T%d x=%d y=%d d=%.2fm a=%.1fdeg | ", i + 1,
                      radar.targets[i].x_mm, radar.targets[i].y_mm,
                      radar.targets[i].distance_m, radar.targets[i].angle_deg);
      } else {
        Serial.printf("T%d empty | ", i + 1);
      }
    }
    Serial.println();
  }

  // Small delay to prevent overwhelming the CPU
  delay(5);
}
