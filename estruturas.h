#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

typedef struct Assinatura {
    int id;
    float valor;
    char metodo_pagamento[50];
    char vencimento[20];
    char tipo[30];
    struct Assinatura *prox;
} Assinatura;

typedef struct Usuario {
    int id;
    char nome[100];
    char email[100];
    char cpf[20];
    int id_assinatura;      /* FK  */
    struct Usuario *prox;
} Usuario;

typedef struct Midia {
    int id;
    char nome[100];
    char categoria[50];
    char diretor[100];
    int duracao;
    char data_lancamento[20];
    float avaliacao_critica;
    char tipo;
    struct Midia *prox;
} Midia;

typedef struct Filme {
    int id_midia;             /* FK  */
    float bilheteria;
    struct Filme *prox;
} Filme;

typedef struct Serie {
    int id_midia;              /* FK  */
    int numero_episodios;
    int numero_temporadas;
    struct Serie *prox;
} Serie;

typedef struct MidiaConsumida {
    int id_usuario;      /* FK  */
    int id_midia;         /* FK  */
    int favorito;
    float avaliacao_usuario;
    struct MidiaConsumida *prox;
} MidiaConsumida;

#endif
