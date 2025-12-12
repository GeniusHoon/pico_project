from micropython import const
import bluetooth
import utime
import uasyncio as asyncio

# ----------------------------------------------------
# 1. BLE 상수 및 UUID 정의
# ----------------------------------------------------

# IRQ 이벤트 상수
_IRQ_CENTRAL_CONNECT = const(1)
_IRQ_CENTRAL_DISCONNECT = const(2)
_IRQ_GATTS_WRITE = const(3)
_IRQ_GATTS_READ_REQUEST = const(4)

# 사용자 정의 서비스 및 특성 UUID (UUID는 16진수 또는 128비트 바이트열로 정의 가능)
# Device Information Service와 비슷하지만, 여기서는 사용자 정의로 알림 전송용으로 사용합니다.
_CUSTOM_SVC_UUID = bluetooth.UUID(0x180A) 
_VIBRATE_CHAR_UUID = bluetooth.UUID(0x2A58)

# 특성 플래그
_FLAG_READ = const(0x0002)
_FLAG_NOTIFY = const(0x0010)

# BLE 서비스 정의: (서비스 UUID, (특성 튜플 리스트))
_VIBRATE_SERVICE = (
    _CUSTOM_SVC_UUID,
    (
        # VIBRATE_CHAR: 읽기 가능 및 알림(Notify) 가능
        (_VIBRATE_CHAR_UUID, _FLAG_READ | _FLAG_NOTIFY), 
    ),
)

# ----------------------------------------------------
# 2. BLE 주변 장치 클래스
# ----------------------------------------------------

class BLEPeripheral:
    """
    HC-SR04 알림을 처리하는 BLE 주변 장치 (Peripheral) 클래스
    다른 메인 로직에서 인스턴스화하여 사용합니다.
    """
    def __init__(self, name='PicoVibe'):
        self._ble = bluetooth.BLE()
        self._ble.active(True)
        self._ble.irq(self._irq)
        
        # 서비스 등록 및 특성 핸들 획득
        # self._handle은 Notify를 위한 _VIBRATE_CHAR_UUID의 핸들입니다.
        ((self._handle,),) = self._ble.gatts_register_services((_VIBRATE_SERVICE,))
        
        self._connections = set()
        self._name = name
        self._advertise_payload = self._create_advertise_payload(name=name, services=[_CUSTOM_SVC_UUID])
        self._advertise()
        print(f"[{name}] BLE Advertising started.")

    def _irq(self, event, data):
        """블루투스 이벤트 핸들러: 연결/해제/쓰기 이벤트 처리"""
        if event == _IRQ_CENTRAL_CONNECT:
            conn_handle, _, _ = data
            self._connections.add(conn_handle)
            print(f"[{self._name}] Connected (Handle: {conn_handle})")
        
        elif event == _IRQ_CENTRAL_DISCONNECT:
            conn_handle, _, _ = data
            if conn_handle in self._connections:
                self._connections.remove(conn_handle)
            self._advertise() # 연결 끊기면 다시 광고 시작
            print(f"[{self._name}] Disconnected. Restarting advertising.")

    def _create_advertise_payload(self, name, services):
        """BLE 광고 패킷 (Advertise Payload) 생성"""
        payload = bytearray()
        
        # 플래그 필드: LE General Discoverable Mode
        payload += b'\x02\x01\x06'
        
        # 이름 필드
        payload += bytearray((len(name) + 1, 0x09)) + name.encode('utf-8')
        return payload

    def _advertise(self, interval_us=100000):
        """광고 시작"""
        self._ble.gap_advertise(interval_us, adv_data=self._advertise_payload)
        
    def is_connected(self):
        """현재 연결된 중앙 장치가 있는지 확인"""
        return len(self._connections) > 0

    async def notify_message(self, message: str) -> bool:
        """
        연결된 모든 중앙 장치(Central)에 알림(Notify) 메시지를 전송합니다.
        
        :param message: 전송할 문자열 데이터
        :return: 알림 전송 성공 여부 (연결된 장치가 있었는지)
        """
        if not self.is_connected():
            return False

        message_bytes = message.encode('utf-8')

        for conn_handle in self._connections:
            # gatts_notify(연결 핸들, 특성 핸들, 값)
            self._ble.gatts_notify(conn_handle, self._handle, message_bytes)
            await asyncio.sleep(0.05)
        # print(f"[{self._name}] Notified: {message}")
        return True

# ----------------------------------------------------
# 3. 테스트 및 사용 예시 (옵션)
# ----------------------------------------------------

async def main_test():
    """BLEPeripheral 클래스 테스트를 위한 메인 코루틴"""
    ble_sensor = BLEPeripheral(name='PicoVibe')
    
    # 5초마다 알림을 보내는 시도를 하는 루프 (핸드폰 연결 후 확인 가능)
    while True:
        if ble_sensor.is_connected():
            success = ble_sensor.notify_message(f"Heartbeat: {utime.time()}")
            if success:
                print("Heartbeat Sent.")
            else:
                print("Notification failed.")
        else:
            print("Awaiting connection...")
            
        await asyncio.sleep(5)
if __name__ == "__main__":
    # 이 파일이 단독으로 실행될 때만 테스트 루프를 실행
    print("Running BLE Peripheral Test. Connect via BLE Scanner App.")
    try:
        asyncio.run(main_test())
    except KeyboardInterrupt:
        print("\nProgram stopped.")
    except Exception as e:
        print(f"Critical error: {e}")