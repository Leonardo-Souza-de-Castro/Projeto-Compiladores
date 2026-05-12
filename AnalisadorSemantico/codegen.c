#include "codegen.h"
#include "node.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIMBOLOS 256

typedef struct { char nome[MAX_NAME_LEN]; char tipo[MAX_NAME_LEN]; } Simbolo;
typedef struct { Simbolo entradas[MAX_SIMBOLOS]; int count; } SymTable;

typedef struct {
    const ASTree *tree;
    int           pos;
    int           indent;
    FILE         *out;
    SymTable      st;
} GenCtx;

static void        emit_indent(GenCtx *ctx);
static void        gen_programa(GenCtx *ctx);
static void        gen_comando(GenCtx *ctx);
static void        gen_if(GenCtx *ctx);
static void        gen_ifelse(GenCtx *ctx);
static void        gen_else(GenCtx *ctx);
static void        gen_while(GenCtx *ctx);
static void        gen_for(GenCtx *ctx);
static void        gen_declaracao(GenCtx *ctx);
static void        gen_print(GenCtx *ctx);
static void        gen_input(GenCtx *ctx);
static void        gen_atribuicao(GenCtx *ctx);
static void        gen_bloco(GenCtx *ctx);
static void        gen_condicoes_folhas(GenCtx *ctx, int profundidade_cond);
static void        gen_expr(GenCtx *ctx);
static const char *mapear_tipo(const char *tipo);
static const char *mapear_op_relacional(const char *op);
static const char *mapear_op_logico(const char *op);
static const char *mapear_op_aritmetico(const char *op);
static const char *formato_io(const char *tipo_c);

static inline int tem_proximo(GenCtx *ctx) {
    return ctx->pos < ctx->tree->count;
}

static inline const ASTNode *atual(GenCtx *ctx) {
    return &ctx->tree->nodes[ctx->pos];
}

static inline const ASTNode *consumir(GenCtx *ctx) {
    return &ctx->tree->nodes[ctx->pos++];
}

static void emit_indent(GenCtx *ctx) {
    for (int i = 0; i < ctx->indent; i++) fprintf(ctx->out, "    ");
}

static void symtable_inserir(SymTable *st, const char *nome, const char *tipo) {
    if (st->count >= MAX_SIMBOLOS) return;
    strncpy(st->entradas[st->count].nome, nome, MAX_NAME_LEN - 1);
    st->entradas[st->count].nome[MAX_NAME_LEN - 1] = '\0';
    strncpy(st->entradas[st->count].tipo, tipo, MAX_NAME_LEN - 1);
    st->entradas[st->count].tipo[MAX_NAME_LEN - 1] = '\0';
    st->count++;
}

static const char *symtable_buscar(const SymTable *st, const char *nome) {
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->entradas[i].nome, nome) == 0)
            return st->entradas[i].tipo;
    }
    return NULL;
}

static const char *mapear_tipo(const char *tipo) {
    if (strcmp(tipo, "entier")     == 0) return "int";
    if (strcmp(tipo, "flotter")    == 0) return "float";
    if (strcmp(tipo, "doubler")    == 0) return "double";
    if (strcmp(tipo, "chaine")     == 0) return "char*";
    if (strcmp(tipo, "personnage") == 0) return "char";
    if (strcmp(tipo, "logique")    == 0) return "int";
    return tipo;
}

static const char *mapear_op_relacional(const char *op) {
    if (strcmp(op, "{__") == 0 || strcmp(op, "IGUAL_MAIOR") == 0) return ">=";
    if (strcmp(op, "}__") == 0 || strcmp(op, "IGUAL_MENOR") == 0) return "<=";
    if (strcmp(op, "__")  == 0 || strcmp(op, "IGUALDADE")   == 0) return "==";
    if (strcmp(op, "{")   == 0 || strcmp(op, "MAIOR")       == 0) return ">";
    if (strcmp(op, "}")   == 0 || strcmp(op, "MENOR")       == 0) return "<";
    return op;
}

