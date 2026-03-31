#!/usr/bin/env python3
"""
Мост между веб-интерфейсом и реальным IoT устройством
Эмулирует подключение к Secure IoT Sensor Node через serial/USB
"""

import serial
import json
import time
import threading
import queue
from datetime import datetime
import sys

class DeviceBridge:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_conn = None
        self.data_queue = queue.Queue()
        self.running = False
        self.device_connected = False
        
    def connect(self):
        """Подключение к устройству"""
        try:
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1,
                write_timeout=1
            )
            self.device_connected = True
            print(f"✅ Подключено к {self.port}")
            return True
        except Exception as e:
            print(f"❌ Ошибка подключения: {e}")
            self.device_connected = False
            return False
    
    def disconnect(self):
        """Отключение от устройства"""
        if self.serial_conn:
            self.serial_conn.close()
            self.device_connected = False
            print("🔌 Отключено от устройства")
    
    def send_command(self, command):
        """Отправка команды на устройство"""
        if not self.device_connected:
            return {"status": "error", "message": "Устройство не подключено"}
        
        try:
            cmd_json = json.dumps({"cmd": command}) + "\n"
            self.serial_conn.write(cmd_json.encode())
            
            # Ожидание ответа
            response = self.serial_conn.readline().decode().strip()
            if response:
                return json.loads(response)
            else:
                return {"status": "timeout", "message": "Нет ответа от устройства"}
                
        except Exception as e:
            return {"status": "error", "message": str(e)}
    
    def read_sensor_data(self):
        """Чтение данных с датчиков"""
        if not self.device_connected:
            return self.generate_mock_data()
        
        try:
            # Запрос данных у устройства
            self.serial_conn.write(json.dumps({"cmd": "get_sensors"}).encode() + b"\n")
            
            # Чтение ответа
            response = self.serial_conn.readline().decode().strip()
            if response:
                data = json.loads(response)
                data["timestamp"] = datetime.now().isoformat()
                return data
            else:
                return self.generate_mock_data()
                
        except Exception as e:
            print(f"❌ Ошибка чтения данных: {e}")
            return self.generate_mock_data()
    
    def generate_mock_data(self):
        """Генерация мок данных когда устройство не подключено"""
        import random
        import math
        
        now = datetime.now()
        hour = now.hour
        base_temp = 22 + 3 * math.sin((hour - 6) * math.pi / 12)
        
        return {
            "timestamp": now.isoformat(),
            "temperature": round(base_temp + random.uniform(-1, 1), 1),
            "cpu": round(30 + random.uniform(0, 40), 1),
            "memory": round(40 + random.uniform(0, 30), 1),
            "pulse": round(65 + random.uniform(-10, 15)),
            "battery": round(87 - random.uniform(0, 5), 1),
            "status": "simulated"
        }
    
    def start_monitoring(self, callback):
        """Запуск мониторинга устройства в отдельном потоке"""
        def monitor():
            while self.running:
                try:
                    data = self.read_sensor_data()
                    callback(data)
                    time.sleep(2)  # Обновление каждые 2 секунды
                except Exception as e:
                    print(f"❌ Ошибка мониторинга: {e}")
                    time.sleep(5)  # Пауза при ошибке
        
        self.running = True
        self.monitor_thread = threading.Thread(target=monitor, daemon=True)
        self.monitor_thread.start()
        print("📊 Мониторинг запущен")
    
    def stop_monitoring(self):
        """Остановка мониторинга"""
        self.running = False
        if hasattr(self, 'monitor_thread'):
            self.monitor_thread.join(timeout=5)
        print("⏹️ Мониторинг остановлен")

class IntegratedAPI:
    """Интегрированный API с поддержкой реального устройства"""
    
    def __init__(self):
        self.bridge = DeviceBridge()
        self.sensor_data = []
        self.events = []
        self.device_status = "offline"
        
        # Попытка подключения к устройству
        self.try_connect_device()
    
    def try_connect_device(self):
        """Попытка подключения к устройству"""
        ports = ['/dev/ttyUSB0', '/dev/ttyACM0', 'COM3', 'COM4']
        
        for port in ports:
            if self.bridge.connect():
                self.device_status = "online"
                self.add_event("success", f"Подключено к {port}")
                return True
        
        print("⚠️ Устройство не найдено, используем симуляцию")
        self.device_status = "simulated"
        self.add_event("warning", "Устройство не найдено, используется симуляция")
        return False
    
    def get_status(self):
        """Получение статуса системы"""
        return {
            "status": self.device_status,
            "device_connected": self.bridge.device_connected,
            "port": self.bridge.port if self.bridge.device_connected else None,
            "uptime": "5d 14h 23m",  # Можно получить от устройства
            "firmware": "v2.1.3"
        }
    
    def get_sensors(self):
        """Получение данных с датчиков"""
        data = self.bridge.read_sensor_data()
        
        # Сохраняем в историю
        self.sensor_data.append(data)
        if len(self.sensor_data) > 100:
            self.sensor_data.pop(0)
        
        return data
    
    def send_command(self, command):
        """Отправка команды устройству"""
        response = self.bridge.send_command(command)
        
        # Логируем событие
        if response.get("status") == "success":
            self.add_event("info", f"Команда '{command}' выполнена успешно")
        else:
            self.add_event("error", f"Ошибка команды '{command}': {response.get('message')}")
        
        return response
    
    def get_events(self, limit=20):
        """Получение журнала событий"""
        return self.events[-limit:]
    
    def add_event(self, event_type, message):
        """Добавление события"""
        event = {
            "timestamp": datetime.now().isoformat(),
            "type": event_type,
            "message": message
        }
        self.events.insert(0, event)
        
        # Ограничиваем размер журнала
        if len(self.events) > 50:
            self.events.pop()

# Глобальный экземпляр для использования в API
integrated_api = IntegratedAPI()

if __name__ == "__main__":
    # Тестирование моста
    print("🔧 Тестирование моста устройства...")
    
    api = IntegratedAPI()
    
    # Тест получения данных
    print("\n📊 Тест получения данных:")
    data = api.get_sensors()
    print(json.dumps(data, indent=2, ensure_ascii=False))
    
    # Тест команды
    print("\n🎮 Тест команды:")
    response = api.send_command("led_on")
    print(json.dumps(response, indent=2, ensure_ascii=False))
    
    # Тест статуса
    print("\n📈 Тест статуса:")
    status = api.get_status()
    print(json.dumps(status, indent=2, ensure_ascii=False))
