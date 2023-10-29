.PHONY: run

run: main
	./$<

main: src/main.c
	gcc `pkg-config --cflags --libs libpng glew glfw3` -lm -g -o $@ $^
