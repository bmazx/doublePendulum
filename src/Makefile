CFLAGS = -std=c++20 -O2

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl

INC_DIR = -Idependencies -Idependencies/glad/include -Idependencies/glfw/include

IMGUI_DIR = dependencies/imgui
IMGUI_SRC = $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_impl_glfw.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_impl_opengl3.cpp $(IMGUI_DIR)/imgui_tables.cpp

PROJECT_SRC = main.cpp dependencies/glad/src/glad.c ogls.cpp $(IMGUI_SRC)

doublePendulum: main.cpp
	g++ -g $(CFLAGS) -o doublePendulum $(PROJECT_SRC) $(LDFLAGS) $(INC_DIR)

.PHONY: test 

test: doublePendulum
	./doublePendulum

