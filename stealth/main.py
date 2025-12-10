# main.py
import uasyncio
import time
from hardware.hardware import hardware as hw
from stock.stock_api import stock as stock_api
from settings.wlan.wlan import connectivity as conn
from machine import Pin
# --- 설정 ---
TARGET_CODE = "KRW-BTC"
CHECK_INTERVAL = 10       # 테스트니까 10초로 짧게!

wlan = conn()
wlan.connect_wifi()

hardware = hw()
hardware.display_init()   # 디스플레이 켜기 (Stub)
#hardware.alert_vibration()
stock = stock_api()
print("=== Stealth V2 (Display Stub) ===")
last_check_time = 0
prev_price = None # 전 가격 저장용 변수

is_boss_detected = False

async def stock_loop():
    global prev_price
    global is_boss_detected
    global last_check_time
    
    while True:
        # [감시] 상사 감지
        print("stock loop running")
        if is_boss_detected :
            hardware.display_clear() # 화면 끄기 (Stub 호출)
            
            print("✅ end recover to normal operation")
            hardware.display_show_msg("Safe.. Loading..") # 복구 메시지
            await uasyncio.sleep(2)
            continue

        # [주식] 가격 확인
        current_time = time.time()
        if current_time - last_check_time > CHECK_INTERVAL:
            hardware.blink_led()
            
            # 가격 가져오기
            curr_price = await stock.get_price(TARGET_CODE)
            
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
                    print("wow")#hardware.alert_vibration()

                # 다음 비교를 위해 현재 가격 저장
                prev_price = curr_price

            last_check_time = current_time

        await uasyncio.sleep(0.1)

async def boss_loop() :
    global is_boss_detected
    while True :
        dist = await hardware.get_boss_distance()
        print(dist)
        if dist > 0 and dist < 30 :
            is_boss_detected = True
        else :
            is_boss_detected = False

        await uasyncio.sleep(0.1)

if __name__ == '__main__':
    uasyncio.create_task(stock_loop())
    uasyncio.create_task(boss_loop())

    uasyncio.get_event_loop().run_forever()
