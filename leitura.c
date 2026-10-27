#include "leitura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 1024

static void copia_campo(char *destino, const char *token, size_t tamanho) {
  if (token == NULL) {
    destino[0] = '\0';
    return;
  }
  strncpy(destino, token, tamanho - 1);
  destino[tamanho - 1] = '\0';
}

// faz o papel do strtok, com duas diferencas:
// - nao pula campo vazio ("1;;x" sao 3 campos, o strtok via so 2 e deslocava as colunas)
// - aceita ';' ou <tab> como separador
static char *proximo_campo(char **cursor) {
  char *inicio = *cursor;
  if (inicio == NULL)
    return NULL;

  char *separador = inicio + strcspn(inicio, ";\t");
  if (*separador != '\0') {
    *separador = '\0';
    *cursor = separador + 1;
  } else {
    *cursor = NULL; // era o ultimo campo da linha
  }
  return inicio;
}

Assinatura *ler_assinaturas(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    printf("Erro ao abrir arquivo '%s'\n", caminho);
    return NULL;
  }

  Assinatura *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];

  while (fgets(linha, sizeof(linha), f)) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strlen(linha) == 0)
      continue;

    Assinatura *a = malloc(sizeof(Assinatura));
    if (!a) {
      printf("Erro de alocacao de memoria\n");
      break;
    }

    char *cursor = linha;
    char *token = proximo_campo(&cursor);
    a->id = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    a->valor = token ? (float)atof(token) : 0.0f;

    token = proximo_campo(&cursor);
    copia_campo(a->metodo_pagamento, token, sizeof(a->metodo_pagamento));

    token = proximo_campo(&cursor);
    copia_campo(a->vencimento, token, sizeof(a->vencimento));

    token = proximo_campo(&cursor);
    copia_campo(a->tipo, token, sizeof(a->tipo));

    a->prox = NULL;
    if (!inicio)
      inicio = fim = a;
    else {
      fim->prox = a;
      fim = a;
    }
  }
  fclose(f);
  return inicio;
}

Usuario *ler_usuarios(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    printf("Erro ao abrir arquivo '%s'\n", caminho);
    return NULL;
  }

  Usuario *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];

  while (fgets(linha, sizeof(linha), f)) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strlen(linha) == 0)
      continue;

    Usuario *u = malloc(sizeof(Usuario));
    if (!u) {
      printf("Erro de alocacao de memoria\n");
      break;
    }

    char *cursor = linha;
    char *token = proximo_campo(&cursor);
    u->id = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    copia_campo(u->nome, token, sizeof(u->nome));

    token = proximo_campo(&cursor);
    copia_campo(u->email, token, sizeof(u->email));

    token = proximo_campo(&cursor);
    copia_campo(u->cpf, token, sizeof(u->cpf));

    token = proximo_campo(&cursor);
    u->id_assinatura = token ? atoi(token) : 0;

    u->prox = NULL;
    if (!inicio)
      inicio = fim = u;
    else {
      fim->prox = u;
      fim = u;
    }
  }
  fclose(f);
  return inicio;
}

// filmes e series agora ficam nos proprios arquivos (filmes.txt e series.txt),
// como as tabelas Filme e Serie do entidades.h
Midia *ler_midias(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    printf("Erro ao abrir arquivo '%s'\n", caminho);
    return NULL;
  }

  Midia *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];

  while (fgets(linha, sizeof(linha), f)) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strlen(linha) == 0)
      continue;

    Midia *m = malloc(sizeof(Midia));
    if (!m) {
      printf("Erro de alocacao de memoria\n");
      break;
    }

    char *cursor = linha;
    char *token = proximo_campo(&cursor);
    m->id = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    copia_campo(m->nome, token, sizeof(m->nome));

    token = proximo_campo(&cursor);
    copia_campo(m->categoria, token, sizeof(m->categoria));

    token = proximo_campo(&cursor);
    copia_campo(m->diretor, token, sizeof(m->diretor));

    token = proximo_campo(&cursor);
    m->duracao = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    copia_campo(m->data_lancamento, token, sizeof(m->data_lancamento));

    token = proximo_campo(&cursor);
    m->avaliacao_critica = token ? (float)atof(token) : 0.0f;

    token = proximo_campo(&cursor);
    m->tipo = (token && token[0] != '\0') ? token[0] : 'F';

    m->prox = NULL;
    if (!inicio)
      inicio = fim = m;
    else {
      fim->prox = m;
      fim = m;
    }
  }
  fclose(f);
  return inicio;
}

Filme *ler_filmes(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    printf("Erro ao abrir arquivo '%s'\n", caminho);
    return NULL;
  }

  Filme *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];

  while (fgets(linha, sizeof(linha), f)) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strlen(linha) == 0)
      continue;

    Filme *filme = malloc(sizeof(Filme));
    if (!filme) {
      printf("Erro de alocacao de memoria\n");
      break;
    }

    char *cursor = linha;
    char *token = proximo_campo(&cursor);
    filme->id_midia = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    filme->bilheteria = token ? (float)atof(token) : 0.0f;

    filme->prox = NULL;
    if (!inicio)
      inicio = fim = filme;
    else {
      fim->prox = filme;
      fim = filme;
    }
  }
  fclose(f);
  return inicio;
}

Serie *ler_series(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    printf("Erro ao abrir arquivo '%s'\n", caminho);
    return NULL;
  }

  Serie *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];

  while (fgets(linha, sizeof(linha), f)) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strlen(linha) == 0)
      continue;

    Serie *serie = malloc(sizeof(Serie));
    if (!serie) {
      printf("Erro de alocacao de memoria\n");
      break;
    }

    char *cursor = linha;
    char *token = proximo_campo(&cursor);
    serie->id_midia = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    serie->numero_episodios = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    serie->numero_temporadas = token ? atoi(token) : 0;

    serie->prox = NULL;
    if (!inicio)
      inicio = fim = serie;
    else {
      fim->prox = serie;
      fim = serie;
    }
  }
  fclose(f);
  return inicio;
}

MidiaConsumida *ler_midias_consumidas(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    printf("Erro ao abrir arquivo '%s'\n", caminho);
    return NULL;
  }

  MidiaConsumida *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];

  while (fgets(linha, sizeof(linha), f)) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strlen(linha) == 0)
      continue;

    MidiaConsumida *mc = malloc(sizeof(MidiaConsumida));
    if (!mc) {
      printf("Erro de alocacao de memoria\n");
      break;
    }

    char *cursor = linha;
    char *token = proximo_campo(&cursor);
    mc->id_usuario = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    mc->id_midia = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    mc->favorito = token ? atoi(token) : 0;

    token = proximo_campo(&cursor);
    mc->avaliacao_usuario = token ? (float)atof(token) : 0.0f;

    mc->prox = NULL;
    if (!inicio)
      inicio = fim = mc;
    else {
      fim->prox = mc;
      fim = mc;
    }
  }
  fclose(f);
  return inicio;
}

// liberacao das listas: guarda o proximo antes de dar free no atual

void liberar_assinaturas(Assinatura *lista) {
  while (lista != NULL) {
    Assinatura *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_usuarios(Usuario *lista) {
  while (lista != NULL) {
    Usuario *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_midias(Midia *lista) {
  while (lista != NULL) {
    Midia *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_filmes(Filme *lista) {
  while (lista != NULL) {
    Filme *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_series(Serie *lista) {
  while (lista != NULL) {
    Serie *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_midias_consumidas(MidiaConsumida *lista) {
  while (lista != NULL) {
    MidiaConsumida *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}
