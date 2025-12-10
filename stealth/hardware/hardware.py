# hardware.py
from machine import Pin, PWM, I2C
import time
import hardware.lcd.lcd as I2cLcd  # 방금 만든 라이브러리 불러오기


# --- 핀 설정 ---
RADAR_PIN = 16
BUZZER_PIN = 15
# I2C 핀 (SDA: GP0, SCL: GP1)
I2C_SDA = 0
I2C_SCL = 1

class hardware :
    def __init__(self, radar_pin=RADAR_PIN, buzzer_pin=BUZZER_PIN):
        # --- 부품 초기화 ---
        self.radar = Pin(radar_pin, Pin.IN)
        self.buzzer = PWM(Pin(buzzer_pin))
        self.buzzer.freq(1000)
        self.led = Pin("LED", Pin.OUT)

        # LCD 설정 (주소는 보통 0x27, 안 되면 0x3F로 바꿔보세요)
        # i2c = I2C(0, sda=Pin(I2C_SDA), scl=Pin(I2C_SCL), freq=400000)
        # 16글자 2줄짜리 LCD 객체 생성
        try:
            self.lcd = I2cLcd.lcd(0x27)
        except:
            print("LCD 연결 실패! 주소(0x27/0x3F)나 배선을 확인하세요.")
            self.lcd = None
        return

    # --- 기능 함수들 ---
    def is_boss_detected(self):
        return self.radar.value() == 1

    def alert_vibration(self):
        self.buzzer.duty_u16(30000)
        time.sleep(0.2)
        self.buzzer.duty_u16(0)

    def blink_led(self):
        self.led.on()
        time.sleep(0.1)
        self.led.off()

    # ==========================
    # 👇 진짜 LCD 제어 함수들
    # ==========================

    def display_init(self):
        if self.lcd:
            self.lcd.clear()
            time.sleep(1)

    def display_clear(self):
        """보스 모드: 화면을 싹 지우거나 가짜 메시지 출력"""
        if self.lcd:
            self.lcd.clear()
            # 아무것도 안 띄우면 백라이트만 켜져 있어서 더 수상할 수 있음
            # 차라리 가짜 에러 메시지나 지루한 텍스트 추천
            self.lcd.print("Updating...")

    def display_show_price(self, code, price, color):
        """
        LCD는 색상이 없으므로 글자로 표현
        color: RED -> 'UP', BLUE -> 'DN'
        """
        if not self.lcd: return

        # 가격 포맷 (쉼표 넣기)
        price_str = "{:,}".format(int(price))
        print(price_str)
        # 등락 표시 문자
        arrow = "-"
        if color == 'RED': arrow = "^" # 오름
        elif color == 'BLUE': arrow = "v" # 내림
        
        # 화면 갱신
        self.lcd.clear()
        
        # 첫째 줄: 종목명 (예: BTC/KRW)
        self.lcd.setCursor(0, 0)
        self.lcd.print(code[:16]) # 16자 넘으면 자름
        
        # 둘째 줄: 가격 및 화살표 (예: 98,000,000 ^)
        self.lcd.setCursor(0, 1)
        self.lcd.print(f"{price_str} {arrow}")
        
    def display_show_msg(self, text):
        if self.lcd:
            self.lcd.clear()
            self.lcd.setCursor(0, 0)
            self.lcd.print(text[:16])