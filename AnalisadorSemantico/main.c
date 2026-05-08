#include "node.h"
#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Nome padrão do arquivo de saída gerado */
#define SAIDA_PADRAO "saida.c"

int main(int argc, char *argv[]) {
    /* Determina o arquivo de entrada da AST */
    const char *entrada = "resultado_sintatico.txt";
    const char *saida   = SAIDA_PADRAO;

    if (argc >= 2) entrada = argv[1];
    if (argc >= 3) saida   = argv[2];

    printf("Analisador Semantico\n");
    printf("Entrada : %s\n", entrada);
    printf("Saida   : %s\n\n", saida);

    /* Carrega a AST exportada pelo analisador sintatico Java */
    ASTree tree;
    if (ast_load(&tree, entrada) != 0) {
        fprintf(stderr, "Erro: nao foi possivel carregar a AST de '%s'\n", entrada);
        return EXIT_FAILURE;
    }

    printf("Nos carregados: %d\n\n", tree.count);

    /* Abre o arquivo de saida .c */
    FILE *out = fopen(saida, "w");
    if (!out) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'\n", saida);
        return EXIT_FAILURE;
    }

    /* Executa a geracao de codigo C */
    if (codegen_run(&tree, out) != 0) {
        fprintf(stderr, "Erro durante a geracao de codigo\n");
        fclose(out);
        return EXIT_FAILURE;
    }

    fclose(out);
    printf("Codigo C gerado com sucesso em '%s'\n", saida);
    return EXIT_SUCCESS;
}
