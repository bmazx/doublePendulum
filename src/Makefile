CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl

INC_PATH = dependencies
INC_DIR = -I$(INC_PATH)/glad/include -I$(INC_PATH)/glfw/include

GLAD_SRC = $(INC_PATH)/glad/src/glad.c

PROJECT_SRC = main.cpp dependencies/glad/src/glad.c ogls.cpp

doublePendulum: main.cpp
	g++ -g $(CFLAGS) -o doublePendulum $(PROJECT_SRC) $(LDFLAGS) $(INC_DIR)

.PHONY: test 

test: doublePendulum
	./doublePendulum

