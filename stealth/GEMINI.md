# Project Overview
이 프로젝트는 **Raspberry Pi Pico 2W (MicroPython)**를 기반으로 한 "사무실용 스텔스 암호화폐 티커(Stealth Crypto Ticker)"입니다.
직장 상사 몰래 주식/코인 시세를 확인하다가, 상사가 접근하면(센서 감지) 즉시 업무 모드 화면으로 전환하는 시스템을 구축합니다.

## 🎯 Key Goals (Level: Undergraduate Software Engineering)
단순한 기능 구현을 넘어, 임베디드 시스템의 **실시간성(Real-time)**과 **자원 효율성(Resource Efficiency)**을 고려한 아키텍처를 지향합니다.
1. **Concurrency:** `uasyncio`를 활용한 Non-blocking 아키텍처 구현 (단일 코어 멀티태스킹).
2. **State Management:** '감시 모드'와 '업무 모드(Fake Mode)' 간의 명확한 상태 전환 관리.
3. **Modularity:** 하드웨어 제어 계층(HAL)과 비즈니스 로직의 철저한 분리.

## 🛠 Tech Stack & Environment
- **Device:** Raspberry Pi Pico 2W (RP2040)
- **Language:** MicroPython (Latest Firmware)
- **Display:** LCD1602 (I2C Interface)
- **Sensor:** Ultrasonic or Radar Sensor (Digital Input)
- **Connectivity:** WiFi (IEEE 802.11n), REST API (Upbit), WebSocket (Future plan)

## 📐 Architecture Guidelines (Strict Rules)
AI 어시스턴트는 코드를 생성할 때 다음 규칙을 반드시 준수해야 합니다.

### 1. 비동기 프로그래밍 필수 (Asyncio)
- **DO NOT USE:** `time.sleep()` (메인 루프나 태스크 내에서 절대 금지).
- **USE:** `await asyncio.sleep()`을 사용하여 CPU 점유를 다른 태스크에 양보해야 함.
- 네트워크 요청(HTTP/WebSocket)과 센서 모니터링은 서로 다른 `async` 태스크로 분리되어 병렬 실행되어야 함.

### 2. 하드웨어 추상화 (HAL)
- `main.py`는 핀 번호나 하드웨어 제어 로직을 직접 알면 안 됨.
- 모든 하드웨어 제어는 `hardware.py`의 클래스 메서드를 통해서만 수행.
- 예: `Pin(15, Pin.OUT).value(1)` (X) -> `hardware.alert_on()` (O)

### 3. 상태 관리 (State Machine)
- 시스템 전역 상태(예: `IS_BOSS_DETECTED`)를 공유하여 태스크 간 충돌 방지.
- 상사 감지 인터럽트 발생 시, UI 갱신 태스크는 즉시 중단되거나 Override 되어야 함.

## 🔌 Hardware Pinout Configuration
코드를 작성할 때 아래 핀 맵을 기준점으로 삼을 것.
- **I2C0 SDA:** GP0
- **I2C0 SCL:** GP1
- **Radar/Sonic Sensor:** GP16 (Input)
- **Buzzer:** GP15 (PWM)
- **Built-in LED:** "LED" (Pico W/2W)

## 📝 Coding Convention
- **Type Hinting:** 모든 함수 인자와 반환값에 타입 힌트 명시 (예: `def get_price(code: str) -> float:`).
- **Docstrings:** 주요 함수에는 기능을 설명하는 Docstring 포함.
- **Error Handling:** 네트워크 연결 끊김, 센서 오작동에 대한 예외 처리(`try-except`) 필수.

## 🚀 Current Roadmap
1. [Completed] 기본 하드웨어(LCD, Buzzer) 제어 및 동기식 API 호출.
2. [In-Progress] **`uasyncio`를 도입하여 센서 감시와 시세 조회를 비동기로 전환.** (현재 집중 단계)
3. [Planned] REST API를 WebSocket으로 업그레이드하여 실시간성 확보.
4. [Planned] 블루투스(BLE)를 이용한 원격 설정 기능 추가.