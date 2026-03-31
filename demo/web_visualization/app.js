// Инициализация Lucide иконок
lucide.createIcons();

// Глобальные переменные
let simulationRunning = true;
let tempChart, performanceChart;
let sensorData = {
    temperature: [],
    timestamps: [],
    cpu: [],
    memory: []
};

// Инициализация графиков
function initCharts() {
    // График температуры
    const tempCtx = document.getElementById('tempChart').getContext('2d');
    tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Температура (°C)',
                data: [],
                borderColor: 'rgb(59, 130, 246)',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                tension: 0.4,
                fill: true,
                pointRadius: 2,
                pointHoverRadius: 4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                intersect: false,
                mode: 'index'
            },
            plugins: {
                legend: {
                    display: false
                },
                tooltip: {
                    backgroundColor: 'rgba(0, 0, 0, 0.8)',
                    titleColor: '#fff',
                    bodyColor: '#fff',
                    borderColor: 'rgb(59, 130, 246)',
                    borderWidth: 1
                }
            },
            scales: {
                x: {
                    display: true,
                    grid: {
                        display: false
                    },
                    ticks: {
                        maxRotation: 45,
                        minRotation: 45,
                        autoSkip: true,
                        maxTicksLimit: 8
                    }
                },
                y: {
                    display: true,
                    beginAtZero: false,
                    min: 15,
                    max: 35,
                    grid: {
                        color: 'rgba(0, 0, 0, 0.05)'
                    },
                    ticks: {
                        stepSize: 5
                    }
                }
            },
            animation: {
                duration: 0
            }
        }
    });

    // График производительности
    const perfCtx = document.getElementById('performanceChart').getContext('2d');
    performanceChart = new Chart(perfCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'CPU (%)',
                data: [],
                borderColor: 'rgb(168, 85, 247)',
                backgroundColor: 'rgba(168, 85, 247, 0.1)',
                tension: 0.4,
                fill: true,
                pointRadius: 2,
                pointHoverRadius: 4
            }, {
                label: 'Память (%)',
                data: [],
                borderColor: 'rgb(34, 197, 94)',
                backgroundColor: 'rgba(34, 197, 94, 0.1)',
                tension: 0.4,
                fill: true,
                pointRadius: 2,
                pointHoverRadius: 4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                intersect: false,
                mode: 'index'
            },
            plugins: {
                legend: {
                    position: 'top',
                    labels: {
                        usePointStyle: true,
                        padding: 15
                    }
                },
                tooltip: {
                    backgroundColor: 'rgba(0, 0, 0, 0.8)',
                    titleColor: '#fff',
                    bodyColor: '#fff',
                    borderColor: 'rgb(168, 85, 247)',
                    borderWidth: 1
                }
            },
            scales: {
                x: {
                    display: true,
                    grid: {
                        display: false
                    },
                    ticks: {
                        maxRotation: 45,
                        minRotation: 45,
                        autoSkip: true,
                        maxTicksLimit: 8
                    }
                },
                y: {
                    display: true,
                    beginAtZero: true,
                    max: 100,
                    grid: {
                        color: 'rgba(0, 0, 0, 0.05)'
                    },
                    ticks: {
                        stepSize: 20
                    }
                }
            },
            animation: {
                duration: 0
            }
        }
    });
}

// Генерация случайных данных датчиков
function generateSensorData() {
    const now = new Date();
    const hour = now.getHours();
    
    // Реалистичные колебания температуры в течение дня
    const baseTemp = 22 + 3 * Math.sin((hour - 6) * Math.PI / 12);
    const temperature = baseTemp + (Math.random() - 0.5) * 2;
    
    // Реалистичные значения пульса
    const basePulse = 70 + Math.sin(Date.now() / 10000) * 10;
    const pulse = basePulse + (Math.random() - 0.5) * 10;
    
    // CPU нагрузка с периодическими пиками
    const cpuBase = 30 + Math.sin(Date.now() / 5000) * 20;
    const cpu = Math.max(10, Math.min(90, cpuBase + (Math.random() - 0.5) * 15));
    
    // Память
    const memory = 40 + Math.sin(Date.now() / 8000) * 20 + (Math.random() - 0.5) * 10;
    
    // Батарея с медленным разрядом
    const battery = Math.max(20, 87 - (Date.now() / 1000000) + (Math.random() - 0.5) * 2);
    
    // WiFi сигнал с флуктуациями
    const wifiSignal = Math.max(20, Math.min(100, 85 + Math.sin(Date.now() / 3000) * 15 + (Math.random() - 0.5) * 10));
    
    // LED статус (меняется редко)
    if (!window.currentLEDStatus) {
        window.currentLEDStatus = 0;
    }
    if (Math.random() < 0.01) { // 1% шанс смены статуса
        window.currentLEDStatus = window.currentLEDStatus === 1 ? 0 : 1;
    }
    
    const timeString = now.toLocaleTimeString('ru-RU', { 
        hour: '2-digit', 
        minute: '2-digit', 
        second: '2-digit' 
    });
    
    return {
        timestamp: timeString,
        temperature: temperature.toFixed(1),
        cpu: cpu.toFixed(0),
        memory: memory.toFixed(0),
        pulse: Math.round(pulse),
        battery: Math.round(battery),
        wifi_signal: Math.round(wifiSignal),
        led_status: window.currentLEDStatus
    };
}

