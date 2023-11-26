.PHONY: run clean

CFLAGS := -Wall -g -Iglad/include -MMD -MP -c
LDFLAGS := -lglfw -lpng

OBJ := $(patsubst src/%.c,out/%.o,$(wildcard src/*.c))
OBJ += out/gl.o

run: out/main
	./$<

glad:
	glad --api gl:core=4.6 --out-path $@

out:
	mkdir -p out

out/%.o: src/%.c | out glad
	clang $(CFLAGS) -o $@ $<

out/gl.o: glad/src/gl.c | out glad
	clang $(CFLAGS) -o $@ $<

out/main: $(OBJ) | out
	clang $(LDFLAGS) -o $@ $^

clean:
	@$(RM) -rv out glad

-include $(OBJ:.o=.d)
