# Compilador do Analisador Semantico
# Uso:
#   make          → compila o executavel 'semantico'
#   make run      → executa com saida/resultado_sintatico.txt padrão
#   make clean    → remove arquivos gerados

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pedantic
TARGET  = semantico
SRCS    = main.c node.c codegen.c
OBJS    = $(SRCS:.c=.o)

# Arquivo intermediario gerado pelo analisador sintatico Java
AST_FILE = ../saida/resultado_sintatico.txt

# Arquivo de saida C gerado pelo analisador semantico
SAIDA = saida.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Executa o analisador semantico com os arquivos padrao do projeto
run: $(TARGET)
	./$(TARGET) $(AST_FILE) $(SAIDA)

clean:
	del /f /q $(OBJS) $(TARGET).exe $(SAIDA) 2>nul || true
