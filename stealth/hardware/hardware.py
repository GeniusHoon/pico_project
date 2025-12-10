# hardware.py
from machine import Pin, PWM, I2C
import time
from hardware.lcd import LcdApi
from hardware.lcd.pico_i2c_lcd import I2cLcd
from hardware.ultrasound.ultrasound import ultrasound

# --- 핀 정의 (사용자가 연결한 핀 번호로 변경) ---
TRIG_PIN = 14
ECHO_PIN = 15
# ---------------------------------------------

# I2C 핀 (SDA: GP0, SCL: GP1)
I2C_SDA = 0
I2C_SCL = 1
I2C_ADDR = 0x27  # LCD I2C 주소 (보통 0x27 또는 0x3F)
I2C_NUM_ROWS = 2
I2C_NUM_COLS = 16

class hardware :
    def __init__(self, i2c_addr=I2C_ADDR):
        # --- 부품 초기화 ---
        self.ultra = ultrasound(TRIG_PIN, ECHO_PIN)
        self.led = Pin("LED", Pin.OUT)

        # i2c = I2C(0, sda=Pin(I2C_SDA), scl=Pin(I2C_SCL), freq=400000)
        # 16글자 2줄짜리 LCD 객체 생성
        try:
            self.i2c = I2C(0, sda=Pin(I2C_SDA), scl=Pin(I2C_SCL), freq=400000)
            self.lcd = I2cLcd(self.i2c, i2c_addr, I2C_NUM_ROWS, I2C_NUM_COLS)    
            self.lcd.putstr("It is working!")
        except Exception as e:
            print("LCD 연결 실패! 주소(0x27/0x3F)나 배선을 확인하세요. exception:",e)
            self.lcd = None
        return

    # --- 기능 함수들 ---
    async def get_boss_distance(self):
        return await self.ultra.measure_distance()

    def alert_vibration(self):
        print("alert temp")

    def blink_led(self):
        self.led.on()
        time.sleep(0.1)
        self.led.off()
        
    # LCD #####################################    
    def display_init(self):
        if self.lcd:
            self.lcd.clear()
            time.sleep(1)

    def display_clear(self):
        if self.lcd:
            self.lcd.clear()
            # 아무것도 안 띄우면 백라이트만 켜져 있어서 더 수상할 수 있음
            self.lcd.putstr("Updating...")

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
        self.lcd.move_to(0, 0)
        self.lcd.putstr(code[:16]) # 16자 넘으면 자름
        
        # 둘째 줄: 가격 및 화살표 (예: 98,000,000 ^)
        self.lcd.move_to(0, 1)
        self.lcd.putstr(f"{price_str} {arrow}")
        
    def display_show_msg(self, text):
        if self.lcd:
            self.lcd.clear()
            self.lcd.move_to(0, 0)
            self.lcd.putstr(text[:16])