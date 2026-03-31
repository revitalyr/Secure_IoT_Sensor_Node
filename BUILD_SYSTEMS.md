# Build Systems: CMake vs Makefile

## Overview

This project uses **two build systems** for different purposes:

| System | Purpose | When to Use |
|--------|---------|-------------|
| **CMake** | Main firmware build system | For C/C++ code compilation |
| **Makefile** | Development convenience commands | For quick demo and test execution |

---

## CMake - Main Build System

### What CMake Does:
- **Cross-compilation** for ARM Cortex-M4
- **Firmware building** (bootloader + application)
- **Generation of .bin/.elf files**
- **Test building** (native compilation)
- **Dependency management**
- **Multiple configuration support**

### CMake Structure:
```cmake
# Main CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# Two configurations:
if(DEFINED BUILD_TESTS)
    # Native compilation for tests
    project(Secure_IoT_Sensor_Node_Tests)
else()
    # Cross-compilation for firmware
    project(Secure_IoT_Sensor_Node)
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
endif()
```

### Using CMake:

#### Firmware Build:
```bash
# Create build directory
mkdir -p build
cd build

# Configuration (cross-compilation)
cmake ..

# Сборка
make

# Результаты:
# build/app.elf      - исполняемый файл для прошивки
# build/app.bin      - бинарный файл для прошивки
# build/bootloader.elf - загрузчик
```

#### Сборка тестов:
```bash
# Конфигурация для тестов
cmake -DBUILD_TESTS=ON ..

# Сборка тестов
make

# Запуск тестов
ctest
```

### 📊 **Преимущества CMake:**
- ✅ **Кросс-платформенность** (Linux/macOS/Windows)
- ✅ **Автоматическое определение** зависимостей
- ✅ **Поддержка IDE** (CLion, VSCode, Eclipse)
- ✅ **Генерация разных форматов** (Makefile, Ninja, Visual Studio)
- ✅ **Управление конфигурациями** (Debug/Release)
- ✅ **Интеграция с CTest** для тестов

---

## 🛠️ Makefile - Удобные команды разработки

### ✅ **Что делает Makefile:**
- **Быстрый запуск демо** (`make demo`)
- **Прошивка устройства** (`make flash`)
- **Управление процессами** (`make stop`)
- **Тестирование моста** (`make test-bridge`)
- **Статистика проекта** (`make stats`)

### 📁 **Структура Makefile:**
```makefile
# Цели для прошивки
build:
	mkdir -p build && cd build && cmake .. && make

flash: build
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program build/app.elf verify reset exit"

# Цели для демо
demo:
	cd demo && python api_server.py &
	cd demo/web_visualization && python server.py &
```

### 🚀 **Использование Makefile:**

#### Основные команды:
```bash
# Полная разработка
make dev              # сборка → прошивка → демо

# Только демо
make demo             # демо с симуляцией
make demo-real        # демо с реальным устройством

# Разработка
make build            # сборка прошивки через CMake
make flash            # прошивка устройства
make test             # запуск тестов через CMake
```

#### Вспомогательные команды:
```bash
make help             # показать все команды
make stop             # остановить демо
make check-ports      # проверить порты устройства
make stats            # статистика проекта
make clean            # очистка сборки
```

### 📊 **Преимущества Makefile:**
- ✅ **Простые команды** для частых операций
- ✅ **Интеграция с демо** (Python процессы)
- ✅ **Удобство для разработки** (одна команда вместо скриптов)
- ✅ **Контекстная помощь** (`make help`)

---

## 🔄 Как они работают вместе

### 📈 **Иерархия:**
```
Makefile (высокоуровневые команды)
    ↓
CMake (низкоуровневая сборка)
    ↓
GCC/ARM Toolchain (компиляция)
```

### 🔄 **Взаимодействие:**
```bash
# Makefile вызывает CMake
make build
# Выполняется: cd build && cmake .. && make

# Makefile управляет демо процессами
make demo
# Запускает Python процессы независимо от CMake
```

### 📊 **Разделение ответственности:**

| Задача | Система | Пример |
|--------|---------|--------|
| Компиляция C/C++ | **CMake** | `make build` → `cmake && make` |
| Кросс-компиляция | **CMake** | ARM toolchain конфигурация |
| Запуск демо | **Makefile** | `make demo` → Python процессы |
| Прошивка устройства | **Makefile** | `make flash` → OpenOCD |
| Тестирование | **CMake** | `ctest` |
| Статистика | **Makefile** | `make stats` |

---

## 🎯 Когда что использовать

### 🏗️ **Используйте CMake для:**
- Компиляции прошивки
- Сборки тестов
- Кросс-компиляции
- Интеграции с IDE
- CI/CD пайплайнов

### 🛠️ **Используйте Makefile для:**
- Быстрого запуска демо
- Прошивки устройства
- Управления процессами
- Повседневной разработки
- Удобных команд

---

## 📝 Примеры использования

### 🚀 **Полный цикл разработки:**
```bash
# 1. Разработка кода
vim app/web_interface.c

# 2. Сборка прошивки
make build

# 3. Прошивка устройства
make flash

# 4. Запуск демо с реальным устройством
make demo-real

# 5. Тестирование
make test

# 6. Очистка
make stop && make clean
```

### 🧪 **Только разработка прошивки:**
```bash
# Используем только CMake
mkdir build && cd build
cmake ..
make -j4          # параллельная сборка
```

### 🌐 **Только демо (без прошивки):**
```bash
# Используем только Makefile
make demo         # или make demo-real
```

---

## 🔧 Расширение систем

### ➕ **Добавление нового файла в CMake:**
```cmake
set(APP_SOURCES
    app/main.c
    app/new_file.c    # добавить сюда
    # ...
)
```

### ➕ **Добавление новой команды в Makefile:**
```makefile
new-command:
	@echo "Новая команда"
	# ваша логика
```

---

## 📊 Сравнение

| Аспект | CMake | Makefile |
|--------|-------|----------|
| **Сложность** | Высокая | Низкая |
| **Гибкость** | Высокая | Средняя |
| **Кросс-платформенность** | ✅ | ❌ (только Unix-like) |
| **IDE поддержка** | ✅ | ❌ |
| **Простота** | ❌ | ✅ |
| **Управление процессами** | ❌ | ✅ |

---

## 🎯 Вывод

**CMake** - **мощный** инструмент для сборки C/C++ кода  
**Makefile** - **удобный** инструмент для разработки и демо

Они **дополняют друг друга**, а не конкурируют:
- CMake отвечает за **компиляцию**
- Makefile отвечает за **удобство использования**

**Это стандартный подход** в embedded проектах: CMake для сборки, Makefile для разработки.
