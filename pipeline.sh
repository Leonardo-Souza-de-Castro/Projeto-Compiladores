#!/bin/bash
# =============================================================
# pipeline.sh — Executa o compilador completo
#
# Uso:
#   ./pipeline.sh <arquivo_fonte>
#
# Exemplo:
#   ./pipeline.sh teste.teste
#
# Etapas:
#   1. Análise léxica   (Python)   → resultado_lexico.txt
#   2. Análise sintática (Java)    → resultado_sintatico.txt
#   3. Compilação do semântico (C) → AnalisadorSemantico/semantico
#   4. Análise semântica / geração → <nome_fonte>.c
# =============================================================

# ── Verificação de Dependências ────────────────────────────────
echo "Verificando dependencias..."

# Python e PLY
if ! python -c "import ply" &> /dev/null; then
    echo "Instalando biblioteca 'ply' para Python..."
    pip install ply
fi

# Java (javac)
if ! command -v javac &> /dev/null; then
    echo "Erro: 'javac' nao encontrado. Por favor, instale o JDK (Java Development Kit)."
    exit 1
fi

# C (gcc)
if ! command -v gcc &> /dev/null; then
    echo "Erro: 'gcc' nao encontrado. Por favor, instale um compilador C (como MinGW ou GCC)."
    exit 1
fi

# ── Validação do argumento ────────────────────────────────────
if [ -z "$1" ]; then
    echo "Uso: $0 <arquivo_fonte>"
    echo "Exemplo: $0 impares.teste"
    exit 1
fi

ARQUIVO_FONTE="$1"

if [ ! -f "$ARQUIVO_FONTE" ]; then
    echo "Erro: arquivo '$ARQUIVO_FONTE' nao encontrado."
    exit 1
fi

# Deriva o nome do arquivo de saída a partir do nome do arquivo fonte:
# remove a extensão original e substitui por .c  (ex: impares.fr → impares.c)
BASE_NOME="${ARQUIVO_FONTE%.*}"
ARQUIVO_SAIDA="${BASE_NOME}.c"

echo "============================================="
echo " Compilador — Pipeline completo"
echo " Fonte:  $ARQUIVO_FONTE"
echo " Saida:  $ARQUIVO_SAIDA"
echo "============================================="

# ── Passo 1: Análise Léxica ───────────────────────────────────
# O lexer lê o caminho do arquivo via input() (stdin),
# portanto usamos uma here-string para passá-lo automaticamente.
echo ""
echo "[1/4] Executando analise lexica..."
python analisador_lexico.py <<< "$ARQUIVO_FONTE"
if [ $? -ne 0 ]; then
    echo "      ERRO: falha na analise lexica."
    exit 1
fi
echo "      OK → resultado_lexico.txt"

# ── Passo 2: Compilação e execução do Analisador Sintático ────
echo ""
echo "[2/4] Compilando o analisador sintatico (Java)..."
javac AnalisadorSintatico/*.java

echo "      Executando o analisador sintatico..."
java -cp AnalisadorSintatico main
echo "      OK → resultado_sintatico.txt"

# ── Passo 3: Compilação do Analisador Semântico (C) ──────────
# Recompila apenas se o binário não existir ou se os fontes
# forem mais recentes que o executável.
# No Windows via Git Bash o gcc gera 'semantico.exe'; tentamos ambos.
SEMANTICO="AnalisadorSemantico/semantico"
FONTES_C="AnalisadorSemantico/main.c AnalisadorSemantico/node.c AnalisadorSemantico/codegen.c"

echo ""
echo "[3/4] Compilando o analisador semantico (C)..."

# Resolve o caminho real do binário (com ou sem .exe)
if   [ -f "${SEMANTICO}.exe" ]; then SEMANTICO_BIN="${SEMANTICO}.exe"
elif [ -f "${SEMANTICO}"     ]; then SEMANTICO_BIN="${SEMANTICO}"
else                                  SEMANTICO_BIN=""
fi

RECOMPILAR=false
if [ -z "$SEMANTICO_BIN" ]; then
    RECOMPILAR=true
else
    # Verifica se algum fonte é mais novo que o binário
    for src in $FONTES_C; do
        if [ "$src" -nt "$SEMANTICO_BIN" ]; then
            RECOMPILAR=true
            break
        fi
    done
fi

if [ "$RECOMPILAR" = true ]; then
    # -Wno-format-truncation: suprime aviso de truncamento do snprintf que o
    # MinGW trata como erro fatal por calcular worst-case com MAX_NAME_LEN.
    gcc -Wall -Wno-format-truncation -std=c11 -o "$SEMANTICO" $FONTES_C 2>&1
    if [ $? -ne 0 ]; then
        echo "      ERRO: falha na compilacao do analisador semantico (veja mensagem acima)."
        exit 1
    fi
    # Após compilar, detecta o binário gerado novamente
    if   [ -f "${SEMANTICO}.exe" ]; then SEMANTICO_BIN="${SEMANTICO}.exe"
    elif [ -f "${SEMANTICO}"     ]; then SEMANTICO_BIN="${SEMANTICO}"
    fi
    echo "      Recompilado com sucesso."
else
    echo "      Binario ja atualizado, pulando compilacao."
fi

# ── Passo 4: Geração de Código C ─────────────────────────────
echo ""
echo "[4/4] Gerando codigo C..."
"$SEMANTICO_BIN" resultado_sintatico.txt "$ARQUIVO_SAIDA"
if [ $? -ne 0 ]; then
    echo "      ERRO: falha na geracao de codigo C."
    exit 1
fi
echo "      OK → $ARQUIVO_SAIDA"

# ── Passo 5: Compila e executa o codigo C gerado ─────────────
EXECUTAVEL="${BASE_NOME}"

echo ""
echo "[5/5] Compilando o codigo C gerado..."
gcc "$ARQUIVO_SAIDA" -o "$EXECUTAVEL"
if [ $? -ne 0 ]; then
    echo "      ERRO: falha ao compilar $ARQUIVO_SAIDA"
    exit 1
fi
echo "      OK → executavel '$EXECUTAVEL' gerado"

echo ""
echo "============================================="
echo " Pipeline concluido com sucesso!"
echo " Arquivo gerado: $ARQUIVO_SAIDA"
echo " Executavel:     $EXECUTAVEL"
echo "============================================="
echo ""
echo "Executando o programa..."
echo "---------------------------------------------"
"./$EXECUTAVEL"