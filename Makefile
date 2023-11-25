.PHONY: run

run: main
	./$<

glad:
	glad --api gl:core=4.6 --out-path $@

glad/src/gl.c: glad

main: src/*.c glad/src/gl.c
	clang -Wall -g -Iglad/include -lglfw -lpng -o $@ $^
