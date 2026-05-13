"""
mostrar_tokens.py — Exibe os tokens disponíveis na linguagem antes de iniciar o pipeline.

Pergunta ao usuário se deseja ver a lista de tokens (S/N).
  - S → imprime todos os tokens e encerra com código 0.
  - N → encerra com código 0, sinalizando ao pipeline que pode continuar.
"""

# ── Definição dos tokens (espelho do analisador_lexico.py) ──────────────────

RESERVED_TOKENS = {
    "entier":    "INT",
    "flotter":   "FLOAT",
    "doubler":   "DOUBLE",
    "chaine":    "STRING",
    "personnage":"CHAR",
    "logique":   "BOOL",
    "si":        "IF",
    "sinon":     "ELSE",
    "sinon_si":  "ELIF",
    "alors_que": "WHILE",
    "pour":      "FOR",
    "ET":        "AND",
    "OU":        "OR",
    "NON":       "NOT",
    "FIN":       "EOF",
    "saisir":    "INPUT",
    "afficher":  "PRINT",
    "ouvrir":    "OPEN",
    "fermer":    "CLOSE",
}

SYMBOL_TOKENS = [
    ("ID",          "Identificadores"),
    ("NUMBER",      "Números inteiros"),
    ("PLUSPLUS",    "Incremento  (PLUSPLUS)"),
    ("MOINSMOINS",  "Decremento  (MOINSMOINS)"),
    ("ouvrirPAREN", "Abre parêntese  ("),
    ("fermerPAREN", "Fecha parêntese )"),
    ("ADD",         "Adição          +"),
    ("SUB",         "Subtração       -"),
    ("DIV",         "Divisão         /"),
    ("MULT",        "Multiplicação   *"),
    ("PCM",         "Módulo          %"),
    ("IGUAL",       "Atribuição      ="),
    ("IGUAL_MAIOR", "Maior ou igual  {__"),
    ("IGUAL_MENOR", "Menor ou igual  }__"),
    ("MAIOR",       "Maior que       {"),
    ("MENOR",       "Menor que       }"),
    ("IGUALDADE",   "Igualdade       __"),
]


def _print_tokens() -> None:
    """Imprime a tabela completa de tokens: palavras reservadas + símbolos."""
    print("\n╔══════════════════════════════════════════════════════╗")
    print("║           Tokens da Linguagem (léxico)               ║")
    print("╠══════════════════════════════════════════════════════╣")

    print("║  ► Palavras Reservadas                               ║")
    print("╠══════════════════╦═══════════════════════════════════╣")
    print("║  Palavra-chave   ║  Token                            ║")
    print("╠══════════════════╬═══════════════════════════════════╣")
    for keyword, token in RESERVED_TOKENS.items():
        print(f"║  {keyword:<16}║  {token:<33}║")

    print("╠══════════════════╩═══════════════════════════════════╣")
    print("║  ► Símbolos e Identificadores                        ║")
    print("╠══════════════════╦═══════════════════════════════════╣")
    print("║  Token           ║  Descrição                        ║")
    print("╠══════════════════╬═══════════════════════════════════╣")
    for token, description in SYMBOL_TOKENS:
        print(f"║  {token:<16}║  {description:<33}║")

    print("╚══════════════════╩═══════════════════════════════════╝")
    print()


def _ask_user() -> bool:
    """
    Pergunta ao usuário se deseja exibir os tokens.
    Aceita 's', 'S', 'n', 'N'. Repete até receber uma resposta válida.
    Retorna True para S, False para N.
    """
    while True:
        resposta = input("Deseja exibir todos os tokens da linguagem? [S/N]: ").strip().upper()
        if resposta == "S":
            return True
        if resposta == "N":
            return False
        print("  Resposta inválida. Digite S ou N.")


def main() -> None:
    if _ask_user():
        _print_tokens()


if __name__ == "__main__":
    main()
