#include <bluefruit.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// --- TFT Setup ---
#define TFT_CS        0
#define TFT_RST       -1
#define TFT_DC        1
#define TFT_BACKLIGHT A3

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
BLEUart bleuart;  

String buffer = "";

void setup() {
  Serial.begin(115200);
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("CPlay Bluefruit LE");

  // UART Setup
  bleuart.begin();  
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.start();

  // TFT Init
  tft.init(240, 240);
  tft.setRotation(2);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.println("Waiting for planes...");
}

void loop() {
  while (bleuart.available()) {
    char c = bleuart.read();
    if (c == '\n' || buffer.length() > 100) {
      displayMessage(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }
}

void drawRadarBackground() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawCircle(120, 120, 100, ST77XX_GREEN);
  tft.drawCircle(120, 120, 70, ST77XX_GREEN);
  tft.drawCircle(120, 120, 40, ST77XX_GREEN);
  tft.drawCircle(120, 120, 10, ST77XX_GREEN);
  tft.drawFastHLine(20, 120, 200, ST77XX_GREEN);
  tft.drawFastVLine(120, 20, 200, ST77XX_GREEN);
  tft.fillCircle(120, 120, 3, ST77XX_GREEN);  // your location
}

void plotPlane(float distanceRatio, float angleDegrees) {
  float angleRad = angleDegrees * PI / 180.0;
  int r = distanceRatio * 100;  // scale to 100px max
  int x = 120 + r * cos(angleRad);
  int y = 120 - r * sin(angleRad);  // y is inverted on screen

  tft.fillCircle(x, y, 3, ST77XX_RED);
}

void displayMessage(String msg) {
  drawRadarBackground();

  // Mock data: cycle through fake planes
  plotPlane(0.2, 45);   // 20% away, 45°
  plotPlane(0.5, 135);  // 50% away, 135°
  plotPlane(0.8, 270);  // 80% away, due south

  tft.setCursor(10, 220);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.print(msg);  // still print last plane string for now
}

