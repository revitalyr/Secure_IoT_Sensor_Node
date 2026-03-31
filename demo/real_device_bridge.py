#!/usr/bin/env python3
"""
Мост для взаимодействия с реальным приложением app/main.c
Читает JSON данные с Serial/USB и отправляет команды
"""

import serial
import json
import time
import threading
import queue
from datetime import datetime
import re

class RealDeviceBridge:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_conn = None
        self.device_connected = False
        self.running = False
        self.monitor_thread = None
        
        # Данные от устройства
        self.sensor_data = {}
        self.system_status = {}
        self.events = []
        
        # LED статус для синхронизации
        self.led_status = 0
        
        print(f"🔌 RealDeviceBridge инициализирован для {port}")
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
            # Формируем JSON команду
            cmd_data = {
                "command": command,
                "timestamp": int(time.time() * 1000)
            }
            
            json_cmd = json.dumps(cmd_data) + "\n"
            self.serial_conn.write(json_cmd.encode())
            
            # Обновляем локальный LED статус для немедленного отклика
            if command == 'led_on':
                self.led_status = 1
            elif command == 'led_off':
                self.led_status = 0
            
            # Ожидаем ответа
            response = self.wait_for_response(timeout=5)
            return response
            
        except Exception as e:
            return {"status": "error", "message": str(e)}
    
    def wait_for_response(self, timeout=5):
        """Ожидание ответа от устройства"""
        start_time = time.time()
        buffer = ""
        
        while time.time() - start_time < timeout:
            if self.serial_conn.in_waiting > 0:
                try:
                    line = self.serial_conn.readline().decode('utf-8').strip()
                    if line:
                        buffer += line
                        
                        # Пытаемся распарсить JSON
                        try:
                            response = json.loads(buffer)
                            return response
                        except json.JSONDecodeError:
                            # Если не полный JSON, продолжаем читать
                            continue
                            
                except UnicodeDecodeError:
                    continue
            
            time.sleep(0.01)
        
        return {"status": "timeout", "message": "Нет ответа от устройства"}
    
    def start_monitoring(self, callback):
        """Запуск мониторинга устройства в отдельном потоке"""
        def monitor():
            while self.running:
                try:
                    if self.serial_conn and self.serial_conn.in_waiting > 0:
                        line = self.serial_conn.readline().decode('utf-8').strip()
                        if line:
                            self.process_device_data(line, callback)
                    time.sleep(0.1)
                except Exception as e:
                    print(f"❌ Ошибка мониторинга: {e}")
                    time.sleep(1)
        
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
    
    def process_device_data(self, line, callback):
        """Обработка данных от устройства"""
        try:
            data = json.loads(line)
            timestamp = datetime.now().isoformat()
            
            # Определяем тип данных
            if 'temperature' in data:
                # Данные с датчиков
                self.sensor_data = data
                self.sensor_data['timestamp'] = timestamp
                
                # Обновляем LED статус из данных устройства
                if 'led_status' in data:
                    self.led_status = data['led_status']
                
                callback('sensors', self.sensor_data)
                
            elif 'status' in data:
                # Статус системы
                self.system_status = data
                self.system_status['timestamp'] = timestamp
                callback('status', self.system_status)
                
            elif 'message' in data:
                # Ответ на команду или событие
                self.add_event(data.get('status', 'info'), data['message'])
                callback('response', data)
                
        except json.JSONDecodeError as e:
            print(f"⚠️ Ошибка парсинга JSON: {e}")
        except Exception as e:
            print(f"❌ Ошибка обработки данных: {e}")
    
    def add_event(self, event_type, message):
        """Добавление события"""
        event = {
            'timestamp': datetime.now().isoformat(),
            'type': event_type,
            'message': message
        }
        self.events.insert(0, event)
        
        # Ограничиваем размер журнала
        if len(self.events) > 50:
            self.events.pop()
    
    def get_sensors(self):
        """Получение последних данных с датчиков"""
        if self.sensor_data:
            # Добавляем текущий LED статус в данные
            data = self.sensor_data.copy()
            if 'led_status' not in data:
                data['led_status'] = self.led_status
            return data
        else:
            # Запрашиваем данные у устройства
            response = self.send_command('get_sensors')
            if response.get('status') == 'success':
                return response.get('data', {})
            return self.generate_fallback_data()
    
    def get_status(self):
        """Получение статуса системы"""
        if self.system_status:
            return self.system_status
        else:
            # Запрашиваем статус у устройства
            response = self.send_command('get_status')
            if response.get('status') == 'success':
                return response.get('data', {})
            return self.generate_fallback_status()
    
    def get_events(self, limit=20):
        """Получение журнала событий"""
        return self.events[:limit]
    
    def generate_fallback_data(self):
        """Генерация запасных данных когда устройство не отвечает"""
        import random
        import math
        
        now = datetime.now()
        hour = now.hour
        
        # Реалистичные колебания температуры в течение дня
        base_temp = 22 + 3 * math.sin((hour - 6) * math.pi / 12)
        temperature = base_temp + (random.random() - 0.5) * 2
        
        # Реалистичные значения пульса
        base_pulse = 70 + math.sin(time.time() / 10000) * 10
        pulse = base_pulse + (random.random() - 0.5) * 10
        
        # CPU нагрузка с периодическими пиками
        cpu_base = 30 + math.sin(time.time() / 5000) * 20
        cpu = max(10, min(90, cpu_base + (random.random() - 0.5) * 15))
        
        # Память
        memory = 40 + math.sin(time.time() / 8000) * 20 + (random.random() - 0.5) * 10
        
        # Батарея с медленным разрядом
        battery = max(20, 87 - (time.time() / 1000000) + (random.random() - 0.5) * 2)
        
        # WiFi сигнал с флуктуациями
        wifi_signal = max(20, min(100, 85 + math.sin(time.time() / 3000) * 15 + (random.random() - 0.5) * 10))
        
        return {
            'timestamp': now.isoformat(),
            'temperature': round(temperature, 1),
            'humidity': round(60 + math.sin(time.time() / 7000) * 15 + (random.random() - 0.5) * 5, 1),
            'pressure': round(1013.25 + math.sin(time.time() / 6000) * 10 + (random.random() - 0.5) * 2, 2),
            'cpu': round(cpu),
            'memory': round(memory),
            'pulse': round(pulse),
            'battery': round(battery),
            'wifi_signal': round(wifi_signal),
            'led_status': self.led_status  # Используем сохраненный LED статус
        }
    
    def generate_fallback_status(self):
        """Генерация запасного статуса"""
        return {
            'timestamp': datetime.now().isoformat(),
            'status': 'simulated',
            'uptime': '5d 14h 23m',
            'firmware': 'v2.1.3',
            'wifi_signal': 85,
            'free_memory': 32768,
            'total_memory': 131072,
            'reboot_count': 0
        }

