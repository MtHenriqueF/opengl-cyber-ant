# ==============================================================================
# Dockerfile - Computação Gráfica (IME 04-10842) - Trabalho Final 2026.1
# Ambiente isolado para compilação e teste do Simulador de Formiga Robô
# ==============================================================================

FROM ubuntu:22.04

# Evita interações interativas no terminal durante instalação de pacotes
ENV DEBIAN_FRONTEND=noninteractive

# Instalação das ferramentas de compilação C++ e bibliotecas OpenGL/GLUT do Edital
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    freeglut3-dev \
    libglu1-mesa-dev \
    libgl1-mesa-dev \
    libx11-6 \
    libgl1-mesa-dri \
    mingw-w64 \
    && rm -rf /var/lib/apt/lists/*

# Diretório de trabalho isolado dentro do container
WORKDIR /app

# Copia os arquivos do projeto para o container (filtrados pelo .dockerignore)
COPY . /app

# Comando padrão: compilação com o Makefile
CMD ["make"]
