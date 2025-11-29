# main.py
import time
from hardware.hardware import hardware as hw
from stock.stock_api import stock as stock_api
from settings.wlan.wlan import connectivity as conn

# --- 설정 ---
TARGET_CODE = "KRW-BTC"
CHECK_INTERVAL = 10       # 테스트니까 10초로 짧게!

# --- 1. 시작 준비 ---
wlan = conn()
wlan.connect_wifi()

hardware = hw()
hardware.display_init()   # 디스플레이 켜기 (Stub)
hardware.alert_vibration()

stock = stock_api()
print("=== Stealth V2 (Display Stub) ===")

last_check_time = 0
prev_price = None # 전 가격 저장용 변수

    # --- 2. 무한 루프 ---
while True:
    # [감시] 상사 감지
    # if hardware.is_boss_detected():
    #     hardware.display_clear() # 화면 끄기 (Stub 호출)
        
    #     while hardware.is_boss_detected():
    #         time.sleep(0.5)
        
    #     print("✅ end recover to normal operation")
    #     hardware.display_show_msg("Safe.. Loading..") # 복구 메시지
    #     time.sleep(2)
    #     continue

    # [주식] 가격 확인
    current_time = time.time()
    if current_time - last_check_time > CHECK_INTERVAL:
        hardware.blink_led()
        
        # 가격 가져오기
        curr_price = stock.get_price(TARGET_CODE)
        
        if curr_price:
            # 색상 결정 로직 (전 가격과 비교)
            color = 'WHITE'
            if prev_price is not None:
                if curr_price > prev_price:
                    color = 'RED'
                elif curr_price < prev_price:
                    color = 'BLUE'
            
            # 디스플레이 Stub 호출!
            hardware.display_show_price(TARGET_CODE, curr_price, color)
            
            # 가격 알림 (예시)
            if curr_price > 100000000: 
                hardware.alert_vibration()

            # 다음 비교를 위해 현재 가격 저장
            prev_price = curr_price

        last_check_time = current_time

    time.sleep(0.1)
