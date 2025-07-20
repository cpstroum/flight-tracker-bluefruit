#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS        0
#define TFT_RST       -1
#define TFT_DC        1
#define TFT_BACKLIGHT PIN_A3

#if (SPI_INTERFACES_COUNT == 1)
  SPIClass* spi = &SPI;
#else
  SPIClass* spi = &SPI1;
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(spi, TFT_CS, TFT_DC, TFT_RST);

#define CENTER_X 120
#define CENTER_Y 120

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Starting Radar Display..."));

  tft.init(240, 240);
  tft.setRotation(2);

  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  tft.fillScreen(ST77XX_BLACK);
  drawRadarGrid();
}

void loop(void) {
  // Future: animations go here
}

void drawRadarGrid() {
  // Concentric circles
  tft.drawCircle(CENTER_X, CENTER_Y, 40, ST77XX_GREEN);
  tft.drawCircle(CENTER_X, CENTER_Y, 80, ST77XX_GREEN);
  tft.drawCircle(CENTER_X, CENTER_Y, 120, ST77XX_GREEN);

  // Crosshairs
  tft.drawLine(CENTER_X, 0, CENTER_X, 240, ST77XX_GREEN);
  tft.drawLine(0, CENTER_Y, 240, CENTER_Y, ST77XX_GREEN);

  // HOME marker
  tft.fillCircle(CENTER_X, CENTER_Y, 3, ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(CENTER_X - 10, CENTER_Y + 10);
  tft.print("HOME");
}
