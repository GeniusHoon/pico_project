# stock_api.py
import network
import urequests
import time
import secrets  # 위에서 만든 secrets.py를 불러옴

def connect_wifi():
    """와이파이 연결"""
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(secrets.SSID, secrets.PASSWORD)

    print("와이파이 연결 중...", end="")
    while not wlan.isconnected():
        time.sleep(1)
        print(".", end="")
    print("\n연결 성공!")

def get_price(code):
    """업비트에서 현재 가격 가져오기"""
    try:
        # 예외 처리를 여기서 해서 main 코드가 안 멈추게 함
        url = f"https://api.upbit.com/v1/ticker?markets={code}"
        res = urequests.get(url)
        data = res.json()
        res.close()
        return data[0]['trade_price']
    except Exception as e:
        print(f"통신 에러: {e}")
        return None