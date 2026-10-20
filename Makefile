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

.PHONY: all run clean validate

all: $(TARGET)

$(TARGET): $(SRCS) RgbImage.h
	$(CXX) $(CXXFLAGS) $(SRCS) $(LIBS) -o $(TARGET)

run: all
	./$(TARGET)

validate:
	@echo "==> [1/3] Verificando diretivas de portabilidade em $(SRC)..."
	@head -n 1 $(SRC) | grep -q "^#define _CRT_SECURE_NO_WARNINGS" || (echo "ERRO: Linha 1 de $(SRC) deve ser #define _CRT_SECURE_NO_WARNINGS" && exit 1)
	@grep -q "#ifdef _WIN32" $(SRC) || (echo "ERRO: Falta guarda #ifdef _WIN32 em $(SRC)" && exit 1)
	@echo "==> [2/3] Compilando $(SRC) com flags estritas (-Wall -Wextra -Werror)..."
	$(CXX) -Wall -Wextra -Werror -O2 $(SRCS) $(LIBS) -o $(TARGET)
	@echo "==> [3/3] Validando texturas .bmp (24-bits e potencia de 2) com RgbImage..."
	@printf '#include <stdio.h>\n#include "RgbImage.h"\nint main(int c,char**v){for(int i=1;i<c;i++){RgbImage m(v[i]);long w=m.GetNumCols(),h=m.GetNumRows();if(m.GetErrorCode()!=0||w<=0||h<=0||(w&(w-1))!=0||(h&(h-1))!=0){printf("ERRO BMP: %%s\\n",v[i]);return 1;}printf("OK BMP: %%s (%%ldx%%ld)\\n",v[i],w,h);}return 0;}\n' | $(CXX) -DRGBIMAGE_DONT_USE_OPENGL -x c++ - RgbImage.cpp -o test_bmp_loader
	@./test_bmp_loader *.bmp && $(RM) test_bmp_loader
	@echo "==> VALIDACAO CONCLUIDA COM SUCESSO!"

clean:
	$(RM) garra garra.exe formiga formiga.exe test_bmp_loader *.o
