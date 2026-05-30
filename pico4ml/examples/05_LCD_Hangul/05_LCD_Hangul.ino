#include <Arduino_GFX_Library.h>
#include <SPI.h>

// [주의] Arduino_GFX.h에 정의된 U8g2 폰트를 사용하기 위해 U8g2lib 헤더가 필요합니다.
#include <U8g2lib.h> 

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
  gfx->fillScreen(RGB565_BLACK);

  // ======================================================
  // ⭐ 한글 출력을 위한 핵심 설정 2가지
  // ======================================================
  
  // 1. UTF-8 인코딩 활성화 (한글 깨짐 방지)
  gfx->setUTF8Print(true); 

  // 2. 한글(CJK)이 지원되는 U8g2 폰트로 붓 교체
  // (u8g2_font_unifont_t_korean1 외에도 헤더에 정의된 여러 폰트 사용 가능)
  gfx->setFont(u8g2_font_unifont_h_cjk);
  
  // ======================================================

  // 텍스트 출력하기
  gfx->setCursor(10, 30);             
  gfx->setTextColor(RGB565_YELLOW);   // 글자색을 노란색으로
  
  // 기존 영어 폰트와 달리 한글 폰트는 기본 크기 자체가 크므로, 
  // setTextSize는 보통 지정하지 않거나 1로 둡니다.
  
  // 한글 출력!
  gfx->println("안녕하세요!");         
  
  gfx->setCursor(10, 60);
  gfx->setTextColor(RGB565_WHITE);
  gfx->println("인하대학교");
}

void loop() {
}