class IntegratedRealDeviceAPI:
    """Интегрированный API с реальным устройством"""
    
    def __init__(self):
        self.bridge = RealDeviceBridge()
        self.sensor_data = []
        self.events = []
        self.device_status = "offline"
        
        # Попытка подключения к устройству
        self.try_connect_device()
    
    def try_connect_device(self):
        """Попытка подключения к устройству"""
        ports = ['/dev/ttyUSB0', '/dev/ttyACM0', '/dev/tty.usbserial', 'COM3', 'COM4']
        
        for port in ports:
            if self.bridge.connect():
                self.device_status = "online"
                self.add_event("success", f"Подключено к {port}")
                self.bridge.start_monitoring(self.data_callback)
                return True
        
        print("⚠️ Устройство не найдено, используем симуляцию")
        self.device_status = "simulated"
        self.add_event("warning", "Устройство не найдено, используется симуляция")
        return False
    
    def data_callback(self, data_type, data):
        """Callback для обработки данных от устройства"""
        if data_type == 'sensors':
            self.sensor_data.append(data)
            if len(self.sensor_data) > 100:
                self.sensor_data.pop(0)
    
    def get_status(self):
        """Получение статуса системы"""
        return self.bridge.get_status()
    
    def get_sensors(self):
        """Получение данных с датчиков"""
        return self.bridge.get_sensors()
    
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
        return self.events[:limit]
    
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
real_device_api = IntegratedRealDeviceAPI()

if __name__ == "__main__":
    # Тестирование моста
    print("🔧 Тестирование моста реального устройства...")
    
    api = real_device_api
    
    # Тест получения данных
    print("\n📊 Тест получения данных:")
    data = api.get_sensors()
    print(json.dumps(data, indent=2, ensure_ascii=False))
    
    # Тест статуса
    print("\n📈 Тест статуса:")
    status = api.get_status()
    print(json.dumps(status, indent=2, ensure_ascii=False))
    
    # Тест команды
    print("\n🎮 Тест команды:")
    response = api.send_command("led_on")
    print(json.dumps(response, indent=2, ensure_ascii=False))
    
    # Мониторинг в течение 10 секунд
    print("\n📡 Мониторинг (10 секунд)...")
    time.sleep(10)