static const char *mapear_op_logico(const char *op) {
    if (strcmp(op, "ET")  == 0 || strcmp(op, "AND") == 0) return "&&";
    if (strcmp(op, "OU")  == 0 || strcmp(op, "OR")  == 0) return "||";
    if (strcmp(op, "NON") == 0 || strcmp(op, "NOT") == 0) return "!";
    return op;
}

static const char *mapear_op_aritmetico(const char *op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "ADD")  == 0) return "+";
    if (strcmp(op, "-") == 0 || strcmp(op, "SUB")  == 0) return "-";
    if (strcmp(op, "*") == 0 || strcmp(op, "MULT") == 0) return "*";
    if (strcmp(op, "/") == 0 || strcmp(op, "DIV")  == 0) return "/";
    if (strcmp(op, "%") == 0 || strcmp(op, "PCM")  == 0) return "%";
    return NULL;
}

static const char *formato_io(const char *tipo_c) {
    if (strcmp(tipo_c, "float")  == 0) return "%f";
    if (strcmp(tipo_c, "double") == 0) return "%lf";
    if (strcmp(tipo_c, "char")   == 0) return "%c";
    if (strcmp(tipo_c, "char*")  == 0) return "%s";
    return "%d";
}

static void gen_expr(GenCtx *ctx) {
    if (!tem_proximo(ctx)) return;
    fprintf(ctx->out, "%s", consumir(ctx)->name);

    while (tem_proximo(ctx)) {
        const char *op_c = mapear_op_aritmetico(atual(ctx)->name);
        if (!op_c) break;
        consumir(ctx);
        fprintf(ctx->out, " %s ", op_c);
        /* o parser embrulha operandos em nos "id" ou "numero" — descarta o envelope */
        if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                  strcmp(atual(ctx)->name, "numero") == 0))
            consumir(ctx);
        if (tem_proximo(ctx))
            fprintf(ctx->out, "%s", consumir(ctx)->name);
    }
}

static void gen_condicoes_folhas(GenCtx *ctx, int profundidade_cond) {
    while (tem_proximo(ctx) && atual(ctx)->depth > profundidade_cond) {
        /* bloco e ouvrir ficam na mesma profundidade que condicao mas pertencem ao corpo */
        const char *prox = atual(ctx)->name;
        if (strcmp(prox, "bloco")  == 0 || strcmp(prox, "ouvrir") == 0 ||
            strcmp(prox, "OPEN")   == 0 || strcmp(prox, "fermer") == 0 ||
            strcmp(prox, "CLOSE")  == 0) break;

        const char *nome = consumir(ctx)->name;

        if (strcmp(nome, "condicoes")        == 0 ||
            strcmp(nome, "condicao")         == 0 ||
            strcmp(nome, "id")               == 0 ||
            strcmp(nome, "numero")           == 0 ||
            strcmp(nome, "operador")         == 0 ||
            strcmp(nome, "agregador_logico") == 0) continue;

        if (strcmp(nome, "(") == 0 || strcmp(nome, "ouvrirPAREN") == 0) {
            fprintf(ctx->out, "("); continue;
        }
        if (strcmp(nome, ")") == 0 || strcmp(nome, "fermerPAREN") == 0) {
            fprintf(ctx->out, ")"); continue;
        }

        const char *logop = mapear_op_logico(nome);
        if (strcmp(logop, nome) != 0) { fprintf(ctx->out, " %s ", logop); continue; }

        const char *relop = mapear_op_relacional(nome);
        if (strcmp(relop, nome) != 0) { fprintf(ctx->out, " %s ", relop); continue; }

        fprintf(ctx->out, "%s", nome);
    }
}

static void gen_bloco(GenCtx *ctx) {
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "ouvrir") == 0 ||
                              strcmp(atual(ctx)->name, "OPEN")   == 0))
        consumir(ctx);

    emit_indent(ctx);
    fprintf(ctx->out, "{\n");
    ctx->indent++;

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "fermer") == 0 || strcmp(nome, "CLOSE") == 0) {
            consumir(ctx);
            break;
        }
        if (strcmp(nome, "bloco") == 0 || strcmp(nome, "comando") == 0) {
            consumir(ctx);
            continue;
        }
        gen_comando(ctx);
    }

    ctx->indent--;
    emit_indent(ctx);
    fprintf(ctx->out, "}\n");
}

