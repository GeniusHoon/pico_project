# main.py
import uasyncio
import time
from hardware.hardware import hardware as hw
from stock.stock_api import stock as stock_api
from settings.wlan.wlan import connectivity as conn
from machine import Pin

from settings.bluetooth.ble import BLEPeripheral

# --- 설정 ---
TARGET_CODE = "KRW-BTC"
CHECK_INTERVAL = 5       # 테스트니까 10초로 짧게!

wlan = conn()
wlan.connect_wifi()
ble = BLEPeripheral("PicoVibe")  # BLE 주변 장치 인스턴스 생성

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
    global ble

    while True:
        # [감시] 상사 감지
        if is_boss_detected :
            await uasyncio.sleep(0.05)
            continue

        # [주식] 가격 확인
        current_time = time.time()
        if current_time - last_check_time > CHECK_INTERVAL:
            await hardware.blink_led()
            
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


def smoothing(data, window_size=3):
    """간단한 이동 평균 스무딩 함수"""
    # data를 window size만큼 나누
    if len(data) < window_size:
        return sum(data) / len(data)  # 데이터가 적으면 그냥 평균 반환
    return sum(data[-window_size:]) / window_size

# want to make code that measures distance continuously and updates is_boss_detected
# if distance < threshold, is_boss_detected = True else False
# prevent flickering by smoothing
async def boss_loop() :
    global is_boss_detected
    distances = []
    count_thd = 15 # 0.05 * 20 = 1 second
    count = 0
    dist_thd = 100 # cm

    while True :
        dist = await hardware.get_boss_distance()

        if dist < 0 :
            continue # measurement failed skip

        distances.append(dist)

        if len(distances) > 5 :
            distances.pop(0)
        avg_dist = smoothing(distances, window_size=10)

        print(f"Avg Distance: {avg_dist} cur dist : {dist} count: {count} state: {is_boss_detected}")

        if is_boss_detected :
            if avg_dist > dist_thd :
                count += 1
                if count >= count_thd :
                    is_boss_detected = False
                    hardware.display_show_msg("Safe.. Loading..") # 복구 메시지
                    print("✅ Boss Gone. Recovering to normal operation.")
                    count = 0
            else :
                count = 0
        else :
            if avg_dist < dist_thd :
                count += 1
                if count >= count_thd :
                    is_boss_detected = True
                    print("⚠️ Boss Detected! Entering stealth mode.")
                    hardware.display_clear() # 화면 끄기 (Stub 호출)
                    try:
                        if ble.is_connected():
                            await ble.notify_message("Boss Detected!") # BLE 알림
                    except Exception as e:
                        print(f"BLE Notify Error: {e}")
                    count = 0
            else :
                count = 0

        await uasyncio.sleep(0.05)

if __name__ == '__main__':
    uasyncio.create_task(stock_loop())
    uasyncio.create_task(boss_loop())

    uasyncio.get_event_loop().run_forever()
