import os

# 아까 확인된 주파수 54Hz 기준 (1000ms / 54 = 약 18.5ms 간격)
INTERVAL_MS = 18.5 

# 에러가 났던 파일 목록
files = [
    'pico4ml_imu_shake1.csv',
    'pico4ml_imu_idle.csv',
    'pico4ml_imu_leftright2.csv',
    'pico4ml_imu_updown2.csv',
    'pico4ml_imu_updown1.csv',
    'pico4ml_imu_shake2.csv',
    'pico4ml_imu_leftright1.csv'
]

for filename in files:
    if not os.path.exists(filename):
        print(f"[{filename}] 파일이 폴더에 없습니다. 스킵합니다.")
        continue
        
    with open(filename, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        
    # Edge Impulse가 원하는 규격으로 헤더 재정의
    new_lines = ["timestamp,AX,AY,AZ\n"]
    timestamp = 0
    
    # 첫 줄 원래 헤더를 제외하고 데이터만 파싱
    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue
        parts = line.split(',')
        
        # 데이터가 정확히 3축(AX, AY, AZ) 다 있는 정상 라인만 필터링 (shake1 에러 방지)
        if len(parts) == 3:
            new_lines.append(f"{int(timestamp)},{parts[0]},{parts[1]},{parts[2]}\n")
            timestamp += INTERVAL_MS
            
    # 기존 파일명에 _ready를 붙여서 새로 저장
    ready_filename = filename.replace('.csv', '_ready.csv')
    with open(ready_filename, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)
    print(f" 성공: {filename} -> {ready_filename} 변환 완료!")

print("\n모든 파일 변환이 끝났습니다. _ready.csv 파일들을 업로드하세요!")