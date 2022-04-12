LOGGER_DIR = ./src/logger/
INSPECTOR_DIR = ./src/inspector/

LOGGER_C_FILES = $(shell find $(LOGGER_DIR) -name "*.c")
LOGGER_O_FILES = $(patsubst %.c,%.o,$(LOGGER_C_FILES))

INSPECTOR_C_FILES = $(shell find $(INSPECTOR_DIR) -name "*.c")
INSPECTOR_O_FILES = $(patsubst %.c,%.o,$(INSPECTOR_C_FILES))

all: logger logger.so

logger: $(LOGGER_O_FILES)
	gcc $^ -o $@

logger.so: $(INSPECTOR_O_FILES)
	gcc $^ -shared -ldl -o $@

%.o: %.c
	gcc -c $< -o $@

.PHONY: clean
clean:
	rm -f logger logger.so $(LOGGER_O_FILES) $(INSPECTOR_O_FILES)