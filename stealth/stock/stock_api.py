import uasyncio
import json
import urequests

class stock:
    async def get_price(self, code):
        """업비트에서 현재 가격 가져오기"""
        try:
            # 예외 처리를 여기서 해서 main 코드가 안 멈추게 함
            url = f"https://api.upbit.com/v1/ticker?markets={code}"
            res = urequests.get(url)
            data = res.json()
            res.close()
            print(data[0]['trade_price'])
            return data[0]['trade_price']
        except Exception as e:
            print(f"통신 에러: {e}")
            return None
