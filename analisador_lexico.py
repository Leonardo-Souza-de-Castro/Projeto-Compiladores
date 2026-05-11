import sys
import ply.lex as lex

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
    "FIN":"EOF",
    "saisir":"INPUT",
    "afficher":"PRINT",
    "ouvrir":"OPEN",
    "fermer":"CLOSE",
}
# Definição de tokens

tokens = [
    "ID",   # Identificadores
    "NUMBER",  # Números
    "PLUSPLUS",  # Incremento
    "MOINSMOINS",  # Decremento
    "ouvrirPAREN",  # (
    "fermerPAREN",  # )
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

def t_PLUSPLUS(t):
    r'PLUSPLUS'
    return t

def t_ouvrirPAREN(t):
    r'\('
    return t

def t_fermerPAREN(t):
    r'\)'
    return t

def t_MOINSMOINS(t):
    r'MOINSMOINS'
    return t

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

def main() -> None:
    """Ponto de entrada: recebe o arquivo fonte como argumento CLI."""
    if len(sys.argv) < 2:
        print("Uso: python analisador_lexico.py <arquivo_fonte>")
        sys.exit(1)

    arquivo_fonte = sys.argv[1]

    with open(arquivo_fonte, 'r') as file:
        data = file.read()

    lexer = lex.lex()
    lexer.input(data)

    arquivo_resultado = "./saida/resultado_lexico.txt"

    with open(arquivo_resultado, 'w') as file:
        for tok in lexer:
            print(f"<{tok.value}, {tok.type}>")
            file.write(f"<{tok.value}, {tok.type}>" + '\n')

    print("Analise lexica concluida. Resultado salvo em './saida/resultado_lexico.txt'")


if __name__ == "__main__":
    main()
