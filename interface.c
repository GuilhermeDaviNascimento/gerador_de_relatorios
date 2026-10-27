#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"

// o que ler_inteiro devolve quando a entrada acaba (Ctrl+D no Linux, Ctrl+Z no Windows)
#define FIM_DA_ENTRADA INT_MIN

// devolve 0 quando a entrada acabou
int ler_linha(char *destino, int tamanho) {
    if (fgets(destino, tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }
    // linha maior que o buffer: joga o resto fora para nao virar a proxima resposta
    if (strchr(destino, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    destino[strcspn(destino, "\r\n")] = '\0';
    return 1;
}

int ler_inteiro(const char *mensagem) {
    char buffer[64];
    int valor;

    while (1) {
        printf("%s", mensagem);
        if (!ler_linha(buffer, sizeof(buffer))) {
            return FIM_DA_ENTRADA; // sem isso o loop repetia "Entrada invalida" para sempre
        }
        if (sscanf(buffer, "%d", &valor) == 1) {
            return valor;
        }
        printf("Entrada invalida. Digite um numero.\n");
    }
}

int menu_principal(void) {
    int opcao;

    printf("\n==============================================\n");
    printf("   GERADOR DE RELATORIOS - STREAMING\n");
    printf("==============================================\n");
    printf(" 1 - Relatorio de Usuarios e Assinaturas\n");
    printf(" 2 - Catalogo de Midias\n");
    printf(" 3 - Catalogo de Series\n");
    printf(" 4 - Catalogo de Filmes\n");
    printf(" 5 - Consumo de Midias por Usuario\n");
    printf(" 6 - Consumo de Series por Usuario\n");
    printf(" 7 - Consumo de Filmes por Usuario\n");
    printf(" 8 - Faturamento por Metodo de Pagamento\n");
    printf(" 9 - Sair\n");
    printf("==============================================\n");

    opcao = ler_inteiro("Escolha uma opcao: ");
    if (opcao == FIM_DA_ENTRADA) {
        printf("\n");
        return 9; // a entrada acabou: mesmo que escolher "Sair"
    }
    return opcao;
}

//
// CONFIGURACAO DO MODELO
// em todas as perguntas, Enter mantem o valor padrao mostrado entre colchetes
//

static int perguntar_sim_nao(const char *mensagem, int padrao) {
    char buffer[16];
    while (1) {
        printf("%s (%s): ", mensagem, padrao ? "S/n" : "s/N");
        if (!ler_linha(buffer, sizeof(buffer)) || buffer[0] == '\0')
            return padrao;
        if (buffer[0] == 's' || buffer[0] == 'S')
            return 1;
        if (buffer[0] == 'n' || buffer[0] == 'N')
            return 0;
        printf("Resposta invalida. Digite 's' ou 'n'.\n");
    }
}

static void ler_texto(const char *mensagem, char *valor, int tamanho) {
    char buffer[TAM_VALOR];
    printf("%s [%s]: ", mensagem, valor);
    if (ler_linha(buffer, sizeof(buffer)) && buffer[0] != '\0')
        snprintf(valor, tamanho, "%s", buffer);
}

static int ler_inteiro_padrao(const char *mensagem, int padrao, int minimo, int maximo) {
    char buffer[64];
    int valor;
    while (1) {
        printf("%s [%d]: ", mensagem, padrao);
        if (!ler_linha(buffer, sizeof(buffer)) || buffer[0] == '\0')
            return padrao;
        if (sscanf(buffer, "%d", &valor) == 1 && valor >= minimo && valor <= maximo)
            return valor;
        printf("Digite um numero de %d a %d.\n", minimo, maximo);
    }
}

static char ler_letra(const char *mensagem, const char *validas, char padrao) {
    char buffer[16];
    while (1) {
        printf("%s [%c]: ", mensagem, padrao);
        if (!ler_linha(buffer, sizeof(buffer)) || buffer[0] == '\0')
            return padrao;
        char letra = (char)toupper((unsigned char)buffer[0]);
        if (strchr(validas, letra) != NULL)
            return letra;
        printf("Opcao invalida. Use uma destas letras: %s\n", validas);
    }
}

// le varios numeros na mesma linha ("3 1 2" ou "3,1,2")
// devolve quantos numeros validos leu, ou -1 se o usuario so apertou Enter
static int ler_lista_numeros(const char *mensagem, int numeros[], int max, int maior_valido) {
    char buffer[128];
    char *p, *fim;
    int qtd = 0;

    printf("%s: ", mensagem);
    if (!ler_linha(buffer, sizeof(buffer)) || buffer[0] == '\0')
        return -1;

    p = buffer;
    while (*p != '\0') {
        long n = strtol(p, &fim, 10);
        if (fim == p) { // nao e numero (espaco, virgula...): pula
            p++;
            continue;
        }
        p = fim;
        if (n < 1 || n > maior_valido) {
            if (n != 0)
                printf("Numero %ld ignorado (use de 1 a %d).\n", n, maior_valido);
            continue;
        }
        if (qtd < max)
            numeros[qtd++] = (int)n;
    }
    return qtd;
}

static const char *nome_do_tipo(char tipo) {
    switch (tipo) {
    case TIPO_INTEIRO:
        return "inteiro";
    case TIPO_DECIMAL:
        return "decimal";
    case TIPO_MOEDA:
        return "moeda";
    default:
        return "texto";
    }
}

static void listar_campos(const Campo *lista) {
    for (const Campo *c = lista; c != NULL; c = c->prox)
        printf("  %2d - %-20s %s\n", c->coluna + 1, c->nome, nome_do_tipo(c->tipo));
}

// nome, tipo e formato de um campo
static void configurar_campo(Campo *c) {
    printf("\nCampo %d - %s (%s)\n", c->coluna + 1, c->nome, nome_do_tipo(c->tipo));
    ler_texto("  Nome da coluna", c->nome, TAM_NOME);

    if (c->tipo == TIPO_TEXTO) {
        c->maiusculas = ler_letra("  Formato: N = normal, M = MAIUSCULAS", "NM",
                                  c->maiusculas ? 'M' : 'N') == 'M';
    } else {
        c->tipo = ler_letra("  Tipo: I = inteiro, D = decimal, M = moeda (R$)", "IDM", c->tipo);
        if (c->tipo == TIPO_INTEIRO) {
            c->casas = 0;
        } else {
            int casas = c->casas > 0 ? c->casas : 2;
            c->casas = ler_inteiro_padrao("  Casas decimais", casas, 0, 6);
        }
    }

    c->largura = ler_inteiro_padrao("  Largura (0 = automatica)", c->largura, 0, TAM_VALOR - 1);
    c->alinhamento = ler_letra("  Alinhamento: E = esquerda, D = direita", "ED", c->alinhamento);
}

// quais campos numericos entram na soma (fim de cada secao e rodape)
static void configurar_somas(Modelo *modelo) {
    int numeros[MAX_COLUNAS];
    int qtd, i, tem_numerico = 0;
    Campo *c;

    for (c = modelo->colunas; c != NULL; c = c->prox) {
        if (c->tipo == TIPO_TEXTO)
            continue;
        if (!tem_numerico)
            printf("Campos numericos do relatorio:\n");
        printf("  %2d - %-20s %s\n", c->coluna + 1, c->nome, c->somar ? "(soma)" : "");
        tem_numerico = 1;
    }
    if (!tem_numerico)
        return;

    qtd = ler_lista_numeros("Somar quais campos? (Enter = manter, 0 = nenhum)", numeros,
                            MAX_COLUNAS, modelo->qtd_campos);
    if (qtd < 0)
        return;

    for (c = modelo->colunas; c != NULL; c = c->prox)
        c->somar = 0;
    for (i = 0; i < qtd; i++) {
        for (c = modelo->colunas; c != NULL; c = c->prox)
            if (c->coluna == numeros[i] - 1)
                break;
        if (c != NULL && c->tipo != TIPO_TEXTO)
            c->somar = 1;
        else
            printf("Campo %d ignorado: nao e numerico ou nao esta no relatorio.\n", numeros[i]);
    }
}

void configurar_modelo(Modelo *modelo) {
    int numeros[MAX_COLUNAS];
    int qtd, i;
    Campo *c;

    printf("\n--- Modelo: %s ---\n", modelo->titulo);
    if (perguntar_sim_nao("Usar o modelo padrao?", 1))
        return;

    printf("\n[Area 1 - CABECALHO]\n");
    ler_texto("Titulo do relatorio", modelo->titulo, TAM_VALOR);
    modelo->mostrar_data = perguntar_sim_nao("Mostrar data de geracao?", modelo->mostrar_data);

    printf("\n[Area 2 - CORPO]\n");
    printf("Campos disponiveis:\n");
    listar_campos(modelo->colunas);

    qtd = ler_lista_numeros("Campos a incluir, na ordem desejada (Enter = todos)", numeros,
                            MAX_COLUNAS, modelo->qtd_campos);
    if (qtd == 0)
        printf("Nenhum campo valido; mantendo todos.\n");
    if (qtd > 0) {
        for (i = 0; i < qtd; i++)
            numeros[i]--; // numero mostrado na tela -> posicao na linha
        escolher_colunas(modelo, numeros, qtd);
    }

    if (perguntar_sim_nao("Personalizar nome, tipo e formato dos campos?", 0)) {
        for (c = modelo->colunas; c != NULL; c = c->prox)
            configurar_campo(c);
        printf("\n");
    }

    modelo->coluna_secao =
        ler_inteiro_padrao("Dividir o corpo em secoes pelo campo numero (0 = nao dividir)", 0, 0,
                           modelo->qtd_campos) - 1;

    printf("\n[Area 3 - RODAPE]\n");
    modelo->mostrar_total = perguntar_sim_nao("Mostrar total de registros?", modelo->mostrar_total);
    configurar_somas(modelo);
}
