# ------------------- Настройки ----------------------------
PROJECT_NAME  := c-chat-app
BUILD_DIR     := build
INSTALL_PREFIX?= $(HOME)/.local  # или /usr/local, если есть права

# Путь к исходникам (можно не менять)
SRC_DIR       := .

# ------------------- Цели по умолчанию --------------------
.PHONY: all build clean test install run

all: build

# ------------------- Сборка проекта -----------------------
build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(SRC_DIR)
	$(MAKE) -C $(BUILD_DIR)

# ------------------- Запуск приложения --------------------
run: build
	$(BUILD_DIR)/$(PROJECT_NAME)

# ------------------- Очистка ------------------------------
clean:
	rm -rf $(BUILD_DIR)

# ------------------- Тестирование -------------------------
test: build
	cd $(BUILD_DIR) && ctest --output-on-failure || true

# ------------------- Установка ----------------------------
install: build
	$(MAKE) -C $(BUILD_DIR) install DESTDIR=$(INSTALL_PREFIX)

# ------------------- Дополнительные цели ------------------
# Пересобрать с полной переконфигурацией
rebuild: clean build

# Форматирование кода (если используете clang-format)
format:
	find ./src/ -name "*.c" -o -name "*.h" | xargs clang-format -style=Google -i

# Справка
help:
	@echo "Доступные цели:"
	@echo "  all      - сборка проекта (по умолчанию)"
	@echo "  build    - собрать проект"
	@echo "  clean    - удалить папку build"
	@echo "  test     - запустить тесты (ctest)"
	@echo "  install  - установить исполняемый файл (в $(INSTALL_PREFIX))"
	@echo "  run      - собрать и запустить приложение"
	@echo "  rebuild  - пересобрать с нуля"
	@echo "  format   - отформатировать исходники (clang-format)"
	@echo "  help     - показать эту справку"
