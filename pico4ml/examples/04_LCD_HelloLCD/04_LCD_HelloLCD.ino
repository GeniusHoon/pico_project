#include <Arduino_GFX_Library.h>
#include <SPI.h>

#define TFT_CS     13
#define TFT_RST    7 
#define TFT_DC     9 
#define TFT_MISO   12
#define TFT_MOSI   11
#define TFT_SCK    10

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, &SPI1, true);
Arduino_GFX *gfx = new Arduino_ST7735(bus, TFT_RST, 3, false, 80, 160, 24, 0, 24, 0);

void setup(void) {
  SPI1.setSCK(TFT_SCK);
  SPI1.setTX(TFT_MOSI);
  SPI1.setRX(TFT_MISO);
  SPI1.setCS(TFT_CS);

  gfx->begin();

  // 1. 배경을 가장 어두운 회색으로 칠하기 (내장 매크로 사용)
  gfx->fillScreen(RGB565_BLACK);

  // 2. 사각형 그리기 (파란색 계열 중 하늘색 사용)
  gfx->drawRect(10, 10, 40, 20, RGB565_SKYBLUE);

  // 3. 색이 채워진 원 그리기 (토마토 색상)
  gfx->fillCircle(120, 40, 15, RGB565_TOMATO);

  // 4. 대각선 긋기 (금색)
  gfx->drawLine(0, 0, 160, 80, RGB565_GOLD);

  // 5. 텍스트 출력하기
  gfx->setCursor(10, 50);             
  gfx->setTextColor(RGB565_WHITE);    
  gfx->setTextSize(2);                
  gfx->println("Hello LCD!");         
}

void loop() {
  // 이번 예제는 한 번만 그리면 되므로 loop는 비워둡니다.
}
