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
    while(true) { // 🚨 실패 시 무한 루프에 가둬서 경고!
      Serial.print("IMU Init Fail");
      sleep_ms(1000);
    }
  }
}

void loop() {
  float Ax, Ay, Ad; 

  // 센서가 데이터를 보낼 준비가 되었다면?
  if (ICM42622::Icm42622DataReady()) {
    // 가속도 데이터 3개를 내 변수에 채워줘!
    ICM42622::Icm42622ReadAccel(&Ax, &Ay, &Ad);

    // 시리얼 모니터로 출력 (탭 '\t'으로 칸 띄우기)
    Serial.print("X:"); Serial.print(Ax); Serial.print('\t');
    Serial.print("Y:"); Serial.print(Ay); Serial.print('\t');
    Serial.print("D:"); Serial.println(Ad);
  }
}
