#ifndef CODEGEN_H
#define CODEGEN_H

#include "node.h"
#include <stdio.h>

/*
 * Percorre a ASTree e emite o código C equivalente no arquivo *out.
 * Retorna 0 em sucesso, -1 em caso de erro.
 */
int codegen_run(const ASTree *tree, FILE *out);

#endif /* CODEGEN_H */
