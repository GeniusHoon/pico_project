#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include "ICM42622.h" 

// ================= LCD 하드웨어 핀 설정 =================
#define TFT_CS     13
#define TFT_RST    7 
#define TFT_DC     9 
#define TFT_MISO   12
#define TFT_MOSI   11
#define TFT_SCK    10

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, &SPI1, true);
Arduino_GFX *gfx = new Arduino_ST7735(bus, TFT_RST, 3, false, 80, 160, 24, 0, 24, 0);

// ================= ICM42622 센서 핀 설정 =================
#define I2C_PORT i2c0 
#define I2C_SDA_PIN 4 
#define I2C_SCL_PIN 5 

bool imuInit(void) {
  i2c_init(I2C_PORT, 400 * 1000); 
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); 
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  uint8_t DeviceID = ICM42622::Icm42622CheckID();
  if (DeviceID == ICM42622_DEVICE_ID) {
    if (!ICM42622::Icm42622Init()) return false; 
  }
  return true;
}

// ================= 수평계 UI 및 상태 변수 =================
const int center_x = 80;  // 화면 정중앙 X 좌표
const int center_y = 40;  // 화면 정중앙 Y 좌표
const int radius = 30;    // 수평계 원의 반지름

int bubble_x = center_x;
int bubble_y = center_y;
int prev_bubble_x = center_x;
int prev_bubble_y = center_y;

// ⭐ 이전 색상을 기억할 변수 선언 및 초기화
uint16_t lastColor = RGB565_GREEN; 

float Ax, Ay, Az; 

unsigned long lcdTimer = 0;  
const long interval = 50; 

bool TimeCheck(unsigned long &prevMillis, unsigned long delayTime) {
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis >= delayTime) {
    prevMillis = currentMillis; 
    return true;                
  }
  return false;                 
}

void setup(void) {
  Serial.begin(115200);

  // 1. LCD 초기화
  SPI1.setSCK(TFT_SCK);
  SPI1.setTX(TFT_MOSI);
  SPI1.setRX(TFT_MISO);
  SPI1.setCS(TFT_CS);
  gfx->begin();
  gfx->fillScreen(RGB565_BLACK);

  // 2. 센서 초기화 확인
  if (!imuInit()) {
    gfx->setTextColor(RGB565_RED);
    gfx->setCursor(10, 30);
    gfx->println("IMU Init Fail!");
    while(1); 
  }

  // 3. 수평계 배경 미리 그리기
  gfx->drawCircle(center_x, center_y, radius, RGB565_WHITE);
  gfx->drawLine(center_x - 35, center_y, center_x + 35, center_y, RGB565_DARKGRAY);
  gfx->drawLine(center_x, center_y - 35, center_x, center_y + 35, RGB565_DARKGRAY);
}

void loop() {
  if (ICM42622::Icm42622DataReady()) {
    ICM42622::Icm42622ReadAccel(&Ax, &Ay, &Az);
  }

  if (TimeCheck(lcdTimer, interval)) {
    
    // 1. 축 스왑 및 매핑
    int x_offset = map(Ay * 100, -100, 100, -radius, radius);
    int y_offset = map(Ax * 100, -100, 100, -radius, radius);

    x_offset = constrain(x_offset, -radius + 4, radius - 4);
    y_offset = constrain(y_offset, -radius + 4, radius - 4);

    // 2. 부호 교정 적용 (보드에 따라 +로 변경 가능)
    bubble_x = center_x - x_offset; 
    bubble_y = center_y + y_offset; 

    // 3. 수평 상태에 따른 색상 결정 (데드존 +-2)
    uint16_t bubbleColor = RGB565_GREEN;
    if (abs(x_offset) <= 2 && abs(y_offset) <= 2) {
      bubbleColor = RGB565_YELLOW;
    }

    // 4. 좌표가 바뀌었거나, 색상이 바뀌었을 때 화면 갱신
    if (bubble_x != prev_bubble_x || bubble_y != prev_bubble_y || bubbleColor != lastColor) {
      
      // [복구 로직 1] 이전 물방울 지우기
      gfx->fillCircle(prev_bubble_x, prev_bubble_y, 4, RGB565_BLACK);
      
      // [복구 로직 2] 지워진 배경 UI 다시 그려서 복구하기
      gfx->drawCircle(center_x, center_y, radius, RGB565_WHITE);
      gfx->drawLine(center_x - 35, center_y, center_x + 35, center_y, RGB565_DARKGRAY);
      gfx->drawLine(center_x, center_y - 35, center_x, center_y + 35, RGB565_DARKGRAY);

      // 새로운 위치에 결정된 색상으로 물방울 그리기
      gfx->fillCircle(bubble_x, bubble_y, 4, bubbleColor);

      // ⭐ 다음 턴을 위한 데이터 업데이트
      prev_bubble_x = bubble_x;
      prev_bubble_y = bubble_y;
      lastColor = bubbleColor; 
    }
  }
}
