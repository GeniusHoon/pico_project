#include <Arduino_GFX_Library.h>
#include <SPI.h>

// 1. 변환된 이미지 데이터 배열이 담긴 파일을 불러옵니다.
// #include "inhalogo.c"
#include "lcds___displays_minibot.c"

// LCD 하드웨어 핀 설정
#define TFT_CS     13
#define TFT_RST    7
#define TFT_DC     9
#define TFT_MISO   12
#define TFT_MOSI   11
#define TFT_SCK    10

// LCD 객체 생성 및 화면 방향(Rotation: 3, 가로 모드) 설정
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, &SPI1, true); 
Arduino_GFX *gfx = new Arduino_ST7735( bus, 7 /* RST */, 3 /* rotation */, false /* IPS */,
    80 /* width */, 160 /* height */,
    24 /* col offset 1 */, 0 /* row offset 1 */,
    24 /* col offset 2 */, 0 /* row offset 2 */);

void setup() {
  // SPI 통신 핀 맵핑
  SPI1.setSCK(TFT_SCK);
  SPI1.setTX(TFT_MOSI);
  SPI1.setRX(TFT_MISO);
  SPI1.setCS(TFT_CS);

  // LCD 디스플레이 깨우기
  gfx->begin();
  
  // ----------------------------------------------------------------------------
  // ⭐ [핵심 실습] 커스텀 이미지 배열을 화면에 출력하기
  // ----------------------------------------------------------------------------
  // 사용법: draw16bitRGBBitmap(시작 X좌표, 시작 Y좌표, 배열 포인터, 그림 폭, 그림 높이)
  
  // 인하대학교 로고 출력
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)lcds___displays_minibot_map, 160, 80);   
   
}

void loop() {
  // 이미지는 한 번만 그리면 되므로 loop는 비워둡니다.
}