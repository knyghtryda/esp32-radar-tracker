/*
 * Radar display for the LilyGO TTGO T-Display.
 * The view uses a forward-facing 180-degree field on a dark, blue-accented UI.
 */

#ifndef RADAR_DISPLAY_H
#define RADAR_DISPLAY_H

#include <TFT_eSPI.h>
#include "HLK_LD2450.h"

#define MAX_RANGE_M 10.0
#define LOG_NEAR_RANGE_M 0.5
#define RADAR_BACKGROUND_COLOR 0x0022

class RadarDisplay {
public:
  explicit RadarDisplay(TFT_eSPI *display) : _display(display) {}

  void begin() {}

  void drawRadarScreen() {
    // Center the semicircle just above the status strip. All arcs stop at
    // the baseline, so the display shows a 180-degree forward view only.
    static const float rangeMarkers[] = {0.5f, 1.0f, 2.0f, 5.0f, 10.0f};
    for (unsigned int i = 0; i < sizeof(rangeMarkers) / sizeof(rangeMarkers[0]); i++) {
      const int radius = radiusForDistance(rangeMarkers[i]);
      const uint16_t color = i == 4 ? GRID_BRIGHT : GRID_BLUE;
      drawArc(radius, color);
    }

    // Angle spokes for the LD2450's forward-facing field of view.
    const int spokeAngles[] = {-60, -30, 0, 30, 60};
    for (unsigned int i = 0; i < sizeof(spokeAngles) / sizeof(spokeAngles[0]); i++) {
      drawSpoke(spokeAngles[i], spokeAngles[i] == 0 ? GRID_BRIGHT : GRID_BLUE);
    }

    // The baseline and evenly spaced range ticks echo the reference image.
    _display->drawFastHLine(_centerX - _maxRadius, _baseY,
                            _maxRadius * 2 + 1, GRID_BRIGHT);
    drawRangeTicks();
    _display->fillCircle(_centerX, _baseY, 2, TFT_CYAN);
  }

  void drawRangeLabels() {
    static const float rangeMarkers[] = {0.5f, 1.0f, 2.0f, 5.0f, 10.0f};
    for (unsigned int i = 0; i < sizeof(rangeMarkers) / sizeof(rangeMarkers[0]); i++) {
      drawRangeLabel(rangeMarkers[i], radiusForDistance(rangeMarkers[i]));
    }
  }

  void drawPulse(float progress) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    const int pulseRadius = radiusForDistance(progress * MAX_RANGE_M);

    // The pulse is a semicircular wave that expands from the origin. Dimmer
    // arcs just inside it create a short radial wake without rotating a beam.
    for (int trail = 4; trail >= 1; trail--) {
      const int radius = pulseRadius - trail * 2;
      if (radius < 1) continue;
      const float opacity = 0.1f * (5 - trail);
      drawArc(radius, blendColor(TFT_CYAN, RADAR_BACKGROUND_COLOR, opacity));
    }

    drawArc(pulseRadius > 0 ? pulseRadius : 1, GRID_BRIGHT);
  }

  int pulseScreenRadius(float progress) const {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    return radiusForDistance(progress * MAX_RANGE_M);
  }

  int targetScreenRadius(float distance_m) const {
    return radiusForDistance(distance_m);
  }

  void plotTarget(float distance_m, float angle_deg, int targetIndex,
                  float brightness = 1.0f) {
    if (distance_m <= 0.0f || angle_deg < -90.0f || angle_deg > 90.0f) {
      return;
    }

    if (distance_m > MAX_RANGE_M) distance_m = MAX_RANGE_M;
    const float radius = radiusForDistance(distance_m);
    const float angleRad = angle_deg * PI / 180.0f;
    const int x = _centerX + static_cast<int>(radius * sin(angleRad));
    const int y = _baseY - static_cast<int>(radius * cos(angleRad));

    static const uint16_t targetColors[] = {TFT_RED, 0xFD20, TFT_YELLOW};
    const uint16_t color = blendColor(targetColors[targetIndex % 3],
                                      RADAR_BACKGROUND_COLOR, brightness);
    _display->drawCircle(x, y, 7,
                         blendColor(color, RADAR_BACKGROUND_COLOR, 0.4f));
    _display->fillCircle(x, y, 3, color);
    if (brightness > 0.7f) _display->drawPixel(x, y, TFT_WHITE);
  }

  void drawTargetInfo(Target targets[]) {
    _display->fillRect(0, _panelY, 240, 19, PANEL_BLUE);
    _display->setTextSize(1);
    static const uint16_t targetColors[] = {TFT_RED, 0xFD20, TFT_YELLOW};
    for (int i = 0; i < NUM_TARGETS; i++) {
      const int columnX = 5 + i * 80;
      _display->setTextColor(targetColors[i], PANEL_BLUE);
      _display->setCursor(columnX, _panelY + 6);
      _display->printf("T%d ", i + 1);
      if (targets[i].valid && targets[i].distance_m > 0.0f) {
        _display->printf("%.1fm", targets[i].distance_m);
      } else {
        _display->print(F("--.-m"));
      }
    }
  }

