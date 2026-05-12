import ply.lex as lex
import sys
import os

# Palavras reservadas da linguagem

reserved = {
    "entier":     "INT",
    "flotter":    "FLOAT",
    "doubler":    "DOUBLE",
    "chaine":     "STRING",
    "personnage": "CHAR",
    "logique":    "BOOL",
    "si":         "IF",
    "sinon":      "ELSE",
    "sinon_si":   "ELIF",
    "alors_que":  "WHILE",
    "pour":       "FOR",
    "ET":         "AND",
    "OU":         "OR",
    "NON":        "NOT",
    "FIN":        "EOF",
    "afficher":   "PRINT",
    "saisir":     "INPUT",
    "ouvrir":     "OPEN",
    "fermer":     "CLOSE",
    "PLUSPLUS":   "PLUSPLUS",
    "MOINSMOINS": "MOINSMOINS",
}

_simbolos = [
    "ID", "NUMBER",
    "ADD", "SUB", "DIV", "MULT", "PCM",
    "IGUAL", "IGUAL_MAIOR", "IGUAL_MENOR",
    "MAIOR", "MENOR", "IGUALDADE",
    "ouvrirPAREN", "fermerPAREN",
]

tokens = _simbolos + [t for t in reserved.values() if t not in _simbolos]

# Tokens simples

t_ADD         = r'\+'
t_SUB         = r'\-'
t_DIV         = r'/'
t_MULT        = r'\*'
t_PCM         = r'\%'
t_IGUAL       = r'\='
t_MAIOR       = r'\{'
t_MENOR       = r'\}'
t_ouvrirPAREN = r'\('
t_fermerPAREN = r'\)'
t_ignore      = ' \t\r'

# Tokens com regras de prioridade (funções têm prioridade sobre strings)

def t_IGUAL_MAIOR(t):
    r'\{__'
    return t

def t_IGUAL_MENOR(t):
    r'\}__'
    return t

def t_IGUALDADE(t):
    r'__'
    return t

def t_NUMBER(t):
    r'\d+'
    t.value = int(t.value)
    return t

def t_ID(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    t.type = reserved.get(t.value, 'ID')
    return t

def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(t):
    print(f"Caractere inválido: {t.value[0]}")
    t.lexer.skip(1)

# Ponto de entrada

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python analisador_lexico.py <arquivo_fonte>")
        sys.exit(1)

    arquivo = sys.argv[1]
    try:
        with open(arquivo, 'r', encoding='utf-8') as f:
            data = f.read()
    except FileNotFoundError:
        print(f"Erro: arquivo '{arquivo}' não encontrado.")
        sys.exit(1)

    lexer = lex.lex()
    lexer.input(data)

    os.makedirs("saida", exist_ok=True)
    with open("saida/resultado_lexico.txt", "w", encoding='utf-8') as out:
        for tok in lexer:
            linha = f"<{tok.value}, {tok.type}>"
            print(linha)
            out.write(linha + "\n")
