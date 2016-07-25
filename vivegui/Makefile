TARGET ?= vivegui
OS ?= $(shell uname)

GLSLIFY ?= node_modules/.bin/glslify
GLSL ?= $(wildcard data/glsl/*.glsl)
GLSL_COMPILED ?= $(GLSL:.glsl=.glsl.compiled)

SRC += $(wildcard deps/*/*.c)
SRC += vivegui.c trackball.c colorspace.c

CFLAGS += -g
CFLAGS += -O0
CFLAGS += -I deps
CFLAGS += -l glfw
CFLAGS += -l GLEW
CFLAGS += -l GLU

ifeq (Darwin, $(OS))
CFLAGS += -framework Foundation
CFLAGS += -framework OpenGL
endif

all: $(GLSL_COMPILED) $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@

$(GLSL_COMPILED): $(GLSL)
	$(GLSLIFY) $(@:.compiled=) -o $@

$(GLSL): node_modules/

$(SRC): deps/

deps/:
	clib install

node_modules/:
	npm i glslify
	npm i glsl-earth glsl-fog glsl-atmosphere

clean:
	rm -f $(TARGET)
	rm -f $(GLSL_COMPILED)
	rm -rf node_modules
