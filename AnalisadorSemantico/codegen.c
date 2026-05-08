#include "codegen.h"
#include "node.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ─────────────────────────────────────────────
   Estado interno compartilhado durante o percurso da AST
   ───────────────────────────────────────────── */
typedef struct {
    const ASTree *tree;
    int           pos;    /* índice do nó atual */
    int           indent; /* nível de indentação atual */
    FILE         *out;    /* arquivo de saída .c */
} GenCtx;

/* Declarações antecipadas */
static void emit_indent(GenCtx *ctx);
static void gen_programa(GenCtx *ctx);
static void gen_comando(GenCtx *ctx);
static void gen_if(GenCtx *ctx);
static void gen_ifelse(GenCtx *ctx);
static void gen_else(GenCtx *ctx);
static void gen_while(GenCtx *ctx);
static void gen_for(GenCtx *ctx);
static void gen_print(GenCtx *ctx);
static void gen_input(GenCtx *ctx);
static void gen_atribuicao(GenCtx *ctx);
static void gen_bloco(GenCtx *ctx);
static void gen_condicoes_folhas(GenCtx *ctx, int profundidade_cond);
static void gen_expr(GenCtx *ctx);
static const char *mapear_tipo(const char *tipo);
static const char *mapear_op_relacional(const char *op);
static const char *mapear_op_logico(const char *op);
static const char *mapear_op_aritmetico(const char *op);

/* ─────────────────────────────────────────────
   Funções auxiliares de navegação
   ───────────────────────────────────────────── */

/* Verifica se ainda há nós a consumir */
static inline int tem_proximo(GenCtx *ctx) {
    return ctx->pos < ctx->tree->count;
}

/* Retorna o nó atual sem avançar */
static inline const ASTNode *atual(GenCtx *ctx) {
    return &ctx->tree->nodes[ctx->pos];
}

/* Avança o cursor e retorna o nó consumido */
static inline const ASTNode *consumir(GenCtx *ctx) {
    return &ctx->tree->nodes[ctx->pos++];
}

/* Emite a indentação atual em espaços */
static void emit_indent(GenCtx *ctx) {
    for (int i = 0; i < ctx->indent; i++) fprintf(ctx->out, "    ");
}

/* ─────────────────────────────────────────────
   Tabelas de mapeamento: linguagem customizada → C
   ───────────────────────────────────────────── */

/* Mapeia tipo da linguagem customizada para tipo C */
static const char *mapear_tipo(const char *tipo) {
    if (strcmp(tipo, "entier")     == 0) return "int";
    if (strcmp(tipo, "flotter")    == 0) return "float";
    if (strcmp(tipo, "doubler")    == 0) return "double";
    if (strcmp(tipo, "chaine")     == 0) return "char*";
    if (strcmp(tipo, "personnage") == 0) return "char";
    if (strcmp(tipo, "logique")    == 0) return "int";
    return tipo; /* passagem direta se não reconhecido */
}

/* Mapeia operador relacional para C */
static const char *mapear_op_relacional(const char *op) {
    if (strcmp(op, "{__") == 0 || strcmp(op, "IGUAL_MAIOR") == 0) return ">=";
    if (strcmp(op, "}__") == 0 || strcmp(op, "IGUAL_MENOR") == 0) return "<=";
    if (strcmp(op, "__")  == 0 || strcmp(op, "IGUALDADE")   == 0) return "==";
    if (strcmp(op, "{")   == 0 || strcmp(op, "MAIOR")       == 0) return ">";
    if (strcmp(op, "}")   == 0 || strcmp(op, "MENOR")       == 0) return "<";
    return op;
}

/* Mapeia operador lógico para C */
static const char *mapear_op_logico(const char *op) {
    if (strcmp(op, "ET")  == 0 || strcmp(op, "AND") == 0) return "&&";
    if (strcmp(op, "OU")  == 0 || strcmp(op, "OR")  == 0) return "||";
    if (strcmp(op, "NON") == 0 || strcmp(op, "NOT") == 0) return "!";
    return op;
}

/* Mapeia operador aritmético para C; retorna NULL se não for aritmético */
static const char *mapear_op_aritmetico(const char *op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "ADD")  == 0) return "+";
    if (strcmp(op, "-") == 0 || strcmp(op, "SUB")  == 0) return "-";
    if (strcmp(op, "*") == 0 || strcmp(op, "MULT") == 0) return "*";
    if (strcmp(op, "/") == 0 || strcmp(op, "DIV")  == 0) return "/";
    if (strcmp(op, "%") == 0 || strcmp(op, "PCM")  == 0) return "%";
    return NULL;
}

