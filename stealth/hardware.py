# hardware.py
from machine import Pin, PWM
import time

# --- 핀 번호 설정 ---
RADAR_PIN = 16
BUZZER_PIN = 15

# --- 부품 초기화 ---
radar = Pin(RADAR_PIN, Pin.IN)
buzzer = PWM(Pin(BUZZER_PIN))
buzzer.freq(1000)
led = Pin("LED", Pin.OUT)

# --- 기능 함수들 ---

def is_boss_detected():
    """상사가 있으면 True"""
    return radar.value() == 1

def alert_vibration():
    """징- 알림"""
    buzzer.duty_u16(30000)
    time.sleep(0.2)
    buzzer.duty_u16(0)

def blink_led():
    led.on()
    time.sleep(0.1)
    led.off()

# ==========================================
# 👇 [Display Stub] 디스플레이 가상 구현 부분
# ==========================================

def display_init():
    """디스플레이 초기화 (가상)"""
    print("\n[🖥️ Display Stub] 초기화 완료! 화면 대기 중...")

def display_clear():
    """화면 끄기/검은색 채우기 (가상)"""
    print("\n[🖥️ Display Stub] ⚫ 화면 꺼짐 (Stealth Mode Activated)")

def display_show_price(code, price, color):
    """
    가격 정보 띄우기 (가상)
    color: 'RED'(상승), 'BLUE'(하락), 'WHITE'(변동없음)
    """
    # 보기 좋게 포맷팅 (천 단위 쉼표 추가)
    formatted_price = "{:,}".format(int(price))
    
    icon = "➖"
    if color == 'RED': icon = "🔺"
    elif color == 'BLUE': icon = "uq"
    
    print(f"\n[🖥️ Display Stub] {code} | {formatted_price}원 | {icon} <Color: {color}>")

def display_show_msg(text):
    """간단한 메시지 띄우기"""
    print(f"\n[🖥️ Display Stub] 메시지 출력: {text}")