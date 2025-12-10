import network
import time
import json
import uasyncio

class connectivity :
    def __init__(self):
        return
    
    def getSessionFromJson(self) :
        # Load credentials from json
        try:
            with open('/settings/wlan/credentials.json', 'r') as f:
                auth_data = json.load(f)
                ssid = auth_data['ssid']
                password = auth_data['password']
                return ssid, password
        except Exception as e:
            print(f"Failed to load authentication.json: {e}")
            return "none","none"

    async def connect_wifi(self, max_retries=5, per_attempt_timeout=10, retry_delay=2):
        """와이파이 연결 (재시도 및 타임아웃 처리)
        Returns True on success, False on failure.
        """
        wlan = network.WLAN(network.STA_IF)
        wlan.active(True)

        # already connected
        if wlan.isconnected():
            print("Already connected.")
            return True

        for attempt in range(1, max_retries + 1):
            print(f"Retry... {attempt}/{max_retries}...", end="")
            try:
                # get data from json
                ssid, password = self.getSessionFromJson()
                if ssid == "none" :
                    print("No SSID found in authentication.json")
                    return False
                
                wlan.connect(ssid, password)
            except Exception as e:
                print(f"\nconnection failed: {e}")

            start = time.time()
            while time.time() - start < per_attempt_timeout:
                if wlan.isconnected():
                    print("\nconnection successful!")
                    return True
                await uasyncio.sleep(1)
                print(".", end="")

            print("\nTimeout not connected.")

            # 클린업: 연결 해제 및 인터페이스 재시작 전 잠깐 대기
            try:
                if hasattr(wlan, "disconnect"):
                    wlan.disconnect()
            except Exception:
                pass
            wlan.active(False)
            await uasyncio.sleep(retry_delay)
            wlan.active(True)

        print("fail to reconnect .")
        return False
