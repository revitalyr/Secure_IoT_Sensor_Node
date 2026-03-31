# Makefile для Secure IoT Sensor Node
# Поддержка сборки прошивки и запуска демо-приложения

.PHONY: all build clean flash demo demo-real test help

# Переменные
BUILD_DIR = build
DEMO_DIR = demo
FIRMWARE = app.elf
BINARY = app.bin

# Цели по умолчанию
all: build

# Сборка прошивки
build:
	@echo "🔧 Сборка прошивки..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make
	@echo "✅ Сборка завершена: $(BUILD_DIR)/$(FIRMWARE)"

# Очистка
clean:
	@echo "🧹 Очистка..."
	@rm -rf $(BUILD_DIR)
	@echo "✅ Очистка завершена"

# Прошивка устройства (требует настроенного OpenOCD)
flash: build
	@echo "📡 Прошивка устройства..."
	@if [ -f "$(BUILD_DIR)/$(FIRMWARE)" ]; then \
		openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
			-c "program $(BUILD_DIR)/$(FIRMWARE) verify reset exit"; \
		echo "✅ Прошивка завершена"; \
	else \
		echo "❌ Файл прошивки не найден. Сначала выполните 'make build'"; \
		exit 1; \
	fi

# Запуск демо с симуляцией
demo:
	@echo "🚀 Запуск демо-панели..."
	@cd demo && source venv/bin/activate && python api_server.py & \
	sleep 2 && \
	cd demo/web_visualization && python server.py & \
	echo "✅ Демо запущено:" && \
	echo "   API: http://localhost:5000" && \
	echo "   Web: http://localhost:8080"

# Запуск демо с реальным устройством
demo-real:
	@echo "🚀 Запуск демо-панели с реальным устройством..."
	@cd demo && source venv/bin/activate && python api_server.py & \
	cd demo/web_visualization && python server.py & \
	echo "✅ Демо запущено с реальным устройством:" && \
	echo "   API: http://localhost:5000" && \
	echo "   Web: http://localhost:8080" && \
	echo "   Убедитесь, что устройство подключено и прошито"

# Остановка всех процессов
stop:
	@echo "🛑 Остановка демо..."
	@pkill -f "python api_server.py" || true
	@pkill -f "python server.py" || true
	@echo "✅ Процессы остановлены"

# Тестирование моста устройства
test-bridge:
	@echo "🔧 Тестирование моста устройства..."
	@cd $(DEMO_DIR) && python real_device_bridge.py

# Запуск тестов
test:
	@echo "🧪 Запуск тестов..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Установка зависимостей
install-deps:
	@echo "📦 Установка зависимостей Python..."
	@pip install -r $(DEMO_DIR)/requirements.txt
	@echo "✅ Зависимости установлены"

# Проверка исходного кода
check:
	@echo "🔍 Проверка исходного кода..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find app/ -name "*.c" -o -name "*.h" | xargs clang-format -i --dry-run --Werror; \
		echo "✅ Форматирование кода корректно"; \
	else \
		echo "⚠️ clang-format не найден. Установите для проверки форматирования."; \
	fi

# Сборка документации
docs:
	@echo "📚 Генерация документации..."
	@mkdir -p docs
	@doxygen Doxyfile 2>/dev/null || echo "⚠️ Doxygen не найден"
	@echo "✅ Документация сгенерирована в docs/"

# Статистика проекта
stats:
	@echo "📊 Статистика проекта:"
	@echo "   Исходных файлов: $$(find app/ -name "*.c" -o -name "*.h" | wc -l)"
	@echo "   Тестовых файлов: $$(find tests/ -name "*.c" -o -name "*.h" | wc -l)"
	@echo "   Строк кода: $$(find app/ -name "*.c" | xargs wc -l | tail -1)"
	@echo "   Размер прошивки: $$(ls -lh $(BUILD_DIR)/$(FIRMWARE) 2>/dev/null | awk '{print $$5}' || echo "N/A")"

# Полный цикл разработки: сборка → прошивка → демо
dev: build flash demo-real

# Быстрый запуск для разработки (без прошивки)
quick-demo:
	@echo "⚡ Быстрый запуск демо..."
	@cd $(DEMO_DIR) && python api_server.py &
	@sleep 1
	@cd $(DEMO_DIR)/web_visualization && python server.py &
	@echo "✅ Быстрый демо запущен"

# Проверка портов устройства
check-ports:
	@echo "🔍 Проверка портов устройства:"
	@echo "   Linux порты:"
	@ls -la /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "     USB/ACM порты не найдены"
	@echo "   macOS порты:"
	@ls -la /dev/tty.usbserial* 2>/dev/null || echo "     USB Serial порты не найдены"

# Создание архива проекта
archive:
	@echo "📦 Создание архива проекта..."
	@git archive --format=zip --prefix=Secure_IoT_Sensor_Node/ HEAD > Secure_IoT_Sensor_Node_$(shell date +%Y%m%d_%H%M%S).zip
	@echo "✅ Архив создан"

# Помощь
help:
	@echo "🚀 Secure IoT Sensor Node - Makefile"
	@echo ""
	@echo "Основные цели:"
	@echo "  build          - Сборка прошивки"
	@echo "  clean          - Очистка сборки"
	@echo "  flash          - Прошивка устройства"
	@echo "  demo           - Запуск демо с симуляцией"
	@echo "  demo-real      - Запуск демо с реальным устройством"
	@echo "  stop           - Остановка всех демо-процессов"
	@echo ""
	@echo "Разработка:"
	@echo "  dev            - Полный цикл: сборка → прошивка → демо"
	@echo "  quick-demo     - Быстрый запуск демо (без прошивки)"
	@echo "  test-bridge    - Тестирование моста устройства"
	@echo "  test           - Запуск тестов"
	@echo ""
	@echo "Утилиты:"
	@echo "  install-deps   - Установка зависимостей"
	@echo "  check          - Проверка форматирования кода"
	@echo "  check-ports    - Проверка портов устройства"
	@echo "  stats          - Статистика проекта"
	@echo "  docs           - Генерация документации"
	@echo "  archive        - Создание архива проекта"
	@echo "  help           - Эта справка"
	@echo ""
	@echo "Примеры использования:"
	@echo "  make dev              # Полная разработка"
	@echo "  make build && make flash # Только прошивка"
	@echo "  make demo-real          # Демо с устройством"
	@echo "  make stop              # Остановка демо"
