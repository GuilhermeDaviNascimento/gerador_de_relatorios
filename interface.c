#include <stdio.h>
#include <string.h>
#include "interface.h"

void ler_linha(char *destino, int tamanho) {
    if (fgets(destino, tamanho, stdin) != NULL) {
        destino[strcspn(destino, "\r\n")] = '\0';
    } else {
        destino[0] = '\0';
    }
}

int ler_inteiro(const char *mensagem) {
    char buffer[64];
    int valor;

    while (1) {
        printf("%s", mensagem);
        ler_linha(buffer, sizeof(buffer));
        if (sscanf(buffer, "%d", &valor) == 1) {
            return valor;
        }
        printf("Entrada invalida. Digite um numero.\n");
    }
}

int menu_principal(void) {
    printf("\n==============================================\n");
    printf("   GERADOR DE RELATORIOS - STREAMING\n");
    printf("==============================================\n");
    printf(" 1 - Relatorio de Usuarios e Assinaturas\n");
    printf(" 2 - Catalogo de Midias\n");
    printf(" 3 - Catalogo de Séries\n");
    printf(" 4 - Catalogo de Filmes\n");
    printf(" 5 - Consumo de Midias por Usuario\n");
    printf(" 6 - Consumo de Séries por Usuario\n");
    printf(" 7 - Consumo de Filmes por Usuario\n");
    printf(" 8 - Faturamento por Metodo de Pagamento\n");
    printf(" 9 - Sair\n");
    printf("==============================================\n");

    return ler_inteiro("Escolha uma opcao: ");
}
