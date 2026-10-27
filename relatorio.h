#ifndef RELATORIO_H
#define RELATORIO_H

#include "entidades.h"

#define TAM_VALOR 100 // mesmo tamanho dos maiores campos do entidades.h (nome, email, diretor)
#define TAM_NOME 30
#define MAX_COLUNAS 10

// tipos de campo
#define TIPO_TEXTO 'T'
#define TIPO_INTEIRO 'I'
#define TIPO_DECIMAL 'D'
#define TIPO_MOEDA 'M'

// um campo (coluna) do relatorio, com o nome e o formato que o usuario escolheu
typedef struct Campo {
  int coluna;          // posicao do valor dentro da Linha
  char nome[TAM_NOME]; // rotulo no topo da coluna
  char tipo;           // TIPO_TEXTO, TIPO_INTEIRO, TIPO_DECIMAL ou TIPO_MOEDA
  int casas;           // casas decimais (decimal e moeda)
  int largura;         // 0 = automatica (do tamanho do maior valor)
  int largura_final;   // largura usada de fato, calculada na hora de escrever
  char alinhamento;    // 'E' esquerda, 'D' direita
  int maiusculas;      // so texto: 1 = escreve em MAIUSCULAS
  int somar;           // so numericos: 1 = soma no fim de cada secao e no rodape
  struct Campo *prox;
} Campo;

// uma linha do corpo do relatorio
typedef struct Linha {
  char texto[MAX_COLUNAS][TAM_VALOR]; // valores de texto (ou "-" quando falta o numero)
  double numero[MAX_COLUNAS];         // valores numericos
  int tem_numero[MAX_COLUNAS];        // 1 se numero[i] foi preenchido
  struct Linha *prox;
} Linha;

// o corpo pode ser dividido em secoes (ex.: uma secao por categoria)
typedef struct Secao {
  char titulo[TAM_VALOR]; // "" quando o corpo nao e dividido
  Linha *linhas, *ultima;
  int qtd_linhas;
  struct Secao *prox;
} Secao;

// modelo do relatorio: as 3 areas e o que o usuario escolheu para cada uma
typedef struct Modelo {
  // area 1: cabecalho
  char titulo[TAM_VALOR];
  int mostrar_data;
  // area 2: corpo
  Campo *colunas;     // campos escolhidos, na ordem em que aparecem
  Campo *disponiveis; // campos que o modelo oferece mas ficaram de fora
  int qtd_campos;     // total de campos que o modelo oferece
  int coluna_secao;   // campo que divide o corpo em secoes (-1 = nao divide)
  Secao *secoes, *ultima_secao;
  // area 3: rodape
  int mostrar_total;
} Modelo;

// deixa em "colunas" so os campos de ordem[] (posicoes), na ordem dada
void escolher_colunas(Modelo *modelo, const int ordem[], int qtd);

void gerarRelatorioUsuariosAssinaturas(Usuario *usuarios, Assinatura *assinaturas, const char *caminho_saida);

void gerarRelatorioCatalogoMidias(Midia *midias, Filme *filmes, Serie *series, const char *caminho_saida);

void gerarRelatorioCatalogoSeries(Midia *midias, Serie *series, const char *caminho_saida);

void gerarRelatorioCatalogoFilmes(Midia *midias, Filme *filmes, const char *caminho_saida);

void gerarRelatorioConsumoMidiaUsuario(Usuario *usuarios, Midia *midias, MidiaConsumida *consumidas, const char *caminho_saida);

void gerarRelatorioConsumoSeriesUsuario(Usuario *usuarios, Midia *midias, Serie *series, MidiaConsumida *consumidas, const char *caminho_saida);

void gerarRelatorioConsumoFilmesUsuario(Usuario *usuarios, Midia *midias, Filme *filmes, MidiaConsumida *consumidas, const char *caminho_saida);

void gerarRelatorioFaturamentoPagamento(Assinatura *assinaturas, const char *caminho_saida);
#endif
