#ifndef NODE_H
#define NODE_H

/* Limites da estrutura de dados */
#define MAX_NODES     4096
#define MAX_NAME_LEN  256

/*
 * Representa um nó da AST lido do arquivo resultado_sintatico.txt.
 * O exportador Java escreve linhas no formato:  profundidade:nomeDoNo
 */
typedef struct {
    int  depth;              /* profundidade do nó na árvore */
    char name[MAX_NAME_LEN]; /* nome/lexema do nó            */
} ASTNode;

/* Vetor plano de todos os nós lidos do arquivo */
typedef struct {
    ASTNode nodes[MAX_NODES];
    int     count; /* quantidade de nós carregados */
} ASTree;

/*
 * Carrega o arquivo resultado_sintatico.txt em *tree.
 * Retorna 0 em caso de sucesso, -1 em caso de erro.
 */
int  ast_load(ASTree *tree, const char *path);

/* Impressão de depuração: lista todos os nós carregados */
void ast_print(const ASTree *tree);

#endif /* NODE_H */
