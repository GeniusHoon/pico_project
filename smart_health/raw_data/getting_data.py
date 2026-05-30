import serial
import time

# 설정 (본인의 환경에 맞춤)
PORT = 'COM4'
BAUD_RATE = 115200
OUTPUT_FILE = 'pico4ml_imu_idle.csv'

try:
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"[{PORT}] 연결 성공. 데이터 수집을 시작합니다.")
    print("종료하려면 Ctrl + C를 누르세요...\n")
    
    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        # Edge Impulse가 인식할 수 있도록 맨 첫 줄에 센서 축 이름(Header)을 넣어줍니다.
        f.write("AX,AY,AZ\n")
        
        while True:
            if ser.in_waiting:
                # Pico4ML에서 들어오는 한 줄을 읽음
                line = ser.readline().decode('utf-8').strip()
                if line:
                    # 화면에 출력하면서 동시에 CSV 파일에 저장
                    print(line)
                    f.write(line + '\n')
                    
except KeyboardInterrupt:
    print("\n데이터 수집이 사용자에 의해 종료되었습니다.")
except Exception as e:
    print(f"\n에러 발생: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("시리얼 포트가 안전하게 닫혔습니다.")