private:
  static const uint16_t GRID_BLUE = 0x021C;
  static const uint16_t GRID_BRIGHT = 0x04FF;
  static const uint16_t PANEL_BLUE = 0x03FF;

  TFT_eSPI *_display;
  const int _centerX = 120;
  const int _baseY = 111;
  const int _maxRadius = 108;
  const int _panelY = 116;

  void pointOnArc(int radius, int angle, int &x, int &y) const {
    const float angleRad = angle * PI / 180.0f;
    x = _centerX + static_cast<int>(radius * sin(angleRad));
    y = _baseY - static_cast<int>(radius * cos(angleRad));
  }

  int radiusForDistance(float distance) const {
    if (distance <= 0.0f) return 0;
    if (distance > MAX_RANGE_M) distance = MAX_RANGE_M;
    const float scale = logf(1.0f + MAX_RANGE_M / LOG_NEAR_RANGE_M);
    const float normalized = logf(1.0f + distance / LOG_NEAR_RANGE_M) / scale;
    return static_cast<int>(normalized * _maxRadius + 0.5f);
  }

  void drawRangeLabel(float distance, int radius) {
    int x, y;
    pointOnArc(radius, 15, x, y);
    const char *label = distance < 1.0f ? "0.5m" :
                        distance == 10.0f ? "10m" :
                        distance == 5.0f ? "5m" :
                        distance == 2.0f ? "2m" : "1m";
    const int labelX = x + 2;
    const int labelY = y - 4;
    _display->fillRect(labelX - 1, labelY - 1, 25, 10, RADAR_BACKGROUND_COLOR);
    _display->setTextSize(1);
    _display->setTextColor(GRID_BRIGHT, RADAR_BACKGROUND_COLOR);
    _display->setCursor(labelX, labelY);
    _display->print(label);
  }

  void drawArc(int radius, uint16_t color) {
    int previousX, previousY;
    pointOnArc(radius, -90, previousX, previousY);
    for (int angle = -88; angle <= 90; angle += 2) {
      int x, y;
      pointOnArc(radius, angle, x, y);
      _display->drawLine(previousX, previousY, x, y, color);
      previousX = x;
      previousY = y;
    }
  }

  void drawSpoke(int angle, uint16_t color) {
    int x, y;
    pointOnArc(_maxRadius, angle, x, y);
    _display->drawLine(_centerX, _baseY, x, y, color);
  }

  void drawRangeTicks() {
    for (int angle = -80; angle <= 80; angle += 10) {
      const int tickLength = (angle % 30 == 0) ? 6 : 3;
      int outerX, outerY, innerX, innerY;
      pointOnArc(_maxRadius, angle, outerX, outerY);
      pointOnArc(_maxRadius - tickLength, angle, innerX, innerY);
      _display->drawLine(innerX, innerY, outerX, outerY,
                         (angle % 30 == 0) ? TFT_CYAN : GRID_BRIGHT);
    }
  }

  uint16_t blendColor(uint16_t foreground, uint16_t background,
                      float opacity) const {
    if (opacity < 0.0f) opacity = 0.0f;
    if (opacity > 1.0f) opacity = 1.0f;

    const int bgR = (background >> 11) & 0x1F;
    const int bgG = (background >> 5) & 0x3F;
    const int bgB = background & 0x1F;
    const int fgR = (foreground >> 11) & 0x1F;
    const int fgG = (foreground >> 5) & 0x3F;
    const int fgB = foreground & 0x1F;
    const int r = bgR + static_cast<int>((fgR - bgR) * opacity);
    const int g = bgG + static_cast<int>((fgG - bgG) * opacity);
    const int b = bgB + static_cast<int>((fgB - bgB) * opacity);
    return static_cast<uint16_t>((r << 11) | (g << 5) | b);
  }
};

#endif // RADAR_DISPLAY_H