static void gen_print(GenCtx *ctx) {
    const char *variavel = "";

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "(") == 0 || strcmp(nome, "ouvrirPAREN") == 0 ||
            strcmp(nome, "id") == 0 || strcmp(nome, "numero") == 0) {
            consumir(ctx); continue;
        }
        if (strcmp(nome, ")") == 0 || strcmp(nome, "fermerPAREN") == 0) {
            consumir(ctx); break;
        }
        variavel = consumir(ctx)->name;
    }

    const char *tipo = symtable_buscar(&ctx->st, variavel);
    const char *fmt  = tipo ? formato_io(tipo) : "%d";

    emit_indent(ctx);
    fprintf(ctx->out, "printf(\"%s\\n\", %s);\n", fmt, variavel);
}

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

    const char *tipo = symtable_buscar(&ctx->st, variavel);
    const char *fmt  = tipo ? formato_io(tipo) : "%d";

    emit_indent(ctx);
    fprintf(ctx->out, "scanf(\"%s\", &%s);\n", fmt, variavel);
}

static void gen_atribuicao(GenCtx *ctx) {
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);

    const char *lhs = "";
    if (tem_proximo(ctx)) lhs = consumir(ctx)->name;
    if (!tem_proximo(ctx)) return;

    const char *prox = atual(ctx)->name;

    if (strcmp(prox, "++") == 0 || strcmp(prox, "PLUSPLUS") == 0) {
        consumir(ctx);
        emit_indent(ctx);
        fprintf(ctx->out, "%s++;\n", lhs);
        return;
    }

    if (strcmp(prox, "--") == 0 || strcmp(prox, "MOINSMOINS") == 0) {
        consumir(ctx);
        emit_indent(ctx);
        fprintf(ctx->out, "%s--;\n", lhs);
        return;
    }

    if (strcmp(prox, "operadorAtribuicao") == 0 ||
        strcmp(prox, "=") == 0 || strcmp(prox, "IGUAL") == 0) {

        if (strcmp(prox, "operadorAtribuicao") == 0) consumir(ctx);
        if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "=")    == 0 ||
                                  strcmp(atual(ctx)->name, "IGUAL") == 0))
            consumir(ctx);

        /* descarta envelope do primeiro operando */
        if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                  strcmp(atual(ctx)->name, "numero") == 0))
            consumir(ctx);

        emit_indent(ctx);
        fprintf(ctx->out, "%s = ", lhs);
        gen_expr(ctx);
        fprintf(ctx->out, ";\n");
    }
}

static void gen_if(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "if (");

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicoes") == 0 || strcmp(nome, "condicao") == 0) {
            int prof = atual(ctx)->depth;
            consumir(ctx);
            gen_condicoes_folhas(ctx, prof);
            break;
        }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    fprintf(ctx->out, ")\n");
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

static void gen_ifelse(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "else if (");

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicoes") == 0 || strcmp(nome, "condicao") == 0) {
            int prof = atual(ctx)->depth;
            consumir(ctx);
            gen_condicoes_folhas(ctx, prof);
            break;
        }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    fprintf(ctx->out, ")\n");
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

static void gen_else(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "else\n");
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

static void gen_while(GenCtx *ctx) {
    emit_indent(ctx);
    fprintf(ctx->out, "while (");

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicoes") == 0 || strcmp(nome, "condicao") == 0) {
            int prof = atual(ctx)->depth;
            consumir(ctx);
            gen_condicoes_folhas(ctx, prof);
            break;
        }
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    fprintf(ctx->out, ")\n");
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

