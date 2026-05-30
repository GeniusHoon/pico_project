#include <smart_health_care_inferencing.h>
#include <ICM42622.h> // Pico4ML 내장 IMU 라이브러리
#include "LCD_st7735.h" // [변경] Pico4ML 내장 디스플레이 전용 드라이버

// magicstick 예제와 face detection의 디스플레이 코드를 참조하였습니다.

/* 하드웨어 설정 (Pico4ML I2C 스펙) */
#define I2C_PORT        i2c0 
#define I2C_SDA_PIN     4
#define I2C_SCL_PIN     5

/* 드라이버용 색상 정의 (RGB565 포맷) */
#define COLOR_GREEN     0x07E0 // UP 텍스트에 사용할 녹색
#define COLOR_BLUE      0x001F // 일반 상태용 청색
#define COLOR_WHITE     0xFFFF // CNT 텍스트용 흰색
#define COLOR_BLACK     0x0000 // 배경색 (글자 잔상 제거용)

/* Edge Impulse 라이브러리 연동 버퍼 및 전역 변수 */
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE]; // 1초치 센서 데이터 저장 버퍼
static bool debug_nn = false;                       // NN 상세 로그 활성화 여부
static uint32_t cnt = 0;                            // UPDN(상하) 누적 카운터
static uint32_t last_cnt = 0xFFFFFFFF; // CNT 업데이트 감지용 이전 카운트 저장

/* 하드웨어 초기화 함수 프로토타입 선언 */
bool imuInit(void);

void setup() {
    Serial.begin(115200);
    
    // PC 시리얼 모니터 대기 (최대 3초)
    while (!Serial && millis() < 3000); 

    // [Step 1] 전용 드라이버를 이용한 LCD 디스플레이 초기화
    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK); // 화면 전체를 검은색으로 지움

    // IMU 센서 초기화
    if (!imuInit()) { 
        Serial.println("[-] IMU 초기화 실패!");
        // 화면에 에러 문구 표시
        ST7735_WriteString(5, 30, "IMU INIT FAIL!", Font_11x18, 0xF800, COLOR_BLACK);
        while (1); 
    }
    
    Serial.println("[+] Edge Impulse 실시간 추론 준비 완료.");
    
    // 기본 대기 화면 레이아웃 그리기 (초기 1회)
    ST7735_WriteString(5, 20, "STATUS: ---", Font_11x18, COLOR_BLUE, COLOR_BLACK);
}

void loop() {
    Serial.println("\n>>> 동작 데이터를 수집 중...");

    /* 1. 윈도우 크기만큼 raw 데이터 수집 (100Hz 샘플링) */
    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
        float Ax, Ay, Az;
        
        while (!ICM42622::Icm42622DataReady()) {
            // 데이터 대기
        }
        
        ICM42622::Icm42622ReadAccel(&Ax, &Ay, &Az);
        features[ix + 0] = Ax;
        features[ix + 1] = Ay;
        features[ix + 2] = Az;

        while (micros() < next_tick); 
    }

    /* 2. 데이터를 signal_t 객체로 포장 */
    signal_t signal;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) return;

    /* 3. 온디바이스 인공지능 추론 엔진 가동 */
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, debug_nn);
    if (res != EI_IMPULSE_OK) return;

    /* 4. 추론 결과 스코어 추출 */
    float idle_score      = result.classification[0].value;
    float leftright_score = result.classification[1].value;
    float shake_score     = result.classification[2].value;
    float updn_score      = result.classification[3].value;

    // [상단 영역 디스플레이 제어: UPDOWN 감지 시에만 대형 UP 출력]
    if (updn_score > 0.50) {
        cnt++; 
        // 큰 폰트(Font_16x26)를 사용하여 화면 중앙 상단에 "UP" 출력
        // 글자 뒤에 공백("  ")을 두어 이전 "STATUS: ---" 등의 긴 문자열 잔상을 깨끗하게 지웁니다.
        ST7735_WriteString(35, 15, "UP      ", Font_16x26, COLOR_GREEN, COLOR_BLACK);
    } 
    else {
        // UPDOWN이 아닐 때는 일반 폰트(Font_11x18)로 현재 상태 표시
        // 마찬가지로 뒤에 공백을 패딩하여 이전 "UP" 글자가 남긴 잔상을 지웁니다.

        // 총 학습 데이터는 5분 정도 사이즈입니다.
        // 하나의 state에 0.8이상 나오는 경우가 많지 않아서 테스트코드로 적합한 값을 찾아 튜닝했습니다.
        if (leftright_score > 0.40) {
            ST7735_WriteString(5, 20, "STATUS: L/R  ", Font_11x18, COLOR_BLUE, COLOR_BLACK);
        }
        else if (shake_score > 0.45) { 
            cnt = 0; 
            ST7735_WriteString(5, 20, "STATUS: RST  ", Font_11x18, COLOR_BLUE, COLOR_BLACK);
        }
        else if (idle_score > 0.80) {
            ST7735_WriteString(5, 20, "STATUS: IDL  ", Font_11x18, COLOR_BLUE, COLOR_BLACK);
        }
        else {
            ST7735_WriteString(5, 20, "STATUS: ---  ", Font_11x18, COLOR_BLUE, COLOR_BLACK);
        }
    }

    // [하단 영역 디스플레이 제어: CNT 상시 표시 및 변경 시 실시간 업데이트]
    // 카운트가 바뀐 시점에만 화면을 갱신하여 드라이버 SPI 통신 부하 최소화
    if (cnt != last_cnt) {
        char cnt_str[32];
        // 스코어 출력 코드와 마찬가지로 자릿수 변화에 따른 잔상을 지우기 위해 마지막에 공백("  ") 포함
        sprintf(cnt_str, "CNT: %-5u  ", cnt); 
        
        // 화면 하단(Y = 60 위치)에 카운트 텍스트 상시 업데이트
        ST7735_WriteString(5, 60, cnt_str, Font_11x18, COLOR_WHITE, COLOR_BLACK);
        
        last_cnt = cnt; // 카운트 상태 동기화
    }
}

/**
 * @brief ICM42622 IMU 센서 하드웨어 초기화 함수
 */
bool imuInit(void) {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN); 
    gpio_pull_up(I2C_SCL_PIN);
    
    uint8_t DeviceID = ICM42622::Icm42622CheckID();
    if (DeviceID == ICM42622_DEVICE_ID) {
        if (!ICM42622::Icm42622Init()) return false; 
    } else {
        return false; 
    }
    return true;
}