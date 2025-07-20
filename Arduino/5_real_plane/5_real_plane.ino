#include <bluefruit.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS        0
#define TFT_RST       -1
#define TFT_DC        1
#define TFT_BACKLIGHT A3

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
BLEUart bleuart;

#define MAX_PLANES 10

struct Plane {
  String callsign;
  float dist;
  float bearing;
  String source;
  String destination;
};

Plane planes[MAX_PLANES];
int planeCount = 0;
String buffer = "";
unsigned long lastPlaneTime = 0;
const unsigned long PLANE_BATCH_TIMEOUT = 2000;

void setup() {
  Serial.begin(115200);
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("CPlay Bluefruit LE");
  bleuart.begin();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.start();

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
  while (bleuart.available()) {
    char c = bleuart.read();
    if (c == '\n') {
      handleMessage(buffer);
      buffer = "";
      lastPlaneTime = millis();
    } else {
      buffer += c;
    }
  }

  if (planeCount > 0 && millis() - lastPlaneTime > PLANE_BATCH_TIMEOUT) {
    drawRadar();
    planeCount = 0;
  }
}

void handleMessage(String msg) {
  if (planeCount >= MAX_PLANES) return;

  int c1 = msg.indexOf(',');
  int c2 = msg.indexOf(',', c1 + 1);
  int c3 = msg.indexOf(',', c2 + 1);
  int c4 = msg.indexOf(',', c3 + 1);
  if (c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0) return;

  Plane p;
  p.callsign = msg.substring(0, c1);
  p.dist = msg.substring(c1 + 1, c2).toFloat();
  p.bearing = msg.substring(c2 + 1, c3).toFloat();
  p.source = msg.substring(c3 + 1, c4);
  p.destination = msg.substring(c4 + 1);
  if (p.source.length() == 0) p.source = "?";
  if (p.destination.length() == 0) p.destination = "?";

  planes[planeCount++] = p;
}

void drawRadar() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawCircle(120, 120, 100, ST77XX_GREEN);
  tft.drawCircle(120, 120, 70, ST77XX_GREEN);
  tft.drawCircle(120, 120, 40, ST77XX_GREEN);
  tft.drawCircle(120, 120, 10, ST77XX_GREEN);
  tft.drawFastHLine(20, 120, 200, ST77XX_GREEN);
  tft.drawFastVLine(120, 20, 200, ST77XX_GREEN);
  tft.fillCircle(120, 120, 3, ST77XX_GREEN);

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
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(x + 4, y - 8);
  tft.print(p.callsign);
  tft.setCursor(x + 4, y + 2);
  tft.print(p.source + " → " + p.destination);
}