static void gen_for(GenCtx *ctx) {
    const char *c_tipo = "int";
    while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "tipoVariavel") != 0 &&
           strcmp(atual(ctx)->name, "bloco") != 0)
        consumir(ctx);
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "tipoVariavel") == 0) {
        consumir(ctx);
        if (tem_proximo(ctx)) c_tipo = mapear_tipo(consumir(ctx)->name);
    }

    while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") != 0 &&
           strcmp(atual(ctx)->name, "bloco") != 0)
        consumir(ctx);
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);
    const char *var = tem_proximo(ctx) ? consumir(ctx)->name : "i";

    while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operadorAtribuicao") != 0 &&
           strcmp(atual(ctx)->name, "bloco") != 0)
        consumir(ctx);
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operadorAtribuicao") == 0) consumir(ctx);
    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "=") == 0 ||
                              strcmp(atual(ctx)->name, "IGUAL") == 0)) consumir(ctx);
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "numero") == 0) consumir(ctx);
    const char *val_ini = tem_proximo(ctx) ? consumir(ctx)->name : "0";

    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "ET") == 0 ||
                              strcmp(atual(ctx)->name, "AND") == 0)) consumir(ctx);

    char buf_cond[1024] = "";
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "condicao") == 0) {
            consumir(ctx);
            while (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                         strcmp(atual(ctx)->name, "numero") == 0)) consumir(ctx);
            const char *esq = tem_proximo(ctx) ? consumir(ctx)->name : "";
            while (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operador") == 0) consumir(ctx);
            const char *rel = tem_proximo(ctx) ? mapear_op_relacional(consumir(ctx)->name) : "==";
            while (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                         strcmp(atual(ctx)->name, "numero") == 0)) consumir(ctx);
            const char *dir = tem_proximo(ctx) ? consumir(ctx)->name : "";
            snprintf(buf_cond, sizeof(buf_cond), "%s %s %s", esq, rel, dir);
            break;
        }
        if (strcmp(nome, "ET") == 0 || strcmp(nome, "AND") == 0) break;
        if (strcmp(nome, "bloco") == 0) break;
        consumir(ctx);
    }

    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "ET") == 0 ||
                              strcmp(atual(ctx)->name, "AND") == 0)) consumir(ctx);

    char buf_incr[512] = "";
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "incrimento") == 0) {
            consumir(ctx);
            if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "PLUSPLUS")   == 0 ||
                                      strcmp(atual(ctx)->name, "MOINSMOINS") == 0 ||
                                      strcmp(atual(ctx)->name, "++")         == 0 ||
                                      strcmp(atual(ctx)->name, "--")         == 0)) {
                const char *op  = consumir(ctx)->name;
                const char *cop = (strcmp(op, "PLUSPLUS") == 0 || strcmp(op, "++") == 0) ? "++" : "--";
                if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);
                const char *iv = tem_proximo(ctx) ? consumir(ctx)->name : var;
                snprintf(buf_incr, sizeof(buf_incr), "%s%s", iv, cop);
            } else {
                if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "id") == 0) consumir(ctx);
                const char *iv = tem_proximo(ctx) ? consumir(ctx)->name : var;
                if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "PLUSPLUS")   == 0 ||
                                          strcmp(atual(ctx)->name, "MOINSMOINS") == 0 ||
                                          strcmp(atual(ctx)->name, "++")         == 0 ||
                                          strcmp(atual(ctx)->name, "--")         == 0)) {
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

    if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, ")") == 0 ||
                              strcmp(atual(ctx)->name, "fermerPAREN") == 0))
        consumir(ctx);

    emit_indent(ctx);
    fprintf(ctx->out, "for (%s %s = %s; %s; %s)\n", c_tipo, var, val_ini, buf_cond, buf_incr);

    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "bloco") == 0) consumir(ctx);
    gen_bloco(ctx);
}

