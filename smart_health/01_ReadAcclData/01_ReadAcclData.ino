#include <ICM42622.h> // Pico4ML에 내장된 ICM42622 IMU(관성 측정 장치) 센서 라이브러리 포함

// I2C 통신을 위한 핀 번호 및 포트 정의 (Pico4ML 하드웨어 스펙에 맞춤)
#define I2C_PORT i2c0 
#define I2C_SDA_PIN 4 // 데이터를 주고받는 SDA 핀
#define I2C_SCL_PIN 5 // 통신 타이밍을 맞추는 SCL(클럭) 핀

void setup() {
  // 1. 시리얼 통신 시작 (속도: 115200bps)
  // Edge Impulse Data Forwarder가 기본적으로 사용하는 통신 속도입니다.
  Serial.begin(115200);

  // 2. PC(시리얼 모니터)가 연결될 때까지 대기
  // 보드에 전원이 들어가자마자 데이터가 날아가는 것을 방지합니다.
  while(!Serial);
  
  // 3. IMU 센서 초기화 함수 호출 (하단에 직접 정의한 imuInit 함수 실행)
  if (imuInit()) 
  {
      // 초기화 성공 시 상태 메시지 출력
      // 주의: Data Forwarder가 데이터로 오해하지 않도록 반드시 줄바꿈(println)을 사용해야 합니다.
      // Serial.println("IMU Init Ok"); 
  } else {
      // 초기화 실패 시 에러 메시지를 무한 반복하며 프로그램 중단
      while(true)
      {
        // Serial.println("IMU Init Fail"); 
        sleep_ms(1000); // 1초 대기 (Arduino IDE의 경우 delay(1000)으로 작성해도 됩니다)
      }
  }
}

void loop()
{
  // X, Y, Z축 가속도 값을 저장할 소수점(float) 변수 선언
  float Ax, Ay, Az;

  // 1. 센서 내부에 새로운 가속도 데이터가 준비되었는지 확인
  if (ICM42622::Icm42622DataReady()) {
    
    // 2. 센서에서 가속도 값을 읽어와 Ax, Ay, Az 변수에 저장 (주소값 '&' 전달)
    ICM42622::Icm42622ReadAccel(&Ax, &Ay, &Az);
    
    // 3. 수집된 데이터를 PC로 전송 (Edge Impulse 요구 포맷: 값,값,값\n)
    Serial.print(Ax);  // X축 데이터 출력
    Serial.print(','); // 쉼표로 데이터 구분 (CSV 포맷)
    Serial.print(Ay);  // Y축 데이터 출력
    Serial.print(','); // 쉼표로 데이터 구분
    Serial.println(Az);// Z축 데이터 출력 후 반드시 줄바꿈(엔터) 실행
    Serial.println();
  }
}

// IMU 센서를 초기화하기 위해 우리가 직접 만든 함수
bool imuInit(void)
{
  // 1. I2C 통신 포트를 400kHz(고속 모드) 속도로 초기화
  i2c_init(I2C_PORT, 400 * 1000);
  
  // 2. 4번, 5번 핀을 일반 입출력이 아닌 I2C 전용 기능으로 설정
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  
  // 3. I2C 통신에 필수적인 풀업(Pull-up) 저항 내부 활성화
  gpio_pull_up(I2C_SDA_PIN); 
  gpio_pull_up(I2C_SCL_PIN);
  
  // 4. 센서 연결 확인: 센서 고유의 장치 ID를 읽어옴
  uint8_t DeviceID = ICM42622::Icm42622CheckID();
  
  // 5. 읽어온 ID가 실제 ICM42622 센서의 ID와 일치하는지 검사
  if (DeviceID == ICM42622_DEVICE_ID) {
    // ID가 일치하면 센서 세부 초기화 진행
    if (!ICM42622::Icm42622Init())
      return false; // 세부 초기화 실패 시 false 반환
  } else {
      return false; // ID가 다르면(연결 불량 등) false 반환
  }
  
  // 모든 초기화 과정을 무사히 통과하면 true 반환
  return true;
}