/* ─────────────────────────────────────────────
   Expressão aritmética:  valor (op valor)*
   Suporta cadeias como:  a + b * c
   ───────────────────────────────────────────── */
static void gen_expr(GenCtx *ctx) {
    /* Emite pelo menos um operando */
    if (!tem_proximo(ctx)) return;
    fprintf(ctx->out, "%s", consumir(ctx)->name);

    /* Continua enquanto o próximo nó folha for operador aritmético */
    while (tem_proximo(ctx)) {
        const char *op_c = mapear_op_aritmetico(atual(ctx)->name);
        if (!op_c) break;
        consumir(ctx); /* consome o operador */
        fprintf(ctx->out, " %s ", op_c);
        if (tem_proximo(ctx)) {
            fprintf(ctx->out, "%s", consumir(ctx)->name);
        }
    }
}

/* ─────────────────────────────────────────────
   Folhas de condição: emite os tokens dentro de um bloco
   condicoes/condicao convertendo cada um para C.
   Percorre até profundidade <= profundidade_cond.
   ───────────────────────────────────────────── */
static void gen_condicoes_folhas(GenCtx *ctx, int profundidade_cond) {
    while (tem_proximo(ctx) && atual(ctx)->depth > profundidade_cond) {
        const char *nome = consumir(ctx)->name;

        /* Ignora nós estruturais (não são folhas de valor) */
        if (strcmp(nome, "condicoes")        == 0 ||
            strcmp(nome, "condicao")         == 0 ||
            strcmp(nome, "id")               == 0 ||
            strcmp(nome, "numero")           == 0 ||
            strcmp(nome, "operador")         == 0 ||
            strcmp(nome, "agregador_logico") == 0) {
            continue;
        }

        /* Parênteses */
        if (strcmp(nome, "(") == 0 || strcmp(nome, "ouvrirPAREN") == 0) {
            fprintf(ctx->out, "("); continue;
        }
        if (strcmp(nome, ")") == 0 || strcmp(nome, "fermerPAREN") == 0) {
            fprintf(ctx->out, ")"); continue;
        }

        /* Operadores lógicos */
        const char *logop = mapear_op_logico(nome);
        if (strcmp(logop, nome) != 0) {
            fprintf(ctx->out, " %s ", logop); continue;
        }

        /* Operadores relacionais */
        const char *relop = mapear_op_relacional(nome);
        if (strcmp(relop, nome) != 0) {
            fprintf(ctx->out, " %s ", relop); continue;
        }

        /* Identificador ou número literal */
        fprintf(ctx->out, "%s", nome);
    }
}

/* ─────────────────────────────────────────────
   Bloco:  ouvrir ... fermer  →  { ... }
   ───────────────────────────────────────────── */
static void gen_bloco(GenCtx *ctx) {
    /* Consome o token de abertura "ouvrir" */
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "ouvrir") == 0 ||
                              strcmp(atual(ctx)->name, "OPEN")   == 0)) {
        consumir(ctx);
    }

    emit_indent(ctx);
    fprintf(ctx->out, "{\n");
    ctx->indent++;

    /* Percorre os filhos até encontrar "fermer" / "CLOSE" */
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "fermer") == 0 || strcmp(nome, "CLOSE") == 0) {
            consumir(ctx); /* consome o fechamento */
            break;
        }
        /* Nós estruturais são ignorados; processa o conteúdo deles */
        if (strcmp(nome, "bloco")   == 0 ||
            strcmp(nome, "comando") == 0) {
            consumir(ctx);
            continue;
        }
        gen_comando(ctx);
    }

    ctx->indent--;
    emit_indent(ctx);
    fprintf(ctx->out, "}\n");
}

/* ─────────────────────────────────────────────
   Print:  afficher(x)  →  printf("%d\n", x);
   Usa %d fixo pois não há tabela de tipos implementada.
   ───────────────────────────────────────────── */
