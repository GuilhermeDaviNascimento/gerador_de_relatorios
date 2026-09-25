#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitura.h"

#define TAM_LINHA 1024

static void copia_campo(char *destino, const char *token, size_t tamanho) {
    if (token == NULL) { destino[0] = '\0'; return; }
    strncpy(destino, token, tamanho - 1);
    destino[tamanho - 1] = '\0';
}


Assinatura *ler_assinaturas(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) { printf("Erro ao abrir arquivo '%s'\n", caminho); return NULL; }

    Assinatura *inicio = NULL, *fim = NULL;
    char linha[TAM_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Assinatura *a = malloc(sizeof(Assinatura));
        if (!a) { printf("Erro de alocacao de memoria\n"); break; }

        char *token = strtok(linha, ";");
        a->id = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        a->valor = token ? (float)atof(token) : 0.0f;

        token = strtok(NULL, ";");
        copia_campo(a->metodo_pagamento, token, sizeof(a->metodo_pagamento));

        token = strtok(NULL, ";");
        copia_campo(a->vencimento, token, sizeof(a->vencimento));

        token = strtok(NULL, ";");
        copia_campo(a->tipo, token, sizeof(a->tipo));

        a->prox = NULL;
        if (!inicio) inicio = fim = a;
        else { fim->prox = a; fim = a; }
    }
    fclose(f);
    return inicio;
}


Usuario *ler_usuarios(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) { printf("Erro ao abrir arquivo '%s'\n", caminho); return NULL; }

    Usuario *inicio = NULL, *fim = NULL;
    char linha[TAM_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Usuario *u = malloc(sizeof(Usuario));
        if (!u) { printf("Erro de alocacao de memoria\n"); break; }

        char *token = strtok(linha, ";");
        u->id = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        copia_campo(u->nome, token, sizeof(u->nome));

        token = strtok(NULL, ";");
        copia_campo(u->email, token, sizeof(u->email));

        token = strtok(NULL, ";");
        copia_campo(u->cpf, token, sizeof(u->cpf));

        token = strtok(NULL, ";");
        u->id_assinatura = token ? atoi(token) : 0;

        u->prox = NULL;
        if (!inicio) inicio = fim = u;
        else { fim->prox = u; fim = u; }
    }
    fclose(f);
    return inicio;
}

Midia *ler_midias(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) { printf("Erro ao abrir arquivo '%s'\n", caminho); return NULL; }

    Midia *inicio = NULL, *fim = NULL;
    char linha[TAM_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Midia *m = malloc(sizeof(Midia));
        if (!m) { printf("Erro de alocacao de memoria\n"); break; }

        char *token = strtok(linha, ";");
        m->id = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        copia_campo(m->nome, token, sizeof(m->nome));

        token = strtok(NULL, ";");
        copia_campo(m->categoria, token, sizeof(m->categoria));

        token = strtok(NULL, ";");
        copia_campo(m->diretor, token, sizeof(m->diretor));

        token = strtok(NULL, ";");
        m->duracao = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        copia_campo(m->data_lancamento, token, sizeof(m->data_lancamento));

        token = strtok(NULL, ";");
        m->avaliacao_critica = token ? (float)atof(token) : 0.0f;

        token = strtok(NULL, ";");
        m->tipo = (token && token[0] != '\0') ? token[0] : 'F';

        m->prox = NULL;
        if (!inicio) inicio = fim = m;
        else { fim->prox = m; fim = m; }
    }
    fclose(f);
    return inicio;
}

Filme *ler_filmes(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) { printf("Erro ao abrir arquivo '%s'\n", caminho); return NULL; }

    Filme *inicio = NULL, *fim = NULL;
    char linha[TAM_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Filme *fl = malloc(sizeof(Filme));
        if (!fl) { printf("Erro de alocacao de memoria\n"); break; }

        char *token = strtok(linha, ";");
        fl->id_midia = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        fl->bilheteria = token ? (float)atof(token) : 0.0f;

        fl->prox = NULL;
        if (!inicio) inicio = fim = fl;
        else { fim->prox = fl; fim = fl; }
    }
    fclose(f);
    return inicio;
}

Serie *ler_series(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) { printf("Erro ao abrir arquivo '%s'\n", caminho); return NULL; }

    Serie *inicio = NULL, *fim = NULL;
    char linha[TAM_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Serie *s = malloc(sizeof(Serie));
        if (!s) { printf("Erro de alocacao de memoria\n"); break; }

        char *token = strtok(linha, ";");
        s->id_midia = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        s->numero_episodios = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        s->numero_temporadas = token ? atoi(token) : 0;

        s->prox = NULL;
        if (!inicio) inicio = fim = s;
        else { fim->prox = s; fim = s; }
    }
    fclose(f);
    return inicio;
}

MidiaConsumida *ler_midias_consumidas(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) { printf("Erro ao abrir arquivo '%s'\n", caminho); return NULL; }

    MidiaConsumida *inicio = NULL, *fim = NULL;
    char linha[TAM_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        MidiaConsumida *mc = malloc(sizeof(MidiaConsumida));
        if (!mc) { printf("Erro de alocacao de memoria\n"); break; }

        char *token = strtok(linha, ";");
        mc->id_usuario = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        mc->id_midia = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        mc->favorito = token ? atoi(token) : 0;

        token = strtok(NULL, ";");
        mc->avaliacao_usuario = token ? (float)atof(token) : 0.0f;

        mc->prox = NULL;
        if (!inicio) inicio = fim = mc;
        else { fim->prox = mc; fim = mc; }
    }
    fclose(f);
    return inicio;
}
