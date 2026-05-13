#include "node.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Analisa uma linha no formato "profundidade:nomeDoNo" e preenche *out.
 * Retorna 0 em sucesso, -1 se a linha não possui o separador ':'.
 */
static int parse_line(const char *line, ASTNode *out) {
    /* Localiza o primeiro ':' que separa profundidade do nome */
    const char *colon = strchr(line, ':');
    if (!colon) return -1;

    out->depth = atoi(line);

    /* Copia tudo após ':' para o campo name, removendo \r\n no final */
    strncpy(out->name, colon + 1, MAX_NAME_LEN - 1);
    out->name[MAX_NAME_LEN - 1] = '\0';

    size_t len = strlen(out->name);
    while (len > 0 && (out->name[len - 1] == '\n' || out->name[len - 1] == '\r')) {
        out->name[--len] = '\0';
    }
    return 0;
}

/* Carrega todos os nós do arquivo de AST no vetor tree->nodes */
int ast_load(ASTree *tree, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", path);
        return -1;
    }

    tree->count = 0;
    char line[MAX_NAME_LEN + 16];

    while (fgets(line, sizeof(line), f)) {
        if (tree->count >= MAX_NODES) {
            fprintf(stderr, "Aviso: limite de nos atingido (%d)\n", MAX_NODES);
            break;
        }

        /* Ignora linhas em branco */
        if (line[0] == '\n' || line[0] == '\r') continue;

        ASTNode *node = &tree->nodes[tree->count];
        if (parse_line(line, node) == 0) {
            tree->count++;
        }
    }

    fclose(f);
    return 0;
}

/* Imprime os nós carregados (usado para depuração) */
void ast_print(const ASTree *tree) {
    for (int i = 0; i < tree->count; i++) {
        printf("%d:%s\n", tree->nodes[i].depth, tree->nodes[i].name);
    }
}
