#include <face_detection_inferencing.h>
#include "LCD_st7735.h"      // PICO4ML 내장 디스플레이 드라이버
#include "arducam_hm01b0.h"  // PICO4ML 내장 카메라 라이브러리

// 8비트 흑백 데이터를 16비트(RGB565) 컬러 규격으로 변환하기 위한 고속 매크로
#define ST7735_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

/* 디스플레이용 RGB565 컬러 매크로 정의 */
#define COLOR_RED   0xF800 // 타인(face) 바운딩 박스용 빨간색
#define COLOR_GREEN 0x07E0 // 주인(owner) 바운딩 박스용 초록색
#define COLOR_WHITE 0xFFFF // 화면 하단 텍스트 색상
#define COLOR_BLACK 0x0000 // 텍스트 배경색 (잔상 제거용)

// [전역 버퍼 메모리 할당]
uint8_t image_data[96 * 96];      // 카메라 원본 흑백 영상 버퍼 (96x96 = 9,216 bytes)
uint8_t displayBuf[96 * 96 * 2];  // LCD 16비트 컬러 영상 버퍼 (18,432 bytes)

struct arducam_config config;     // 카메라 설정 구조체

// ------------------------------------------------------------------------------
// 1. Edge Impulse 데이터 공급 콜백 함수
// ------------------------------------------------------------------------------
int ei_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        uint8_t pixel = image_data[offset + i]; 
        out_ptr[i] = (pixel << 16) | (pixel << 8) | pixel; // RGB888 형태로 밀어넣기
    }
    return 0; 
}

// ------------------------------------------------------------------------------
// 2. 디스플레이 버퍼(displayBuf) 조작 함수: 점 찍기
// ------------------------------------------------------------------------------
void draw_pixel_to_buffer(int x, int y, uint16_t color) {
    if (x < 0 || x >= 96 || y < 0 || y >= 96) return;
    
    int index = (y * 96 + x) * 2;
    displayBuf[index] = (color >> 8) & 0xFF;     
    displayBuf[index + 1] = color & 0xFF;        
}

// ------------------------------------------------------------------------------
// 3. 바운딩 박스 렌더링 함수: 사각형 그리기
// ------------------------------------------------------------------------------
void draw_box_to_buffer(int x, int y, int w, int h, uint16_t color) {
    for (int i = x; i < x + w; i++) {
        draw_pixel_to_buffer(i, y, color);
        draw_pixel_to_buffer(i, y + h - 1, color);
    }
    for (int i = y; i < y + h; i++) {
        draw_pixel_to_buffer(x, i, color);
        draw_pixel_to_buffer(x + w - 1, i, color);
    }
}

void setup() {
    Serial.begin(115200); 
    
    // [Step 1] LCD 디스플레이 및 카메라 하드웨어 초기화
    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK);

    config.sccb = i2c0;
    config.sccb_mode = I2C_MODE_16_8;    
    config.sensor_address = 0x24;        
    config.pin_sioc = PIN_CAM_SIOC;
    config.pin_siod = PIN_CAM_SIOD;
    config.pin_resetb = PIN_CAM_RESETB;
    config.pin_xclk = PIN_CAM_XCLK;
    config.pin_vsync = PIN_CAM_VSYNC;    
    config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE; 
    config.pio = pio0;                   
    config.pio_sm = 0;
    config.dma_channel = 0;              
    
    arducam_init(&config); 
    
    Serial.println("PICO4ML Multi-Label Face Detection Started...");
}

void loop() {
    // [Step 2] 영상 캡처 (DMA)
    arducam_capture_frame(&config, image_data);

    // [Step 3] 흑백 버퍼 데이터를 LCD용 16비트 컬러 버퍼로 복사 및 비트 변환
    uint16_t index = 0;
    for (int x = 0; x < 96 * 96; x++) {
        uint8_t pixel = image_data[x];
        uint16_t imageRGB = ST7735_COLOR565(pixel, pixel, pixel);
        displayBuf[index++] = (imageRGB >> 8) & 0xFF; 
        displayBuf[index++] = imageRGB & 0xFF;
    }

    // [Step 4] Edge Impulse 인공지능 추론 엔진 실행 준비
    signal_t signal; // 네임스페이스 및 타입 통일
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT; 
    signal.get_data = &ei_get_data; 

    ei_impulse_result_t result = { 0 }; 
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false); 

    if (err != EI_IMPULSE_OK) {
        Serial.printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // [Step 5] 추론 결과 분석 및 클래스별 바운딩 박스 분기 조건문
    bool owner_found = false; // 현재 화면에 주인이 있는지 판별 플래그
    bool face_found  = false; // 현재 화면에 일반 타인이 있는지 판별 플래그
    float max_score  = 0.0f;  // 타인 혹은 주인의 최상위 스코어 기록용

#if EI_CLASSIFIER_OBJECT_DETECTION == 1 
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        
        // 신뢰도 임계값 0.5(50%)를 초과하는 유효 객체만 핸들링
        if (bb.value > 0.5) {
            if (bb.value > max_score) max_score = bb.value;

            // 문자열 비교 함수(strcmp)를 사용하여 감지된 라벨(bb.label)을 구체적으로 분기합니다.
            if (strcmp(bb.label, "owner") == 0) {
                owner_found = true;
                Serial.printf("[OWNER] 감지! 확률: %f [x:%u, y:%u]\n", bb.value, bb.x, bb.y);
                // 주인인 경우 초록색(COLOR_GREEN) 박스 렌더링
                draw_box_to_buffer(bb.x, bb.y, bb.width, bb.height, COLOR_GREEN);
            } 
            else if (strcmp(bb.label, "face") == 0) {
                face_found = true;
                Serial.printf("[FACE] 일반 타인 감지! 확률: %f [x:%u, y:%u]\n", bb.value, bb.x, bb.y);
                // 일반 타인인 경우 빨간색(COLOR_RED) 박스 렌더링
                draw_box_to_buffer(bb.x, bb.y, bb.width, bb.height, COLOR_RED);
            }
            // background 클래스는 사각형을 그릴 필요가 없으므로 자연스럽게 무시됩니다.
        }
    }
#endif

    // [Step 6] 카메라 프레임 최종 출력 (96x96 해상도 전송)
    ST7735_DrawImage(0, 0, 96, 96, displayBuf);

    // [Step 7] LCD 하단 (Y = 100) 결과 텍스트 업데이트 분기 로직
    char score_str[32];
    
    // 우선순위 결정: 화면에 주인과 타인이 모두 있다면 'Owner'를 우선 표시합니다.
    if (owner_found) {
        sprintf(score_str, "OWNER: %3d%%  ", (int)(max_score * 100));
    } 
    else if (face_found) {
        sprintf(score_str, "FACE: %3d%%   ", (int)(max_score * 100));
    } 
    else {
        sprintf(score_str, "Searching... ");
    }
    
    // 최종 텍스트 드라이버 덤프출력 (잔상 제거용 공백 패딩 적용됨)
    ST7735_WriteString(5, 100, score_str, Font_7x10, COLOR_WHITE, COLOR_BLACK);
}