static void gen_print(GenCtx *ctx) {
    const char *variavel = "";

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        /* Pula estruturais e parêntese de abertura */
        if (strcmp(nome, "(") == 0 || strcmp(nome, "ouvrirPAREN") == 0 ||
            strcmp(nome, "id") == 0 || strcmp(nome, "numero") == 0) {
            consumir(ctx); continue;
        }
        /* Parêntese de fechamento encerra a coleta */
        if (strcmp(nome, ")") == 0 || strcmp(nome, "fermerPAREN") == 0) {
            consumir(ctx); break;
        }
        variavel = consumir(ctx)->name;
    }

    emit_indent(ctx);
    fprintf(ctx->out, "printf(\"%%d\\n\", %s);\n", variavel);
}

/* ─────────────────────────────────────────────
   Input:  saisir(x)  →  scanf("%d", &x);
   ───────────────────────────────────────────── */
static void gen_input(GenCtx *ctx) {
    const char *variavel = "";

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "(") == 0 || strcmp(nome, "ouvrirPAREN") == 0 ||
            strcmp(nome, "id") == 0) {
            consumir(ctx); continue;
        }
        if (strcmp(nome, ")") == 0 || strcmp(nome, "fermerPAREN") == 0) {
            consumir(ctx); break;
        }
        variavel = consumir(ctx)->name;
    }

    emit_indent(ctx);
    fprintf(ctx->out, "scanf(\"%%d\", &%s);\n", variavel);
}

/* ─────────────────────────────────────────────
   Atribuição:  id = expr  (suporta expressões aritméticas)
   Também trata  id++  e  id--
   ───────────────────────────────────────────── */
static void gen_atribuicao(GenCtx *ctx) {
    /* Pula wrapper "id" se presente */
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);

    const char *lhs = "";
    if (tem_proximo(ctx)) lhs = consumir(ctx)->name;
    if (!tem_proximo(ctx)) return;

    const char *proximo = atual(ctx)->name;

    /* Incremento pós-fixado:  id++ */
    if (strcmp(proximo, "++") == 0 || strcmp(proximo, "PLUSPLUS") == 0) {
        consumir(ctx);
        emit_indent(ctx);
        fprintf(ctx->out, "%s++;\n", lhs);
        return;
    }

    /* Decremento pós-fixado:  id-- */
    if (strcmp(proximo, "--") == 0 || strcmp(proximo, "MOINSMOINS") == 0) {
        consumir(ctx);
        emit_indent(ctx);
        fprintf(ctx->out, "%s--;\n", lhs);
        return;
    }

    /* Atribuição com expressão:  id = expr */
    if (strcmp(proximo, "=") == 0 || strcmp(proximo, "IGUAL") == 0) {
        consumir(ctx); /* consome o '=' */

        /* Pula wrapper operadorAtribuicao se presente */
        if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operadorAtribuicao") == 0)
            consumir(ctx);

        /* Pula wrappers id/numero antes da expressão */
        while (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                     strcmp(atual(ctx)->name, "numero") == 0)) {
            consumir(ctx);
        }

        emit_indent(ctx);
        fprintf(ctx->out, "%s = ", lhs);
        gen_expr(ctx);
        fprintf(ctx->out, ";\n");
        return;
    }
}

/* ─────────────────────────────────────────────
   IF:  si(cond) bloco  →  if (cond) { ... }
   ───────────────────────────────────────────── */
static void gen_if(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "if (");

    /* Avança até achar o nó de condição ou o início do bloco */
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicoes") == 0 || strcmp(nome, "condicao") == 0) {
            int prof_cond = atual(ctx)->depth;
            consumir(ctx);
            gen_condicoes_folhas(ctx, prof_cond);
            break;
        }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx); /* pula tokens estruturais: IF lexema, parênteses */
    }

    fprintf(ctx->out, ")\n");

    /* Pula wrapper do bloco e gera o corpo */
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

/* ─────────────────────────────────────────────
   ELSE IF:  sinon_si(cond) bloco  →  else if (cond) { ... }
   ───────────────────────────────────────────── */
static void gen_ifelse(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "else if (");

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicoes") == 0 || strcmp(nome, "condicao") == 0) {
            int prof_cond = atual(ctx)->depth;
            consumir(ctx);
            gen_condicoes_folhas(ctx, prof_cond);
            break;
        }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    fprintf(ctx->out, ")\n");

    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

/* ─────────────────────────────────────────────
   ELSE:  sinon bloco  →  else { ... }
   ───────────────────────────────────────────── */
static void gen_else(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "else\n");

    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

