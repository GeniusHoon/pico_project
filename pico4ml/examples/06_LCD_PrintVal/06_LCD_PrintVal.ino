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

// 센서 화면 갱신 타이머 (1초)
unsigned long sensorTimer = 0;  
const long interval = 1000;    

// 가상의 센서 값
int temp_value = 25;

bool TimeCheck(unsigned long &prevMillis, unsigned long delayTime) {
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis >= delayTime) {
    prevMillis = currentMillis; 
    return true;                
  }
  return false;                 
}

void setup(void) {
  SPI1.setSCK(TFT_SCK);
  SPI1.setTX(TFT_MOSI);
  SPI1.setRX(TFT_MISO);
  SPI1.setCS(TFT_CS);

  gfx->begin();
  gfx->fillScreen(RGB565_BLACK);

  // 변하지 않는 고정 텍스트 (UI 레이아웃) 미리 그리기
  gfx->setTextSize(2);
  gfx->setCursor(10, 20);
  gfx->setTextColor(RGB565_SKYBLUE);
  gfx->println("TEMP:");
}

void loop() {
  // 1초마다 센서 값 갱신 및 화면 출력
  if (TimeCheck(sensorTimer, interval)) {
    
    // 가상의 온도 변화 (20~30도 사이를 오르내림)
    temp_value = random(20, 31); 
    
    // [핵심] 글자색(WHITE)과 배경색(BLACK)을 동시에 지정하여 잔상 제거!
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK); 
    
    // 온도 값 출력 좌표 이동
    gfx->setCursor(80, 20); 
    
    // 숫자 뒤에 공백("  ")을 넣어, 자릿수가 줄어들 때(예: 100 -> 99) 남는 찌꺼기 방지
    gfx->print(temp_value);
    gfx->println(" C  "); 
  }
}
