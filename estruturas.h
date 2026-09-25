#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

// ===== ASSINATURA =====
typedef struct Assinatura {
    int id;
    float valor;
    char metodo_pagamento[50];
    char vencimento[20];
    char tipo[30];
    struct Assinatura *prox;
} Assinatura;

// ===== USUARIO =====
typedef struct Usuario {
    int id;
    char nome[100];
    char email[100];
    char cpf[20];
    int id_assinatura;      // FK -> Assinatura (relacionamento "Possui")
    struct Usuario *prox;
} Usuario;

// ===== MIDIA =====
typedef struct Midia {
    int id;
    char nome[100];
    char categoria[50];
    char diretor[100];
    int duracao;
    char data_lancamento[20];
    float avaliacao_critica;
    char tipo;               // 'F' = Filme, 'S' = Serie
    struct Midia *prox;
} Midia;

// ===== FILME (especialização de Midia) =====
typedef struct Filme {
    int id_midia;             // FK -> Midia
    float bilheteria;
    struct Filme *prox;
} Filme;

// ===== SERIE (especialização de Midia) =====
typedef struct Serie {
    int id_midia;              // FK -> Midia
    int numero_episodios;
    int numero_temporadas;
    struct Serie *prox;
} Serie;

// ===== MIDIAS CONSUMIDAS (relacionamento "Consome", com atributos) =====
typedef struct MidiaConsumida {
    int id_usuario;      // FK -> Usuario
    int id_midia;         // FK -> Midia
    int favorito;          // 0 = não, 1 = sim
    float avaliacao_usuario;
    struct MidiaConsumida *prox;
} MidiaConsumida;

#endif