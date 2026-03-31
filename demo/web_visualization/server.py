#!/usr/bin/env python3
"""
Простой HTTP сервер для демо-панели Secure IoT Sensor Node
Запуск: python server.py
"""

import http.server
import socketserver
import os
import json
from urllib.parse import urlparse, parse_qs
import threading
import time

class IoTRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=".", **kwargs)
    
    def do_GET(self):
        parsed_path = urlparse(self.path)
        
        # API endpoints
        if parsed_path.path == '/api/status':
            self.send_api_response(self.get_status_data())
        elif parsed_path.path == '/api/sensors':
            self.send_api_response(self.get_sensors_data())
        elif parsed_path.path == '/api/events':
            self.send_api_response(self.get_events_data())
        elif parsed_path.path == '/api/command':
            self.send_api_response({"error": "Use POST for commands"})
        else:
            # Serve static files
            super().do_GET()
    
    def do_POST(self):
        parsed_path = urlparse(self.path)
        
        if parsed_path.path == '/api/command':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            command_data = json.loads(post_data.decode('utf-8'))
            
            response = self.process_command(command_data.get('command'))
            self.send_api_response(response)
        else:
            self.send_error(404)
    
    def send_api_response(self, data):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode('utf-8'))
    
    def get_status_data(self):
        """Статус устройства"""
        return {
            "status": "online",
            "uptime": "5d 14h 23m",
            "firmware": "v2.1.3",
            "last_update": "2024-03-31T14:23:15Z",
            "security": {
                "encryption": "active",
                "authentication": "ok",
                "firewall": "enabled"
            },
            "network": {
                "ip": "192.168.1.42",
                "mac": "A4:B5:C6:D7:E8:F9",
                "wifi_signal": 85,
                "connected": True
            }
        }
    
    def get_sensors_data(self):
        """Данные с датчиков"""
        import random
        import datetime
        
        current_time = datetime.datetime.now()
        hour = current_time.hour
        base_temp = 22 + (hour - 6) * 0.25  # Суточные колебания
        
        return {
            "timestamp": current_time.isoformat(),
            "temperature": round(base_temp + random.uniform(-2, 2), 1),
            "humidity": round(45 + random.uniform(-10, 10), 1),
            "pressure": round(1013 + random.uniform(-5, 5), 1),
            "cpu": round(30 + random.uniform(0, 40), 1),
            "memory": round(40 + random.uniform(0, 30), 1),
            "battery": round(max(20, 95 - (time.time() / 100000) % 80), 1),
            "pulse": round(65 + random.uniform(-10, 15)),
            "led_status": random.choice(['on', 'off'])
        }
    
    def get_events_data(self):
        """Журнал событий"""
        return [
            {
                "timestamp": "2024-03-31T14:23:15Z",
                "type": "info",
                "message": "Устройство успешно подключено к сети"
            },
            {
                "timestamp": "2024-03-31T14:22:48Z",
                "type": "success",
                "message": "OTA обновление завершено успешно"
            },
            {
                "timestamp": "2024-03-31T14:20:12Z",
                "type": "warning",
                "message": "Температура превысила пороговое значение"
            },
            {
                "timestamp": "2024-03-31T14:15:30Z",
                "type": "info",
                "message": "Проверка безопасности пройдена"
            }
        ]
    
    def process_command(self, command):
        """Обработка команд"""
        commands = {
            'led_on': {
                "status": "success",
                "message": "LED индикатор включен",
                "timestamp": time.time()
            },
            'led_off': {
                "status": "success", 
                "message": "LED индикатор выключен",
                "timestamp": time.time()
            },
            'reboot': {
                "status": "success",
                "message": "Команда перезагрузки отправлена",
                "timestamp": time.time()
            },
            'calibrate': {
                "status": "success",
                "message": "Калибровка датчиков начата",
                "timestamp": time.time()
            }
        }
        
        return commands.get(command, {
            "status": "error",
            "message": f"Неизвестная команда: {command}",
            "timestamp": time.time()
        })
    
    def log_message(self, format, *args):
        """Логирование запросов"""
        print(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] {format % args}")

def run_server():
    """Запуск сервера"""
    PORT = 8080
    
    print(f"🚀 Запуск демо-сервера Secure IoT Sensor Node")
    print(f"📊 Панель управления: http://localhost:{PORT}")
    print(f"🔗 API endpoints:")
    print(f"   GET  http://localhost:{PORT}/api/status")
    print(f"   GET  http://localhost:{PORT}/api/sensors") 
    print(f"   GET  http://localhost:{PORT}/api/events")
    print(f"   POST http://localhost:{PORT}/api/command")
    print(f"⏹️  Нажмите Ctrl+C для остановки")
    print("-" * 50)
    
    with socketserver.TCPServer(("", PORT), IoTRequestHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n🛑 Сервер остановлен")

if __name__ == "__main__":
    run_server()
