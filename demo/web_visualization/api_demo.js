// API клиент для взаимодействия с сервером
class IoTApiClient {
    constructor(baseUrl = '') {
        this.baseUrl = baseUrl;
    }
    
    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const config = {
            headers: {
                'Content-Type': 'application/json',
            },
            ...options
        };
        
        try {
            const response = await fetch(url, config);
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return await response.json();
        } catch (error) {
            console.error('API request failed:', error);
            throw error;
        }
    }
    
    async getStatus() {
        return await this.request('/api/status');
    }
    
    async getSensors() {
        return await this.request('/api/sensors');
    }
    
    async getEvents() {
        return await this.request('/api/events');
    }
    
    async sendCommand(command) {
        return await this.request('/api/command', {
            method: 'POST',
            body: JSON.stringify({ command })
        });
    }
}

// Расширенная версия app.js с API интеграцией
class IoTDashboard {
    constructor() {
        this.api = new IoTApiClient();
        this.simulationMode = true;
        this.charts = {};
        this.init();
    }
    
    async init() {
        this.initCharts();
        await this.loadInitialData();
        this.startDataUpdates();
    }
    
    async loadInitialData() {
        try {
            const [status, sensors, events] = await Promise.all([
                this.api.getStatus(),
                this.api.getSensors(), 
                this.api.getEvents()
            ]);
            
            this.updateStatus(status);
            this.updateSensors(sensors);
            this.updateEvents(events);
        } catch (error) {
            console.log('Using simulation mode (API unavailable)');
            this.simulationMode = true;
            this.startSimulation();
        }
    }
    
    updateStatus(status) {
        document.getElementById('statusText').textContent = status.status === 'online' ? 'Подключено' : 'Отключено';
        document.getElementById('uptime').textContent = status.uptime;
        
        // Обновление сетевой информации
        const networkInfo = status.network;
        document.querySelector('[data-field="ip"]').textContent = networkInfo.ip;
        document.querySelector('[data-field="mac"]').textContent = networkInfo.mac;
        document.querySelector('[data-field="signal"]').textContent = `${networkInfo.wifi_signal}%`;
    }
    
    updateSensors(sensors) {
        document.getElementById('tempValue').textContent = `${sensors.temperature}°C`;
        document.getElementById('cpuValue').textContent = `${sensors.cpu}%`;
        document.getElementById('pulseValue').textContent = `${sensors.pulse} bpm`;
        document.getElementById('batteryValue').textContent = `${sensors.battery}%`;
        
        this.updateCharts(sensors);
    }
    
    updateEvents(events) {
        const eventLog = document.getElementById('eventLog');
        eventLog.innerHTML = '';
        
        events.forEach(event => {
            const eventDiv = this.createEventElement(event);
            eventLog.appendChild(eventDiv);
        });
    }
    
    createEventElement(event) {
        const div = document.createElement('div');
        div.className = 'flex items-center space-x-3 text-sm';
        
        const time = new Date(event.timestamp).toLocaleTimeString('ru-RU');
        const color = this.getEventColor(event.type);
        
        div.innerHTML = `
            <span class="text-gray-500">${time}</span>
            <span class="px-2 py-1 bg-${color}-100 text-${color}-800 rounded text-xs">${event.type.toUpperCase()}</span>
            <span>${event.message}</span>
        `;
        
        return div;
    }
    
    getEventColor(type) {
        const colors = {
            'info': 'blue',
            'success': 'green', 
            'warning': 'yellow',
            'error': 'red'
        };
        return colors[type] || 'gray';
    }
    
    async sendCommand(command) {
        try {
            const response = await this.api.sendCommand(command);
            this.addCommandEvent(command, response);
            return response;
        } catch (error) {
            this.addCommandEvent(command, { status: 'error', message: error.message });
        }
    }
    
