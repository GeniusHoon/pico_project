#include "ICM42622.h" // 우리가 방금 설치한 센서 전용 라이브러리!
#define I2C_PORT i2c0 // Raspberry Pi Pico의 I2C0 포트 사용
#define I2C_SDA_PIN 4 // 데이터 선
#define I2C_SCL_PIN 5 // 클럭 선

// IMU 센서 초기화 함수
bool imuInit(void) {
  i2c_init(I2C_PORT, 400 * 1000); // I2C 통신 속도(400kHz) 설정
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // 핀 기능 할당
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

  // 💡 3장에서 배운 풀업 저항! I2C 통신선의 안정성을 위해 필수 적용
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  uint8_t DeviceID = ICM42622::Icm42622CheckID();
  if (DeviceID == ICM42622_DEVICE_ID) {
    if (!ICM42622::Icm42622Init()) return false; // 센서 하드웨어 초기화 시작! [cite: 228, 229]
  }
  
  return true;
}

void setup() {
  Serial.begin(9600);
  while(!Serial); // 시리얼 모니터가 열릴 때까지 대기!

  // imuInit()을 실행해서 센서가 정상적으로 응답하는지 확인
  if (imuInit()) { 
    Serial.print("IMU Init Ok"); // 성공!
  } else {
    while(true) { // 🚨 실패 시 무한 x루프에 가둬서 경고!
      Serial.print("IMU Init Fail");
      sleep_ms(1000);
    }
  }
}

#define AVRMAX 5 // 5개의 데이터를 모아서 평균을 냄

// 포인터(*)를 사용해 원본 변수(Gx, Gy, Gz)의 값을 직접 수정!
bool AvrGyroRead(float *GX, float *GY, float *GZ) {
  // static: 함수가 끝나도 데이터(과거 기억)를 지우지 않고 유지하는 마법의 키워드!
  static float AGX[AVRMAX], AGY[AVRMAX], AGZ[AVRMAX];
  static int step = 0;

  if (ICM42622::Icm42622DataReady()) {
    // 센서 값을 읽어서 배열의 'loop' 번째 방에 차곡차곡 저장
    ICM42622::Icm42622ReadGyro(&AGX[step], &AGY[step], &AGZ[step]);
    step++; // 방 번호 증가

    if (AVRMAX == step) { // 5개가 다 모였다면?
      *GX = 0; *GY = 0; *GZ = 0;
      for (int i=0; i<AVRMAX; i++) { // 5개를 전부 더하기
        *GX += AGX[i]; *GY += AGY[i]; *GZ += AGZ[i];
      }
      *GX /= AVRMAX; *GY /= AVRMAX; *GZ /= AVRMAX; // 5로 나누어 평균 내기

      step = 0; // 다음 5개를 모으기 위해 방 번호 초기화
      return true; // "평균값 계산 끝났습니다!" 보고
    }
  }
  return false; // "아직 5개 안 모였어요. 기다리세요."
}

void loop() {
  float Gx, Gy, Gd;
  // AvrGyroRead가 'true(5개 모아서 평균 냄)'를 반환할 때만 출력!
  if (AvrGyroRead(&Gx, &Gy, &Gd)) {
    Serial.print("X:"); Serial.print(Gx); Serial.print('\t');
    Serial.print("Y:"); Serial.print(Gy); Serial.print('\t');
    Serial.print("Z1:"); Serial.print(Gd); Serial.print('\n');
  }
}
