#include <bluefruit.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// --- TFT Gizmo Setup ---
#define TFT_CS        0
#define TFT_RST       -1
#define TFT_DC        1
#define TFT_BACKLIGHT A3

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
BLEUart bleuart;

// --- Radar Data Structures ---
#define MAX_PLANES 10

struct Plane {
  String callsign;
  float dist;
  float bearing;
};

Plane planes[MAX_PLANES];
int planeCount = 0;
String buffer = "";

unsigned long lastPlaneTime = 0;
const unsigned long PLANE_BATCH_TIMEOUT = 2000; // 2 seconds without data triggers redraw

void setup() {
  Serial.begin(115200);

  // --- BLE UART ---
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("CPlay Bluefruit LE");
  bleuart.begin();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.start();

  // --- Display Init ---
  tft.init(240, 240);
  tft.setRotation(2);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);

  drawRadar();
}

void loop() {
  // --- Collect BLE UART messages ---
  while (bleuart.available()) {
    char c = bleuart.read();
    if (c == '\n') {
      handleMessage(buffer);
      buffer = "";
      lastPlaneTime = millis();  // Reset timer when new message comes in
    } else {
      buffer += c;
    }
  }

  // --- If no messages for 2 seconds, draw all planes and clear ---
  if (planeCount > 0 && millis() - lastPlaneTime > PLANE_BATCH_TIMEOUT) {
    drawRadar();
    planeCount = 0;
  }
}

void handleMessage(String msg) {
  if (planeCount >= MAX_PLANES) return;

  int comma1 = msg.indexOf(',');
  int comma2 = msg.indexOf(',', comma1 + 1);
  if (comma1 < 0 || comma2 < 0) return;

  Plane p;
  p.callsign = msg.substring(0, comma1);
  p.dist = msg.substring(comma1 + 1, comma2).toFloat();
  p.bearing = msg.substring(comma2 + 1).toFloat();
  planes[planeCount++] = p;
}

void drawRadar() {
  tft.fillScreen(ST77XX_BLACK);

  // Draw radar grid
  tft.drawCircle(120, 120, 100, ST77XX_GREEN);
  tft.drawCircle(120, 120, 70, ST77XX_GREEN);
  tft.drawCircle(120, 120, 40, ST77XX_GREEN);
  tft.drawCircle(120, 120, 10, ST77XX_GREEN);
  tft.drawFastHLine(20, 120, 200, ST77XX_GREEN);
  tft.drawFastVLine(120, 20, 200, ST77XX_GREEN);
  tft.fillCircle(120, 120, 3, ST77XX_GREEN);  // Your location

  // Plot each plane
  for (int i = 0; i < planeCount; i++) {
    plotPlaneWithLabel(planes[i]);
  }
}

void plotPlaneWithLabel(Plane p) {
  float angleRad = p.bearing * PI / 180.0;
  int r = p.dist * 100;
  int x = 120 + r * cos(angleRad);
  int y = 120 - r * sin(angleRad);

  tft.fillCircle(x, y, 3, ST77XX_RED);

  // Offset the label slightly to avoid overlap
  tft.setCursor(x + 4, y - 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print(p.callsign);
}