// Обновление данных датчиков
function updateSensorData(data) {
    // Обновление карточек датчиков
    document.getElementById('tempValue').textContent = `${data.temperature}°C`;
    document.getElementById('cpuValue').textContent = `${data.cpu}%`;
    document.getElementById('pulseValue').textContent = `${data.pulse} bpm`;
    document.getElementById('batteryValue').textContent = `${data.battery}%`;
    
    // Обновление системного статуса
    if (data.led_status !== undefined) {
        updateLEDIndicator(data.led_status === 1);
    }
    
    // Обновление WiFi сигнала (если есть)
    if (data.wifi_signal !== undefined) {
        updateWiFiIndicator(data.wifi_signal);
    }
    
    // Обновление графиков
    updateCharts(data);
    
    // Обновление времени работы
    updateUptime();
    
    // Добавление событий в журнал
    if (Math.random() < 0.05) { // 5% шанс добавить событие
        addRandomEvent();
    }
}

// Обновление WiFi индикатора
function updateWiFiIndicator(signalStrength) {
    const wifiSignal = document.getElementById('wifiSignal');
    const wifiSignalText = document.getElementById('wifiSignalText');
    
    // Обновляем текст
    wifiSignalText.textContent = `${signalStrength}%`;
    
    // Обновляем прогресс-бар
    wifiSignal.style.width = `${signalStrength}%`;
    
    // Обновляем цвет в зависимости от качество сигнала
    if (signalStrength >= 70) {
        wifiSignal.className = 'bg-green-500 h-2 rounded-full transition-all duration-500';
    } else if (signalStrength >= 40) {
        wifiSignal.className = 'bg-yellow-500 h-2 rounded-full transition-all duration-500';
    } else {
        wifiSignal.className = 'bg-red-500 h-2 rounded-full transition-all duration-500';
    }
}

// Обновление графиков
function updateCharts(data) {
    // Ограничиваем количество точек для скроллинга
    const maxPoints = 30; // Показываем последние 30 точек
    
    // Обновление графика температуры
    if (tempChart.data.labels.length >= maxPoints) {
        tempChart.data.labels.shift();
        tempChart.data.datasets[0].data.shift();
    }
    tempChart.data.labels.push(data.timestamp);
    tempChart.data.datasets[0].data.push(parseFloat(data.temperature));
    tempChart.update('none'); // Без анимации для плавности
    
    // Обновление графика производительности
    if (performanceChart.data.labels.length >= maxPoints) {
        performanceChart.data.labels.shift();
        performanceChart.data.datasets[0].data.shift();
        performanceChart.data.datasets[1].data.shift();
    }
    performanceChart.data.labels.push(data.timestamp);
    performanceChart.data.datasets[0].data.push(parseFloat(data.cpu));
    performanceChart.data.datasets[1].data.push(parseFloat(data.memory));
    performanceChart.update('none'); // Без анимации для плавности
}

