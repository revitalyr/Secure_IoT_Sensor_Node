#!/usr/bin/env python3
"""
API сервер для Secure IoT Sensor Node с эмуляцией устройства
Предоставляет REST API для веб-интерфейса
"""

from flask import Flask, jsonify, request
from flask_cors import CORS
import time
import threading
import random
import math
from datetime import datetime, timedelta
import json

# Импорт моста для реального устройства
try:
    from real_device_bridge import real_device_api
    REAL_DEVICE_AVAILABLE = True
    DEVICE_BRIDGE_AVAILABLE = False
    print("🔌 Мост реального устройства доступен")
except ImportError:
    REAL_DEVICE_AVAILABLE = False
    try:
        from device_bridge import integrated_api
        DEVICE_BRIDGE_AVAILABLE = True
        print("🔌 Эмулятор устройства доступен")
    except ImportError:
        DEVICE_BRIDGE_AVAILABLE = False
        print("⚠️ Мост устройства недоступен, используем симуляцию")

app = Flask(__name__)
CORS(app)

# Эмуляция состояния устройства (fallback)
device_state = {
    'status': 'online',
    'uptime_start': datetime.now() - timedelta(days=5, hours=14, minutes=23),
    'firmware': 'v2.1.3',
    'led_status': 'off',
    'last_calibration': datetime.now() - timedelta(hours=2),
    'battery_level': 87.0,
    'reboot_count': 0
}

# История данных для графиков
sensor_history = {
    'temperature': [],
    'cpu': [],
    'memory': [],
    'timestamps': []
}

# Журнал событий
event_log = [
    {
        'timestamp': '2024-03-31T14:23:15Z',
        'type': 'info',
        'message': 'Устройство успешно подключено к сети'
    },
    {
        'timestamp': '2024-03-31T14:22:48Z', 
        'type': 'success',
        'message': 'OTA обновление завершено успешно'
    },
    {
        'timestamp': '2024-03-31T14:20:12Z',
        'type': 'warning', 
        'message': 'Температура превысила пороговое значение'
    }
]

def generate_sensor_data():
    """Генерация реалистичных данных датчиков"""
    now = datetime.now()
    hour = now.hour
    
    # Температура с суточными колебаниями (минимум в 6 утра, максимум в 18:00)
    base_temp = 22 + 3 * math.sin((hour - 6) * math.pi / 12)
    temperature = base_temp + random.uniform(-1, 1)
    
    # CPU нагрузка с периодическими пиками
    cpu_base = 35 + 15 * math.sin(time.time() / 300)  # 5-минутные циклы
    cpu = max(10, min(80, cpu_base + random.uniform(-10, 10)))
    
    # Память с плавными изменениями
    memory = 45 + 20 * math.sin(time.time() / 600) + random.uniform(-5, 5)
    memory = max(20, min(90, memory))
    
    # Влажность (связана с температурой)
    humidity = 60 - (temperature - 20) * 2 + random.uniform(-5, 5)
    humidity = max(30, min(80, humidity))
    
    # Давление с медленными изменениями
    pressure = 1013 + 5 * math.sin(time.time() / 3600) + random.uniform(-2, 2)
    
    # Пульс с вариациями
    pulse = 70 + 10 * math.sin(time.time() / 30) + random.uniform(-5, 5)
    pulse = max(55, min(90, pulse))
    
    # Батарея (медленно разряжается)
    device_state['battery_level'] = max(20, device_state['battery_level'] - 0.001)
    
    return {
        'timestamp': now.isoformat(),
        'temperature': round(temperature, 1),
        'humidity': round(humidity, 1),
        'pressure': round(pressure, 1),
        'cpu': round(cpu, 1),
        'memory': round(memory, 1),
        'pulse': round(pulse),
        'battery': round(device_state['battery_level'], 1),
        'led_status': device_state['led_status']
    }