/* ─────────────────────────────────────────────
   WHILE:  alors_que(cond) bloco  →  while (cond) { ... }
   ───────────────────────────────────────────── */
static void gen_while(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "while (");

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicoes") == 0 || strcmp(nome, "condicao") == 0) {
            int prof_cond = atual(ctx)->depth;
            consumir(ctx);
            gen_condicoes_folhas(ctx, prof_cond);
            break;
        }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    fprintf(ctx->out, ")\n");

    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

/* ─────────────────────────────────────────────
   FOR:
   pour(tipo id = num ET cond ET incr) bloco
   →  for (tipo id = num; cond; incr) { ... }
   ───────────────────────────────────────────── */
static void gen_for(GenCtx *ctx) {
    /* --- Tipo da variável de controle --- */
    const char *c_tipo = "int";
    while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "tipoVariavel") != 0 &&
           strcmp(atual(ctx)->name, "bloco") != 0) {
        consumir(ctx);
    }
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "tipoVariavel") == 0) {
        consumir(ctx); /* pula rótulo tipoVariavel */
        if (tem_proximo(ctx)) c_tipo = mapear_tipo(consumir(ctx)->name);
    }

    /* --- Identificador de controle --- */
    while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") != 0 &&
           strcmp(atual(ctx)->name, "bloco") != 0) {
        consumir(ctx);
    }
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);
    const char *var = tem_proximo(ctx) ? consumir(ctx)->name : "i";

    /* --- Valor inicial (após o '=') --- */
    while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operadorAtribuicao") != 0 &&
           strcmp(atual(ctx)->name, "bloco") != 0) {
        consumir(ctx);
    }
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operadorAtribuicao") == 0) consumir(ctx);
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "=") == 0 ||
                              strcmp(atual(ctx)->name, "IGUAL") == 0)) consumir(ctx);
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "numero") == 0) consumir(ctx);
    const char *val_inicial = tem_proximo(ctx) ? consumir(ctx)->name : "0";

    /* --- Separador ET entre init e condição --- */
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "ET") == 0 ||
                              strcmp(atual(ctx)->name, "AND") == 0)) consumir(ctx);

    /* --- Condição de parada --- */
    char buf_cond[1024] = "";
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicao") == 0) {
            consumir(ctx); /* pula rótulo */

            /* operando esquerdo */
            while (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                         strcmp(atual(ctx)->name, "numero") == 0)) consumir(ctx);
            const char *esq = tem_proximo(ctx) ? consumir(ctx)->name : "";

            /* operador relacional */
            while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operador") == 0) consumir(ctx);
            const char *rel = tem_proximo(ctx) ? mapear_op_relacional(consumir(ctx)->name) : "==";

            /* operando direito */
            while (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                         strcmp(atual(ctx)->name, "numero") == 0)) consumir(ctx);
            const char *dir = tem_proximo(ctx) ? consumir(ctx)->name : "";

            snprintf(buf_cond, sizeof(buf_cond), "%s %s %s", esq, rel, dir);
            break;
        }
        if (strcmp(nome, "AND") == 0 || strcmp(nome, "ET") == 0) break;
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    /* --- Separador ET entre condição e incremento --- */
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "ET") == 0 ||
                              strcmp(atual(ctx)->name, "AND") == 0)) consumir(ctx);

    /* --- Incremento/decremento --- */
    char buf_incr[512] = "";
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "incrimento") == 0) {
            consumir(ctx); /* pula rótulo */

            /* Prefixo:  ++i  ou  --i */
            if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "PLUSPLUS")   == 0 ||
                                      strcmp(atual(ctx)->name, "MOINSMOINS") == 0 ||
                                      strcmp(atual(ctx)->name, "++") == 0 ||
                                      strcmp(atual(ctx)->name, "--") == 0)) {
                const char *op  = consumir(ctx)->name;
                const char *cop = (strcmp(op, "PLUSPLUS") == 0 || strcmp(op, "++") == 0) ? "++" : "--";
                if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);
                const char *iv = tem_proximo(ctx) ? consumir(ctx)->name : var;
                snprintf(buf_incr, sizeof(buf_incr), "%s%s", iv, cop);
            } else {
                /* Posfixo:  i++  ou  i-- */
                if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);
                const char *iv = tem_proximo(ctx) ? consumir(ctx)->name : var;
                if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "PLUSPLUS")   == 0 ||
                                          strcmp(atual(ctx)->name, "MOINSMOINS") == 0 ||
                                          strcmp(atual(ctx)->name, "++") == 0 ||
                                          strcmp(atual(ctx)->name, "--") == 0)) {
                    const char *op  = consumir(ctx)->name;
                    const char *cop = (strcmp(op, "PLUSPLUS") == 0 || strcmp(op, "++") == 0) ? "++" : "--";
                    snprintf(buf_incr, sizeof(buf_incr), "%s%s", iv, cop);
                }
            }
            break;
        }
        if (strcmp(nome, ")") == 0 || strcmp(nome, "fermerPAREN") == 0) { consumir(ctx); break; }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    /* Consome parêntese de fechamento se ainda presente */
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, ")") == 0 ||
                              strcmp(atual(ctx)->name, "fermerPAREN") == 0)) {
        consumir(ctx);
    }

    emit_indent(ctx);
    fprintf(ctx->out, "for (%s %s = %s; %s; %s)\n",
            c_tipo, var, val_inicial, buf_cond, buf_incr);

    /* Gera o corpo do for */
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