// Обновление uptime
function updateUptime() {
    // Используем реальное время запуска демо (сессия браузера)
    if (!window.demoStartTime) {
        window.demoStartTime = new Date() - (5 * 24 * 60 * 60 * 1000) // 5 дней назад для реалистичности
    }
    
    const now = new Date();
    const diff = now - window.demoStartTime;
    
    const days = Math.floor(diff / (1000 * 60 * 60 * 24));
    const hours = Math.floor((diff % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
    const minutes = Math.floor((diff % (1000 * 60 * 60)) / (1000 * 60));
    
    document.getElementById('uptime').textContent = `${days}д ${hours}ч ${minutes}м`;
}

// Добавление случайного события
function addRandomEvent() {
    const events = [
        { type: 'INFO', message: 'Датчик температуры откалиброван', color: 'blue' },
        { type: 'SUCCESS', message: 'Резервное копирование создано', color: 'green' },
        { type: 'WARNING', message: 'Обнаружена высокая нагрузка на CPU', color: 'yellow' },
        { type: 'INFO', message: 'Проверка безопасности пройдена', color: 'blue' },
        { type: 'SUCCESS', message: 'Синхронизация с сервером завершена', color: 'green' }
    ];
    
    const event = events[Math.floor(Math.random() * events.length)];
    const time = new Date().toLocaleTimeString('ru-RU');
    
    const eventLog = document.getElementById('eventLog');
    const eventDiv = document.createElement('div');
    eventDiv.className = 'flex items-center space-x-3 text-sm';
    eventDiv.innerHTML = `
        <span class="text-gray-500">${time}</span>
        <span class="px-2 py-1 bg-${event.color}-100 text-${event.color}-800 rounded text-xs">${event.type}</span>
        <span>${event.message}</span>
    `;
    
    eventLog.insertBefore(eventDiv, eventLog.firstChild);
    
    // Ограничиваем количество событий
    while (eventLog.children.length > 10) {
        eventLog.removeChild(eventLog.lastChild);
    }
}

// Отправка команды на устройство
function sendCommand(command) {
    const time = new Date().toLocaleTimeString('ru-RU');
    const eventLog = document.getElementById('eventLog');
    
    const commandMessages = {
        'led_on': 'LED включен',
        'led_off': 'LED выключен',
        'reboot': 'Перезагрузка инициирована',
        'calibrate': 'Калибровка датчиков запущена',
        'sync_time': 'Синхронизация времени'
    };
    
    const eventDiv = document.createElement('div');
    eventDiv.className = 'flex items-center space-x-3 text-sm';
    eventDiv.innerHTML = `
        <span class="text-gray-500">${time}</span>
        <span class="px-2 py-1 bg-purple-100 text-purple-800 rounded text-xs">COMMAND</span>
        <span>${commandMessages[command]}</span>
    `;
    
    eventLog.insertBefore(eventDiv, eventLog.firstChild);
    
    // Обновление LED индикатора для команд LED
    if (command === 'led_on' || command === 'led_off') {
        updateLEDIndicator(command === 'led_on');
    }
    
    // Имитация ответа устройства
    setTimeout(() => {
        const responseDiv = document.createElement('div');
        responseDiv.className = 'flex items-center space-x-3 text-sm';
        responseDiv.innerHTML = `
            <span class="text-gray-500">${new Date().toLocaleTimeString('ru-RU')}</span>
            <span class="px-2 py-1 bg-green-100 text-green-800 rounded text-xs">RESPONSE</span>
            <span>Команда выполнена успешно</span>
        `;
        eventLog.insertBefore(responseDiv, eventLog.firstChild);
        
        while (eventLog.children.length > 10) {
            eventLog.removeChild(eventLog.lastChild);
        }
    }, 1000);
}

// Обновление LED индикатора
function updateLEDIndicator(isOn) {
    const ledIndicator = document.getElementById('ledIndicator');
    const ledStatusText = document.getElementById('ledStatusText');
    
    if (isOn) {
        ledIndicator.className = 'w-8 h-8 rounded-full bg-yellow-400 border-2 border-yellow-500 shadow-lg shadow-yellow-500/50 transition-all duration-300';
        ledStatusText.textContent = 'Включен';
        ledStatusText.className = 'text-sm text-green-600 font-medium';
    } else {
        ledIndicator.className = 'w-8 h-8 rounded-full bg-gray-300 border-2 border-gray-400 transition-all duration-300';
        ledStatusText.textContent = 'Выключен';
        ledStatusText.className = 'text-sm text-gray-500';
    }
}

// Переключение симуляции
function toggleSimulation() {
    simulationRunning = !simulationRunning;
    const button = document.getElementById('simButton');
    const statusIndicator = document.getElementById('statusIndicator');
    const statusText = document.getElementById('statusText');
    
    if (simulationRunning) {
        button.textContent = 'Остановить симуляцию';
        statusIndicator.className = 'w-3 h-3 rounded-full status-online';
        statusText.textContent = 'Подключено';
        startSimulation();
    } else {
        button.textContent = 'Запустить симуляцию';
        statusIndicator.className = 'w-3 h-3 rounded-full status-offline';
        statusText.textContent = 'Отключено';
    }
}

// Запуск симуляции
function startSimulation() {
    if (!simulationRunning) return;
    
    updateUI();
    setTimeout(startSimulation, 2000); // Обновление каждые 2 секунды
}

// Инициализация при загрузке страницы
document.addEventListener('DOMContentLoaded', function() {
    initCharts();
    
    // Начальное заполнение данными для красивого отображения
    for (let i = 0; i < 10; i++) {
        const data = generateSensorData();
        updateCharts(data);
    }
    
    // Запуск симуляции
    startSimulation();
});

// Обработка горячих клавиш
document.addEventListener('keydown', function(e) {
    if (e.key === ' ') {
        e.preventDefault();
        toggleSimulation();
    }
});
