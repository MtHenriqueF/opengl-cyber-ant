# ==============================================================================
# Makefile - Computação Gráfica (IME 04-10842) - Trabalho Final 2026.1
# Projeto: Simulador de Formiga Robô 3D em OpenGL
# ==============================================================================

CXX      ?= g++
CXXFLAGS ?= -Wall -Wextra -O2

# Detecção de SO para seleção das bibliotecas gráficas nativas
ifeq ($(OS),Windows_NT)
    LIBS       := -lfreeglut -lopengl32 -lglu32
    TARGET_EXT := .exe
    RM         := del /Q /F
else
    LIBS       := -lGL -lGLU -lglut -lm
    TARGET_EXT :=
    RM         := rm -f
endif

# Alvo dinâmico: compila formiga se existir, senão compila garra
ifeq ($(wildcard formiga.cpp),)
    TARGET := garra$(TARGET_EXT)
    SRC    := garra.cpp
else
    TARGET := formiga$(TARGET_EXT)
    SRC    := formiga.cpp
endif

SRCS := $(SRC) RgbImage.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) RgbImage.h
	$(CXX) $(CXXFLAGS) $(SRCS) $(LIBS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	$(RM) garra garra.exe formiga formiga.exe *.o
