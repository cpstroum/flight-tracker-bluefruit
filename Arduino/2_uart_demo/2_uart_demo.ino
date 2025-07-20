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
  bleuart.begin();  // 👈 init instance
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

void displayMessage(String msg) {
  static int y = 40;
  if (y > 200) {
    tft.fillRect(0, 40, 240, 200, ST77XX_BLACK);
    y = 40;
  }
  tft.setCursor(10, y);
  tft.print(msg);
  y += 20;
}