def update_sensor_history():
    """Обновление истории данных датчиков"""
    data = generate_sensor_data()
    
    # Добавляем новые данные
    sensor_history['temperature'].append(data['temperature'])
    sensor_history['cpu'].append(data['cpu'])
    sensor_history['memory'].append(data['memory'])
    sensor_history['timestamps'].append(data['timestamp'])
    
    # Ограничиваем историю последними 100 точками
    max_points = 100
    if len(sensor_history['timestamps']) > max_points:
        for key in sensor_history:
            sensor_history[key] = sensor_history[key][-max_points:]
    
    return data

def get_uptime():
    """Расчет времени работы"""
    uptime = datetime.now() - device_state['uptime_start']
    days = uptime.days
    hours = uptime.seconds // 3600
    minutes = (uptime.seconds % 3600) // 60
    return f"{days}d {hours}h {minutes}m"

def add_event(event_type, message):
    """Добавление события в журнал"""
    event = {
        'timestamp': datetime.now().isoformat(),
        'type': event_type,
        'message': message
    }
    event_log.insert(0, event)
    
    # Ограничиваем журнал последними 50 событиями
    if len(event_log) > 50:
        event_log.pop()

@app.route('/api/status', methods=['GET'])
def get_status():
    """Статус устройства"""
    if REAL_DEVICE_AVAILABLE:
        # Используем реальное устройство
        bridge_status = real_device_api.get_status()
        
        return jsonify({
            'status': bridge_status.get('status', 'unknown'),
            'uptime': bridge_status.get('uptime', 'Unknown'),
            'firmware': bridge_status.get('firmware', 'v2.1.3'),
            'last_update': bridge_status.get('timestamp', datetime.now().isoformat()),
            'device_connected': real_device_api.bridge.device_connected,
            'port': real_device_api.bridge.port if real_device_api.bridge.device_connected else None,
            'security': {
                'encryption': 'active',
                'authentication': 'ok',
                'firewall': 'enabled',
                'last_scan': datetime.now().isoformat()
            },
            'network': {
                'ip': '192.168.1.42',
                'mac': 'A4:B5:C6:D7:E8:F9',
                'wifi_signal': bridge_status.get('wifi_signal', 85),
                'connected': True,
                'gateway': '192.168.1.1',
                'dns': '8.8.8.8'
            },
            'system': {
                'reboot_count': bridge_status.get('reboot_count', 0),
                'free_memory': bridge_status.get('free_memory', 32768),
                'total_memory': bridge_status.get('total_memory', 131072)
            }
        })
    elif DEVICE_BRIDGE_AVAILABLE:
        # Используем эмулятор устройства
        bridge_status = integrated_api.get_status()
        
        return jsonify({
            'status': bridge_status['status'],
            'uptime': bridge_status.get('uptime', 'Unknown'),
            'firmware': bridge_status.get('firmware', 'v2.1.3'),
            'last_update': datetime.now().isoformat(),
            'device_connected': bridge_status.get('device_connected', False),
            'port': bridge_status.get('port'),
            'security': {
                'encryption': 'active',
                'authentication': 'ok',
                'firewall': 'enabled',
                'last_scan': datetime.now().isoformat()
            },
            'network': {
                'ip': '192.168.1.42',
                'mac': 'A4:B5:C6:D7:E8:F9',
                'wifi_signal': random.randint(75, 95),
                'connected': True,
                'gateway': '192.168.1.1',
                'dns': '8.8.8.8'
            },
            'system': {
                'reboot_count': device_state['reboot_count'],
                'last_calibration': device_state['last_calibration'].isoformat(),
                'free_memory': round(random.uniform(20, 40), 1),
                'total_memory': 128.0
            }
        })
    else:
        # Fallback на симуляцию
        return jsonify({
            'status': device_state['status'],
            'uptime': get_uptime(),
            'firmware': device_state['firmware'],
            'last_update': datetime.now().isoformat(),
            'device_connected': False,
            'port': None,
            'security': {
                'encryption': 'active',
                'authentication': 'ok',
                'firewall': 'enabled',
                'last_scan': datetime.now().isoformat()
            },
            'network': {
                'ip': '192.168.1.42',
                'mac': 'A4:B5:C6:D7:E8:F9',
                'wifi_signal': random.randint(75, 95),
                'connected': True,
                'gateway': '192.168.1.1',
                'dns': '8.8.8.8'
            },
            'system': {
                'reboot_count': device_state['reboot_count'],
                'last_calibration': device_state['last_calibration'].isoformat(),
                'free_memory': round(random.uniform(20, 40), 1),
                'total_memory': 128.0
            }
        })