    addCommandEvent(command, response) {
        const eventLog = document.getElementById('eventLog');
        const time = new Date().toLocaleTimeString('ru-RU');
        
        const commandDiv = document.createElement('div');
        commandDiv.className = 'flex items-center space-x-3 text-sm';
        commandDiv.innerHTML = `
            <span class="text-gray-500">${time}</span>
            <span class="px-2 py-1 bg-purple-100 text-purple-800 rounded text-xs">COMMAND</span>
            <span>Команда: ${command}</span>
        `;
        
        eventLog.insertBefore(commandDiv, eventLog.firstChild);
        
        setTimeout(() => {
            const responseDiv = document.createElement('div');
            responseDiv.className = 'flex items-center space-x-3 text-sm';
            const color = response.status === 'success' ? 'green' : 'red';
            responseDiv.innerHTML = `
                <span class="text-gray-500">${new Date().toLocaleTimeString('ru-RU')}</span>
                <span class="px-2 py-1 bg-${color}-100 text-${color}-800 rounded text-xs">${response.status.toUpperCase()}</span>
                <span>${response.message}</span>
            `;
            eventLog.insertBefore(responseDiv, eventLog.firstChild);
            
            while (eventLog.children.length > 10) {
                eventLog.removeChild(eventLog.lastChild);
            }
        }, 500);
    }
    
    initCharts() {
        // График температуры
        const tempCtx = document.getElementById('tempChart').getContext('2d');
        this.charts.temp = new Chart(tempCtx, {
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
        this.charts.performance = new Chart(perfCtx, {
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
    
    updateCharts(sensors) {
        const time = new Date().toLocaleTimeString('ru-RU');
        const maxPoints = 30; // Показываем последние 30 точек
        
        // Обновление графика температуры
        const tempChart = this.charts.temp;
        if (tempChart.data.labels.length >= maxPoints) {
            tempChart.data.labels.shift();
            tempChart.data.datasets[0].data.shift();
        }
        tempChart.data.labels.push(time);
        tempChart.data.datasets[0].data.push(sensors.temperature);
        tempChart.update('none'); // Без анимации для плавности
        
        // Обновление графика производительности
        const perfChart = this.charts.performance;
        if (perfChart.data.labels.length >= maxPoints) {
            perfChart.data.labels.shift();
            perfChart.data.datasets[0].data.shift();
            perfChart.data.datasets[1].data.shift();
        }
        perfChart.data.labels.push(time);
        perfChart.data.datasets[0].data.push(sensors.cpu);
        perfChart.data.datasets[1].data.push(sensors.memory);
        perfChart.update('none'); // Без анимации для плавности
    }
    
    async startDataUpdates() {
        if (this.simulationMode) {
            this.startSimulation();
        } else {
            this.startApiUpdates();
        }
    }
    
    async startApiUpdates() {
        try {
            const sensors = await this.api.getSensors();
            this.updateSensors(sensors);
        } catch (error) {
            console.error('Failed to update from API:', error);
            this.simulationMode = true;
            this.startSimulation();
        }
        
        setTimeout(() => this.startApiUpdates(), 2000);
    }
    
    startSimulation() {
        if (!this.simulationMode) return;
        
        const sensors = this.generateMockData();
        this.updateSensors(sensors);
        
        setTimeout(() => this.startSimulation(), 2000);
    }
    
    generateMockData() {
        const now = new Date();
        const hour = now.getHours();
        const baseTemp = 22 + Math.sin((hour - 6) * Math.PI / 12) * 3;
        
        return {
            temperature: baseTemp + (Math.random() - 0.5) * 2,
            cpu: 30 + Math.random() * 40 + Math.sin(Date.now() / 10000) * 10,
            memory: 40 + Math.random() * 30,
            pulse: 65 + Math.random() * 20,
            battery: Math.max(20, 95 - (Date.now() / 100000) % 80)
        };
    }
}

// Глобальные функции для совместимости
window.sendCommand = async function(command) {
    if (window.dashboard) {
        return await window.dashboard.sendCommand(command);
    }
};

window.toggleSimulation = function() {
    if (window.dashboard) {
        window.dashboard.simulationMode = !window.dashboard.simulationMode;
        const button = document.getElementById('simButton');
        button.textContent = window.dashboard.simulationMode ? 'Остановить симуляцию' : 'Запустить симуляцию';
        
        if (window.dashboard.simulationMode) {
            window.dashboard.startSimulation();
        } else {
            window.dashboard.startApiUpdates();
        }
    }
};

// Инициализация
document.addEventListener('DOMContentLoaded', function() {
    lucide.createIcons();
    window.dashboard = new IoTDashboard();
});
