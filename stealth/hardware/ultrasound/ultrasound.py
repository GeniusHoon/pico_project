from machine import Pin
import utime
import uasyncio as asyncio

class ultrasound :
    def __init__(self, trigpin, echopin) :
        # 핀 초기화
        self.trigger = Pin(trigpin, Pin.OUT)
        self.echo = Pin(echopin, Pin.IN)
        pass
    
    async def measure_distance(self):
        # 1. Trigger 핀 초기화 (Low 상태 유지)
        self.trigger.low()
        await asyncio.sleep_ms(2) # 짧은 대기 (비동기적으로 다른 작업 허용)

        # 2. 10us 펄스를 발생시켜 초음파 발사
        self.trigger.high()
        utime.sleep_us(10) # 펄스 지속 시간은 매우 짧으므로 동기적으로 처리
        self.trigger.low()

        # 3. Echo 핀 상태 모니터링 및 시간 측정
        
        # Echo 핀이 High가 될 때까지 비동기적으로 기다림 (측정 시작)
        timeout_us = 25000  # 약 4m 거리까지 측정 가능한 최대 시간 (25ms)
        
        start_time = utime.ticks_us()
        while self.echo.value() == 0:
            await asyncio.sleep_ms(0) # 다른 코루틴에게 제어권 양보
            if utime.ticks_diff(utime.ticks_us(), start_time) > timeout_us:
                # 타임아웃 발생 (물체 감지 실패)
                print("Measurement Failed (Timeout - Low)")
                return -1  
        
        # 펄스 시작 시간 기록
        start_time = utime.ticks_us()
        
        # Echo 핀이 Low가 될 때까지 비동기적으로 기다림 (측정 종료)
        while self.echo.value() == 1:
            await asyncio.sleep_ms(0) # 다른 코루틴에게 제어권 양보
            if utime.ticks_diff(utime.ticks_us(), start_time) > timeout_us:
                # 타임아웃 발생
                print("Measurement Failed (Timeout - High)")
                return -1  
            
        end_time = utime.ticks_us()

        # 4. 거리 계산
        pulse_duration = utime.ticks_diff(end_time, start_time)

        # 거리(cm) = 펄스 지속 시간 (us) / 58
        distance_cm = pulse_duration / 58
        
        # 측정 가능 범위 제한 (400cm가 최대)
        if distance_cm > 400:
            return 401
        
        return distance_cm
"""
async def loopp() :
    while True :
        b = await a.measure_distance()
        print(b)
        await asyncio.sleep_ms(30)
    
a = ultrasound(14,15)
asyncio.run(loopp())
"""