@app.route('/api/sensors', methods=['GET'])
def get_sensors():
    """Текущие данные с датчиков"""
    if REAL_DEVICE_AVAILABLE:
        # Используем реальные данные с устройства
        data = real_device_api.get_sensors()
        return jsonify(data)
    elif DEVICE_BRIDGE_AVAILABLE:
        # Используем эмулятор устройства
        data = integrated_api.get_sensors()
        return jsonify(data)
    else:
        # Fallback на симуляцию
        data = update_sensor_history()
        return jsonify(data)

@app.route('/api/sensors/history', methods=['GET'])
def get_sensor_history():
    """История данных датчиков для графиков"""
    hours = request.args.get('hours', 24, type=int)
    
    # Фильтрация по времени
    cutoff_time = datetime.now() - timedelta(hours=hours)
    filtered_data = {
        'timestamps': [],
        'temperature': [],
        'cpu': [],
        'memory': []
    }
    
    for i, timestamp in enumerate(sensor_history['timestamps']):
        dt = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
        if dt >= cutoff_time:
            filtered_data['timestamps'].append(timestamp)
            filtered_data['temperature'].append(sensor_history['temperature'][i])
            filtered_data['cpu'].append(sensor_history['cpu'][i])
            filtered_data['memory'].append(sensor_history['memory'][i])
    
    return jsonify(filtered_data)

@app.route('/api/events', methods=['GET'])
def get_events():
    """Журнал событий"""
    limit = request.args.get('limit', 20, type=int)
    return jsonify(event_log[:limit])

@app.route('/api/command', methods=['POST'])
def process_command():
    """Обработка команд устройству"""
    command_data = request.get_json()
    command = command_data.get('command')
    
    if REAL_DEVICE_AVAILABLE:
        # Отправляем команду реальному устройству
        response = real_device_api.send_command(command)
        return jsonify(response)
    elif DEVICE_BRIDGE_AVAILABLE:
        # Отправляем команду эмулятору
        response = integrated_api.send_command(command)
        return jsonify(response)
    else:
        # Fallback на симуляцию
        responses = {
            'led_on': {
                'status': 'success',
                'message': 'LED индикатор включен',
                'timestamp': datetime.now().isoformat()
            },
            'led_off': {
                'status': 'success',
                'message': 'LED индикатор выключен', 
                'timestamp': datetime.now().isoformat()
            },
            'reboot': {
                'status': 'success',
                'message': 'Команда перезагрузки отправлена',
                'timestamp': datetime.now().isoformat()
            },
            'calibrate': {
                'status': 'success',
                'message': 'Калибровка датчиков начата',
                'timestamp': datetime.now().isoformat()
            },
            'sync_time': {
                'status': 'success',
                'message': 'Время синхронизировано',
                'timestamp': datetime.now().isoformat()
            }
        }
        
        if command == 'led_on':
            device_state['led_status'] = 'on'
            add_event('info', f"LED включен через веб-интерфейс")
        elif command == 'led_off':
            device_state['led_status'] = 'off'
            add_event('info', f"LED выключен через веб-интерфейс")
        elif command == 'reboot':
            device_state['reboot_count'] += 1
            device_state['uptime_start'] = datetime.now() + timedelta(seconds=5)  # Симуляция перезагрузки
            add_event('warning', f"Перезагрузка инициирована через веб-интерфейс")
        elif command == 'calibrate':
            device_state['last_calibration'] = datetime.now()
            add_event('info', f"Калибровка датчиков выполнена")
        elif command == 'sync_time':
            add_event('info', f"Время синхронизировано с NTP сервером")
        else:
            return jsonify({
                'status': 'error',
                'message': f'Неизвестная команда: {command}',
                'timestamp': datetime.now().isoformat()
            }), 400
        
        response = responses.get(command, {
            'status': 'error',
            'message': f'Неизвестная команда: {command}',
            'timestamp': datetime.now().isoformat()
        })
        
        return jsonify(response)

