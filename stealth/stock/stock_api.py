import uasyncio
import json

class stock:
    async def get_price(self, code):
        """업비트에서 현재 가격 가져오기"""
        host = "api.upbit.com"
        path = f"/v1/ticker?markets={code}"

        try:
            # urequests 대신 uasyncio를 사용하여 비동기 HTTP 요청 수행
            reader, writer = await uasyncio.open_connection(host, 443, ssl=True)
            
            request = f"GET {path} HTTP/1.0\r\nHost: {host}\r\nUser-Agent: Pico\r\n\r\n"
            writer.write(request.encode())
            await writer.drain()

            # 헤더 스킵
            while True:
                line = await reader.readline()
                if not line or line == b'\r\n':
                    break

            response = await reader.read()
            data = json.loads(response)
            
            writer.close()
            await writer.wait_closed()

            print(data[0]['trade_price'])
            return data[0]['trade_price']
        except Exception as e:
            print(f"통신 에러: {e}")
            return None