static void gen_declaracao(GenCtx *ctx) {
    const char *c_tipo = "int";
    char var[MAX_NAME_LEN] = "";

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "tipoVariavel") == 0) {
            consumir(ctx);
            if (tem_proximo(ctx)) c_tipo = mapear_tipo(consumir(ctx)->name);
            break;
        }
        if (strcmp(nome, "id") == 0 || strcmp(nome, "operadorAtribuicao") == 0) break;
        consumir(ctx);
    }

    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "id") == 0) {
            consumir(ctx);
            if (tem_proximo(ctx)) strncpy(var, consumir(ctx)->name, MAX_NAME_LEN - 1);
            break;
        }
        if (strcmp(nome, "operadorAtribuicao") == 0) break;
        consumir(ctx);
    }

    int tem_valor = 0;
    if (tem_proximo(ctx) && strcmp(atual(ctx)->name, "operadorAtribuicao") == 0) {
        consumir(ctx);
        if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "=")    == 0 ||
                                  strcmp(atual(ctx)->name, "IGUAL") == 0))
            consumir(ctx);
        tem_valor = 1;
    }

    if (var[0] != '\0' && symtable_buscar(&ctx->st, var) != NULL)
        fprintf(stderr, "Aviso semantico: variavel '%s' ja declarada\n", var);
    else if (var[0] != '\0')
        symtable_inserir(&ctx->st, var, c_tipo);

    emit_indent(ctx);
    if (tem_valor) {
        fprintf(ctx->out, "%s %s = ", c_tipo, var);
        if (tem_proximo(ctx) && (strcmp(atual(ctx)->name, "id")     == 0 ||
                                  strcmp(atual(ctx)->name, "numero") == 0))
            consumir(ctx);
        gen_expr(ctx);
        fprintf(ctx->out, ";\n");
    } else {
        fprintf(ctx->out, "%s %s;\n", c_tipo, var);
    }
}

static void gen_comando(GenCtx *ctx) {
    if (!tem_proximo(ctx)) return;

    const char *nome = atual(ctx)->name;

    if (strcmp(nome, "comando") == 0 || strcmp(nome, "programa") == 0 ||
        strcmp(nome, "main")    == 0) {
        consumir(ctx);
        return;
    }

    if (strcmp(nome, "IF")        == 0) { consumir(ctx); gen_if(ctx);         return; }
    if (strcmp(nome, "ifelse")    == 0) { consumir(ctx); gen_ifelse(ctx);     return; }
    if (strcmp(nome, "ELSE")      == 0) { consumir(ctx); gen_else(ctx);       return; }
    if (strcmp(nome, "WHILE")     == 0) { consumir(ctx); gen_while(ctx);      return; }
    if (strcmp(nome, "FOR")       == 0) { consumir(ctx); gen_for(ctx);        return; }
    if (strcmp(nome, "print")     == 0) { consumir(ctx); gen_print(ctx);      return; }
    if (strcmp(nome, "input")     == 0) { consumir(ctx); gen_input(ctx);      return; }
    if (strcmp(nome, "atribuicao") == 0) { consumir(ctx); gen_atribuicao(ctx); return; }
    if (strcmp(nome, "declaracao") == 0) { consumir(ctx); gen_declaracao(ctx); return; }

    if (strcmp(nome, "bloco") == 0) return;

    consumir(ctx);
}

static void gen_programa(GenCtx *ctx) {
    while (tem_proximo(ctx)) {
        const char *nome = atual(ctx)->name;
        if (strcmp(nome, "EOF") == 0 || strcmp(nome, "FIN") == 0) {
            consumir(ctx); break;
        }
        if (strcmp(nome, "main") == 0 || strcmp(nome, "programa") == 0 ||
            strcmp(nome, "comando") == 0) {
            consumir(ctx); continue;
        }
        gen_comando(ctx);
    }
}

int codegen_run(const ASTree *tree, FILE *out) {
    GenCtx ctx = { .tree = tree, .pos = 0, .indent = 0, .out = out };

    fprintf(out, "#include <stdio.h>\n\n");
    fprintf(out, "int main(void) {\n");
    ctx.indent = 1;

    while (tem_proximo(&ctx)) {
        const char *nome = atual(&ctx)->name;
        if (strcmp(nome, "main") == 0 || strcmp(nome, "programa") == 0) {
            consumir(&ctx); continue;
        }
        break;
    }

    gen_programa(&ctx);

    fprintf(out, "\n    printf(\"\\nPressione ENTER para sair...\");\n");
    fprintf(out, "    getchar();\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    return 0;
}