@app.route('/api/health', methods=['GET'])
def health_check():
    """Проверка здоровья API"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'version': '1.0.0',
        'device_status': device_state['status']
    })

@app.route('/api/stats', methods=['GET'])
def get_statistics():
    """Статистика устройства"""
    if not sensor_history['timestamps']:
        return jsonify({'error': 'No data available'}), 404
    
    temp_data = sensor_history['temperature']
    cpu_data = sensor_history['cpu']
    memory_data = sensor_history['memory']
    
    return jsonify({
        'period': 'Last 100 readings',
        'temperature': {
            'current': temp_data[-1] if temp_data else 0,
            'min': min(temp_data) if temp_data else 0,
            'max': max(temp_data) if temp_data else 0,
            'avg': round(sum(temp_data) / len(temp_data), 1) if temp_data else 0
        },
        'cpu': {
            'current': cpu_data[-1] if cpu_data else 0,
            'min': min(cpu_data) if cpu_data else 0,
            'max': max(cpu_data) if cpu_data else 0,
            'avg': round(sum(cpu_data) / len(cpu_data), 1) if cpu_data else 0
        },
        'memory': {
            'current': memory_data[-1] if memory_data else 0,
            'min': min(memory_data) if memory_data else 0,
            'max': max(memory_data) if memory_data else 0,
            'avg': round(sum(memory_data) / len(memory_data), 1) if memory_data else 0
        },
        'uptime': get_uptime(),
        'total_events': len(event_log),
        'last_event': event_log[0] if event_log else None
    })

# Фоновая задача для генерации случайных событий
def background_events():
    """Генерация фоновых событий"""
    while True:
        time.sleep(random.randint(30, 120))  # События каждые 30-120 секунд
        
        event_types = [
            ('info', [
                'Периодическая проверка системы завершена',
                'Резервное копирование данных создано',
                'Синхронизация с сервером успешна',
                'Проверка безопасности пройдена'
            ]),
            ('warning', [
                'Обнаружена повышенная нагрузка на CPU',
                'Температура приближается к пороговому значению',
                'Слабый сигнал WiFi сети'
            ]),
            ('success', [
                'Автоматическая калибровка выполнена',
                'Обновление конфигурации успешно',
                'Оптимизация памяти завершена'
            ])
        ]
        
        event_type, messages = random.choice(event_types)
        message = random.choice(messages)
        
        # Случайные события с низкой вероятностью
        if random.random() < 0.3:  # 30% шанс
            add_event(event_type, message)

# Запуск фоновой задачи
def start_background_tasks():
    """Запуск фоновых задач"""
    event_thread = threading.Thread(target=background_events, daemon=True)
    event_thread.start()
    
    print("🔧 Фоновые задачи запущены")
    print("📊 API сервер готов к работе")

if __name__ == '__main__':
    start_background_tasks()
    
    print("🚀 Запуск API сервера Secure IoT Sensor Node")
    print("📡 API endpoints:")
    print("   GET  http://localhost:5000/api/status")
    print("   GET  http://localhost:5000/api/sensors")
    print("   GET  http://localhost:5000/api/sensors/history")
    print("   GET  http://localhost:5000/api/events")
    print("   POST http://localhost:5000/api/command")
    print("   GET  http://localhost:5000/api/health")
    print("   GET  http://localhost:5000/api/stats")
    print("⏹️  Нажмите Ctrl+C для остановки")
    print("-" * 50)
    
    app.run(host='0.0.0.0', port=5000, debug=False)
