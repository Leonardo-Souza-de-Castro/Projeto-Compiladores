#include <stdio.h>

int main(void) {
    for (int i = 1; i < 20; i++)
    {
        printf("%d\n", i);
        i++;
    }

    printf("\nPressione ENTER para sair...");
    getchar();
    return 0;
}
