CC = gcc
EXEC = main
SOURCE_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
CFLAGS = -Wall -MMD -MP -g -I $(INCLUDE_DIR)

CFILES = $(shell find $(SOURCE_DIR) -name "*.c" -type f)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/%.o, $(CFILES))
DEPENDS = ${OBJECTS:.o=.d}

.PHONY: all run clean

all : ${EXEC}

run: ${EXEC}
	./${EXEC}

clean:
	rm -f ${EXEC} ${OBJECTS} ${DEPENDS}

${EXEC}: ${OBJECTS}
	${CC} ${OBJECTS} -o ${EXEC}
-include ${DEPENDS}

# remember to ensure that build directory exists
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(shell dirname $@) 
	${CC} ${CFLAGS} -c $< -o $@