/* ─────────────────────────────────────────────
   Despacha um único nó de comando
   ───────────────────────────────────────────── */
static void gen_comando(GenCtx *ctx) {
    if (!tem_proximo(ctx)) return;

    const char *nome = atual(ctx)->name;

    /* Pula nós puramente estruturais */
    if (strcmp(nome, "comando")  == 0 || strcmp(nome, "programa") == 0 ||
        strcmp(nome, "main")     == 0) {
        consumir(ctx);
        return;
    }

    /* Despacha para o gerador correto de acordo com o tipo de comando */
    if (strcmp(nome, "IF")         == 0) { consumir(ctx); gen_if(ctx);        return; }
    if (strcmp(nome, "ifelse")     == 0) { consumir(ctx); gen_ifelse(ctx);    return; }
    if (strcmp(nome, "ELSE")       == 0) { consumir(ctx); gen_else(ctx);      return; }
    if (strcmp(nome, "WHILE")      == 0) { consumir(ctx); gen_while(ctx);     return; }
    if (strcmp(nome, "FOR")        == 0) { consumir(ctx); gen_for(ctx);       return; }
    if (strcmp(nome, "print")      == 0) { consumir(ctx); gen_print(ctx);     return; }
    if (strcmp(nome, "input")      == 0) { consumir(ctx); gen_input(ctx);     return; }
    if (strcmp(nome, "atribuicao") == 0) { consumir(ctx); gen_atribuicao(ctx); return; }

    /* bloco é tratado pelos callers; não consumir aqui */
    if (strcmp(nome, "bloco") == 0) return;

    /* Nó desconhecido — consome e ignora */
    consumir(ctx);
}

/* ─────────────────────────────────────────────
   Percorre o nível "programa" até EOF
   ───────────────────────────────────────────── */
static void gen_programa(GenCtx *ctx) {
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;

        /* Marcador de fim do programa */
        if (strcmp(nome, "EOF") == 0 || strcmp(nome, "FIN") == 0) {
            consumir(ctx); break;
        }

        /* Nós estruturais de alto nível são pulados */
        if (strcmp(nome, "main")     == 0 || strcmp(nome, "programa") == 0 ||
            strcmp(nome, "comando")  == 0) {
            consumir(ctx); continue;
        }

        gen_comando(ctx);
    }
}

/* ─────────────────────────────────────────────
   Ponto de entrada público do gerador de código
   ───────────────────────────────────────────── */
int codegen_run(const ASTree *tree, FILE *out) {
    GenCtx ctx = { tree, 0, 0, out };

    /* Cabeçalho padrão C */
    fprintf(out, "#include <stdio.h>\n\n");
    fprintf(out, "int main(void) {\n");
    ctx.indent = 1;

    /* Pula os nós raiz "main" e "programa" antes de processar comandos */
    while (tem_proximo(&ctx)) {
        const char *nome = atual(&ctx)->name;
        if (strcmp(nome, "main") == 0 || strcmp(nome, "programa") == 0) {
            consumir(&ctx); continue;
        }
        break;
    }

    gen_programa(&ctx);

    /* Rodapé padrão C — pausa para o usuário ver a saída antes de fechar */
    fprintf(out, "\n    printf(\"\\nPressione ENTER para sair...\");\n");
    fprintf(out, "    getchar();\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    return 0;
}
