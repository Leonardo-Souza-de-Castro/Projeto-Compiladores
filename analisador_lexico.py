import ply.lex as lex
from ply import yacc

# Definição de palavras reservadas

reserved = {
    "entier":"INT",
    "flotter":"FLOAT",
    "doubler":"DOUBLE",
    "chaine":"STRING",
    "personnage":"CHAR",
    "logique":"BOOL",
    "si":"IF",
    "sinon":"ELSE",
    "sinon_si":"ELIF",
    "alors_que":"WHILE",
    "pour":"FOR",
    "ET":"AND",
    "OU":"OR",
    "NON":"NOT",
}

# Definição de tokens

tokens = [
    "ID",   # Identificadores
    "NUMBER",  # Números
    "ADD",    #+
    "SUB",    #-
    "DIV",    #/
    "MULT",   #*
    "PCM",    #%
    "IGUAL",  #=
    "IGUAL_MAIOR",  #{__
    "IGUAL_MENOR",  #}__
    "MAIOR",        #{
    "MENOR",        #}
    "IGUALDADE"     #__
] + list(reserved.values())

# Definição de expressões regulares para os tokens simples

t_ADD = r'\+'
t_SUB = r'\-'
t_DIV = r'/'
t_MULT = r'\*'
t_PCM = r'\%'
t_IGUAL = r'\='
t_MAIOR = r'\{'
t_MENOR = r'\}'
t_ignore = ' \t'

# Definição de funções para tokens mais complexos

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

# Definição de função para contar linhas
def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

# Definição de função para lidar com erros
def t_error(t):
    print(f"Caractere inválido: {t.value[0]}")
    t.lexer.skip(1)

# Exemplo de uso do lexer
data = """
entier x = 10 {__ 5
si x __ 10
"""

lexer = lex.lex()
lexer.input(data)

for tok in lexer:
    print(tok)