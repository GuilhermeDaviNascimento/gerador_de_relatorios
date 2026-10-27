#include "relatorio.h"
#include "entidades.h"
#include "interface.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEM_REGISTROS "Nenhum registro encontrado"

// static significa que a função é interna do arquivo em q foi declarada
// essa função só pode ser chamada aqui, relatorio.c
// proposito: evitar confusão

static Midia *buscar_midia(Midia *lista, int id) {
  Midia *m;
  // pressupõe q o arquivo txt de midia já foi lido
  // imitar esse para os proxs
  for (m = lista; m != NULL; m = m->prox)
    if (m->id == id)
      return m;
  return NULL;
}

static Filme *buscar_filme(Filme *lista, int id_midia) {
  Filme *f;
  for (f = lista; f != NULL; f = f->prox)
    if (f->id_midia == id_midia)
      return f;
  return NULL;
}

static Serie *buscar_serie(Serie *lista, int id_midia) {
  Serie *s;
  for (s = lista; s != NULL; s = s->prox)
    if (s->id_midia == id_midia)
      return s;
  return NULL;
}

static Assinatura *buscar_assinatura(Assinatura *lista, int id) {
  Assinatura *a;
  for (a = lista; a != NULL; a = a->prox)
    if (a->id == id)
      return a;
  return NULL;
}

static Usuario *buscar_usuario(Usuario *lista, int id) {
  Usuario *u;
  for (u = lista; u != NULL; u = u->prox)
    if (u->id == id)
      return u;
  return NULL;
}

//
//
//
// MODELO DO RELATORIO
// cabecalho, corpo (lista de campos + lista de secoes com a lista de linhas) e rodape
//
//
//

static int maior(int a, int b) { return a > b ? a : b; }

// conta caracteres, nao bytes: em UTF-8 letra acentuada ocupa 2 bytes
// e o printf("%-*s") desalinharia a tabela
static int largura_texto(const char *s) {
  int n = 0;
  for (; *s != '\0'; s++)
    if (((unsigned char)*s & 0xC0) != 0x80)
      n++;
  return n;
}

static void iniciar_modelo(Modelo *modelo, const char *titulo) {
  snprintf(modelo->titulo, TAM_VALOR, "%s", titulo);
  modelo->mostrar_data = 1;
  modelo->colunas = NULL;
  modelo->disponiveis = NULL;
  modelo->qtd_campos = 0;
  modelo->coluna_secao = -1;
  modelo->secoes = NULL;
  modelo->ultima_secao = NULL;
  modelo->mostrar_total = 1;
}

// o campo vai para o fim da lista; a ordem das chamadas define a posicao
// do valor na Linha (0, 1, 2...), igual aos antigos valores[0], valores[1]...
static void adicionar_campo(Modelo *modelo, const char *nome, char tipo, int somar) {
  if (modelo->qtd_campos >= MAX_COLUNAS) {
    printf("Campo '%s' ignorado: limite de %d campos\n", nome, MAX_COLUNAS);
    return;
  }

  Campo *c = malloc(sizeof(Campo));
  if (!c) {
    printf("Erro de alocacao de memoria\n");
    modelo->qtd_campos++; // mantem a posicao dos proximos campos
    return;
  }

  c->coluna = modelo->qtd_campos++;
  snprintf(c->nome, TAM_NOME, "%s", nome);
  c->tipo = tipo;
  c->casas = tipo == TIPO_MOEDA ? 2 : (tipo == TIPO_DECIMAL ? 1 : 0);
  c->largura = 0;
  c->largura_final = 0;
  c->alinhamento = tipo == TIPO_TEXTO ? 'E' : 'D';
  c->maiusculas = 0;
  c->somar = somar;
  c->prox = NULL;

  if (modelo->colunas == NULL)
    modelo->colunas = c;
  else {
    Campo *ultimo = modelo->colunas;
    while (ultimo->prox != NULL)
      ultimo = ultimo->prox;
    ultimo->prox = c;
  }
}

static Campo *buscar_campo(Modelo *modelo, int coluna) {
  Campo *c;
  for (c = modelo->colunas; c != NULL; c = c->prox)
    if (c->coluna == coluna)
      return c;
  for (c = modelo->disponiveis; c != NULL; c = c->prox)
    if (c->coluna == coluna)
      return c;
  return NULL;
}

void escolher_colunas(Modelo *modelo, const int ordem[], int qtd) {
  int i;
  Campo *ultima = NULL;

  // 1) junta todos os campos na lista de disponiveis
  if (modelo->disponiveis == NULL)
    modelo->disponiveis = modelo->colunas;
  else {
    Campo *fim = modelo->disponiveis;
    while (fim->prox != NULL)
      fim = fim->prox;
    fim->prox = modelo->colunas;
  }
  modelo->colunas = NULL;

  // 2) tira de disponiveis e poe no fim de colunas, na ordem pedida
  for (i = 0; i < qtd; i++) {
    Campo *anterior = NULL, *c = modelo->disponiveis;
    while (c != NULL && c->coluna != ordem[i]) {
      anterior = c;
      c = c->prox;
    }
    if (c == NULL)
      continue; // numero repetido ou inexistente

    if (anterior == NULL)
      modelo->disponiveis = c->prox;
    else
      anterior->prox = c->prox;

    c->prox = NULL;
    if (ultima == NULL)
      modelo->colunas = c;
    else
      ultima->prox = c;
    ultima = c;
  }

  // nenhum numero valido: volta todos os campos
  if (modelo->colunas == NULL) {
    modelo->colunas = modelo->disponiveis;
    modelo->disponiveis = NULL;
  }
}

static Linha *nova_linha(void) {
  Linha *l = calloc(1, sizeof(Linha)); // calloc zera tudo: textos vazios e nenhum numero
  if (!l)
    printf("Erro de alocacao de memoria\n");
  return l;
}

static void definir_numero(Linha *l, int coluna, double valor) {
  l->numero[coluna] = valor;
  l->tem_numero[coluna] = 1;
}

// R$ 1.234.567,89
static void formatar_moeda(double valor, int casas, char *destino) {
  char numero[TAM_VALOR];
  char inteiro[2 * TAM_VALOR];
  int i, j = 0, digitos;
  char *ponto;

  snprintf(numero, sizeof(numero), "%.*f", casas, valor < 0 ? -valor : valor);
  ponto = strchr(numero, '.');
  digitos = ponto ? (int)(ponto - numero) : (int)strlen(numero);

  for (i = 0; i < digitos; i++) {
    if (i > 0 && (digitos - i) % 3 == 0)
      inteiro[j++] = '.';
    inteiro[j++] = numero[i];
  }
  inteiro[j] = '\0';

  snprintf(destino, TAM_VALOR, "%sR$ %s%s%s", valor < 0 ? "-" : "", inteiro,
           ponto ? "," : "", ponto ? ponto + 1 : "");
}

static void formatar_numero(const Campo *c, double valor, char *destino) {
  char *ponto;

  if (c->tipo == TIPO_MOEDA) {
    formatar_moeda(valor, c->casas, destino);
    return;
  }
  if (c->tipo == TIPO_INTEIRO)
    snprintf(destino, TAM_VALOR, "%.0f", valor);
  else
    snprintf(destino, TAM_VALOR, "%.*f", c->casas, valor);

  ponto = strchr(destino, '.'); // padrao brasileiro: virgula nas casas decimais
  if (ponto)
    *ponto = ',';
}

// transforma o valor guardado na linha em texto, do jeito que o campo foi configurado
static void formatar_valor(const Campo *c, const Linha *l, char *destino) {
  int i;

  if (c->tipo != TIPO_TEXTO && l->tem_numero[c->coluna]) {
    formatar_numero(c, l->numero[c->coluna], destino);
    return;
  }

  snprintf(destino, TAM_VALOR, "%s", l->texto[c->coluna]);
  if (c->maiusculas)
    for (i = 0; destino[i] != '\0'; i++)
      destino[i] = (char)toupper((unsigned char)destino[i]);
}

// poe a linha no fim da secao dela; se o corpo nao e dividido, tudo vai numa secao so
static void adicionar_linha(Modelo *modelo, Linha *l) {
  char titulo[TAM_VALOR] = "";
  Secao *s;

  if (modelo->coluna_secao >= 0) {
    Campo *c = buscar_campo(modelo, modelo->coluna_secao);
    char valor[TAM_VALOR];
    if (c != NULL) {
      formatar_valor(c, l, valor);
      snprintf(titulo, sizeof(titulo), "%s: %.48s", c->nome, valor);
    }
  }

  for (s = modelo->secoes; s != NULL; s = s->prox)
    if (strcmp(s->titulo, titulo) == 0)
      break;

  if (s == NULL) {
    s = malloc(sizeof(Secao));
    if (!s) {
      printf("Erro de alocacao de memoria\n");
      free(l);
      return;
    }
    snprintf(s->titulo, TAM_VALOR, "%s", titulo);
    s->linhas = s->ultima = NULL;
    s->qtd_linhas = 0;
    s->prox = NULL;

    if (modelo->secoes == NULL)
      modelo->secoes = s;
    else
      modelo->ultima_secao->prox = s;
    modelo->ultima_secao = s;
  }

  l->prox = NULL;
  if (s->linhas == NULL)
    s->linhas = l;
  else
    s->ultima->prox = l;
  s->ultima = l;
  s->qtd_linhas++;
}

static int contar_registros(const Modelo *modelo) {
  int total = 0;
  for (const Secao *s = modelo->secoes; s != NULL; s = s->prox)
    total += s->qtd_linhas;
  return total;
}

static double somar_secao(const Secao *s, int coluna) {
  double soma = 0;
  for (const Linha *l = s->linhas; l != NULL; l = l->prox)
    if (l->tem_numero[coluna])
      soma += l->numero[coluna];
  return soma;
}

static double somar_modelo(const Modelo *modelo, int coluna) {
  double soma = 0;
  for (const Secao *s = modelo->secoes; s != NULL; s = s->prox)
    soma += somar_secao(s, coluna);
  return soma;
}

static int deve_somar(const Campo *c) { return c->somar && c->tipo != TIPO_TEXTO; }

static void texto_soma(const Campo *c, double soma, char *destino) {
  char valor[TAM_VALOR];
  formatar_numero(c, soma, valor);
  snprintf(destino, TAM_VALOR, "Soma de %s: %.40s", c->nome, valor);
}

// largura do texto dentro de uma linha que ocupa a tabela toda: "| texto |"
static int largura_interna(const Modelo *modelo) {
  int total = 1; // linha de tabela "| a | b |" = 1 + soma(largura + 3)
  for (const Campo *c = modelo->colunas; c != NULL; c = c->prox)
    total += c->largura_final + 3;
  return total - 4;
}

// cada coluna fica do tamanho do maior valor (ou da largura que o usuario pediu);
// se titulo, secoes ou totais nao couberem na tabela, a ultima coluna cresce
static void calcular_larguras(Modelo *modelo, const char *data) {
  char texto[TAM_VALOR];
  Campo *c, *ultima = NULL;
  Secao *s;
  Linha *l;
  int precisa, interna;

  for (c = modelo->colunas; c != NULL; c = c->prox) {
    ultima = c;
    if (c->largura > 0) {
      c->largura_final = c->largura;
      continue;
    }
    c->largura_final = largura_texto(c->nome);
    for (s = modelo->secoes; s != NULL; s = s->prox)
      for (l = s->linhas; l != NULL; l = l->prox) {
        formatar_valor(c, l, texto);
        c->largura_final = maior(c->largura_final, largura_texto(texto));
      }
  }

  precisa = maior(largura_texto(modelo->titulo), largura_texto(data));
  if (modelo->secoes == NULL)
    precisa = maior(precisa, largura_texto(SEM_REGISTROS));

  if (modelo->coluna_secao >= 0)
    for (s = modelo->secoes; s != NULL; s = s->prox) {
      precisa = maior(precisa, largura_texto(s->titulo));
      snprintf(texto, sizeof(texto), "Registros na secao: %d", s->qtd_linhas);
      precisa = maior(precisa, largura_texto(texto));
      for (c = modelo->colunas; c != NULL; c = c->prox)
        if (deve_somar(c)) {
          texto_soma(c, somar_secao(s, c->coluna), texto);
          precisa = maior(precisa, largura_texto(texto));
        }
    }

  if (modelo->mostrar_total) {
    snprintf(texto, sizeof(texto), "Total de registros: %d", contar_registros(modelo));
    precisa = maior(precisa, largura_texto(texto));
  }
  for (c = modelo->colunas; c != NULL; c = c->prox)
    if (deve_somar(c)) {
      texto_soma(c, somar_modelo(modelo, c->coluna), texto);
      precisa = maior(precisa, largura_texto(texto));
    }

  interna = largura_interna(modelo);
  if (ultima != NULL && precisa > interna)
    ultima->largura_final += precisa - interna;
}

// escreve o texto ocupando exatamente 'largura' caracteres:
// corta se for maior e completa com espacos se for menor (isso mantem a tabela alinhada)
static void escrever_celula(FILE *saida, const char *texto, int largura, char alinhamento) {
  const char *fim = texto;
  int usados = 0, sobra, i;

  while (*fim != '\0') {
    if (((unsigned char)*fim & 0xC0) != 0x80) { // comeco de um caractere
      if (usados == largura)
        break;
      usados++;
    }
    fim++;
  }
  sobra = largura - usados;

  if (alinhamento == 'D')
    for (i = 0; i < sobra; i++)
      fputc(' ', saida);
  fwrite(texto, 1, (size_t)(fim - texto), saida);
  if (alinhamento != 'D')
    for (i = 0; i < sobra; i++)
      fputc(' ', saida);
}

static void imprimir_borda(FILE *saida, const Campo *colunas) {
  int j;
  for (const Campo *c = colunas; c != NULL; c = c->prox) {
    fputc('+', saida);
    for (j = 0; j < c->largura_final + 2; j++)
      fputc('-', saida);
  }
  fputc('+', saida);
  fputc('\n', saida);
}

// borda sem divisao de colunas, para as linhas que ocupam a tabela toda
static void imprimir_borda_lisa(FILE *saida, int interna) {
  int j;
  fputc('+', saida);
  for (j = 0; j < interna + 2; j++)
    fputc('-', saida);
  fputc('+', saida);
  fputc('\n', saida);
}

static void imprimir_linha_cheia(FILE *saida, const char *texto, int interna) {
  fputs("| ", saida);
  escrever_celula(saida, texto, interna, 'E');
  fputs(" |\n", saida);
}

static void imprimir_titulos(FILE *saida, const Campo *colunas) {
  for (const Campo *c = colunas; c != NULL; c = c->prox) {
    fputs("| ", saida);
    escrever_celula(saida, c->nome, c->largura_final, c->alinhamento);
    fputc(' ', saida);
  }
  fputs("|\n", saida);
}

static void imprimir_linha_tabela(FILE *saida, const Campo *colunas, const Linha *l) {
  char valor[TAM_VALOR];
  for (const Campo *c = colunas; c != NULL; c = c->prox) {
    formatar_valor(c, l, valor);
    fputs("| ", saida);
    escrever_celula(saida, valor, c->largura_final, c->alinhamento);
    fputc(' ', saida);
  }
  fputs("|\n", saida);
}

static void escrever_relatorio(FILE *saida, Modelo *modelo) {
  char texto[TAM_VALOR];
  char data[40] = "";
  int interna, tem_rodape;
  Campo *c;
  Secao *s;
  Linha *l;

  if (modelo->colunas == NULL) {
    fprintf(saida, "Modelo sem campos\n");
    return;
  }

  if (modelo->mostrar_data) {
    time_t agora = time(NULL);
    strftime(data, sizeof(data), "Gerado em %d/%m/%Y %H:%M", localtime(&agora));
  }

  calcular_larguras(modelo, data);
  interna = largura_interna(modelo);

  // area 1: cabecalho
  imprimir_borda_lisa(saida, interna);
  imprimir_linha_cheia(saida, modelo->titulo, interna);
  if (modelo->mostrar_data)
    imprimir_linha_cheia(saida, data, interna);
  imprimir_borda_lisa(saida, interna);
  fputc('\n', saida);

  // area 2: corpo, uma tabela por secao
  if (modelo->secoes == NULL) {
    imprimir_borda(saida, modelo->colunas);
    imprimir_titulos(saida, modelo->colunas);
    imprimir_borda(saida, modelo->colunas);
    imprimir_linha_cheia(saida, SEM_REGISTROS, interna);
    imprimir_borda_lisa(saida, interna);
    fputc('\n', saida);
  }

  for (s = modelo->secoes; s != NULL; s = s->prox) {
    if (modelo->coluna_secao >= 0) {
      imprimir_borda_lisa(saida, interna);
      imprimir_linha_cheia(saida, s->titulo, interna);
    }

    imprimir_borda(saida, modelo->colunas);
    imprimir_titulos(saida, modelo->colunas);
    imprimir_borda(saida, modelo->colunas);
    for (l = s->linhas; l != NULL; l = l->prox)
      imprimir_linha_tabela(saida, modelo->colunas, l);
    imprimir_borda(saida, modelo->colunas);

    if (modelo->coluna_secao >= 0) { // subtotal da secao
      snprintf(texto, sizeof(texto), "Registros na secao: %d", s->qtd_linhas);
      imprimir_linha_cheia(saida, texto, interna);
      for (c = modelo->colunas; c != NULL; c = c->prox)
        if (deve_somar(c)) {
          texto_soma(c, somar_secao(s, c->coluna), texto);
          imprimir_linha_cheia(saida, texto, interna);
        }
      imprimir_borda_lisa(saida, interna);
    }
    fputc('\n', saida);
  }

  // area 3: rodape
  tem_rodape = modelo->mostrar_total;
  for (c = modelo->colunas; c != NULL; c = c->prox)
    if (deve_somar(c))
      tem_rodape = 1;
  if (!tem_rodape)
    return;

  imprimir_borda_lisa(saida, interna);
  if (modelo->mostrar_total) {
    snprintf(texto, sizeof(texto), "Total de registros: %d", contar_registros(modelo));
    imprimir_linha_cheia(saida, texto, interna);
  }
  for (c = modelo->colunas; c != NULL; c = c->prox)
    if (deve_somar(c)) {
      texto_soma(c, somar_modelo(modelo, c->coluna), texto);
      imprimir_linha_cheia(saida, texto, interna);
    }
  imprimir_borda_lisa(saida, interna);
}

static void liberar_campos(Campo *c) {
  while (c != NULL) {
    Campo *prox = c->prox;
    free(c);
    c = prox;
  }
}

static void liberar_modelo(Modelo *modelo) {
  Secao *s = modelo->secoes;
  while (s != NULL) {
    Linha *l = s->linhas;
    while (l != NULL) {
      Linha *prox_linha = l->prox;
      free(l);
      l = prox_linha;
    }
    Secao *prox_secao = s->prox;
    free(s);
    s = prox_secao;
  }
  liberar_campos(modelo->colunas);
  liberar_campos(modelo->disponiveis);
  modelo->secoes = modelo->ultima_secao = NULL;
  modelo->colunas = modelo->disponiveis = NULL;
}

static void finalizar_relatorio(FILE *saida, Modelo *modelo, const char *caminho_saida) {
  int registros = contar_registros(modelo);
  int erro;

  escrever_relatorio(saida, modelo);
  erro = ferror(saida);
  if (fclose(saida) != 0)
    erro = 1;

  if (erro)
    printf("Erro ao gravar o arquivo '%s'\n", caminho_saida);
  else if (registros == 0)
    printf("Aviso: '%s' gerado sem nenhum registro (confira os arquivos em input/)\n", caminho_saida);
  else
    printf("Relatorio gerado com sucesso em '%s' (%d registros)\n", caminho_saida, registros);

  liberar_modelo(modelo);
}

static FILE *abrir_saida(const char *caminho_saida) {
  FILE *saida = fopen(caminho_saida, "w");
  if (!saida)
    printf("Erro ao criar o arquivo '%s' (a pasta output existe?)\n", caminho_saida);
  return saida;
}

void gerarRelatorioUsuariosAssinaturas(Usuario *usuarios,
                                       Assinatura *assinaturas,
                                       const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "RELATORIO DE USUARIOS E ASSINATURAS");
  adicionar_campo(&modelo, "ID", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Nome", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Email", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "CPF", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Plano/Tipo", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Valor", TIPO_MOEDA, 0);
  adicionar_campo(&modelo, "Pagamento", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Vencimento", TIPO_TEXTO, 0);
  configurar_modelo(&modelo);

  for (Usuario *u = usuarios; u != NULL; u = u->prox) {
    Assinatura *a = buscar_assinatura(assinaturas, u->id_assinatura);
    Linha *l = nova_linha();
    if (!l)
      break;

    definir_numero(l, 0, u->id);
    snprintf(l->texto[1], TAM_VALOR, "%s", u->nome);
    snprintf(l->texto[2], TAM_VALOR, "%s", u->email);
    snprintf(l->texto[3], TAM_VALOR, "%s", u->cpf);

    if (a != NULL) {
      snprintf(l->texto[4], TAM_VALOR, "%s", a->tipo);
      definir_numero(l, 5, a->valor);
      snprintf(l->texto[6], TAM_VALOR, "%s", a->metodo_pagamento);
      snprintf(l->texto[7], TAM_VALOR, "%s", a->vencimento);
    } else {
      snprintf(l->texto[4], TAM_VALOR, "Nenhuma");
      snprintf(l->texto[5], TAM_VALOR, "-");
      snprintf(l->texto[6], TAM_VALOR, "-");
      snprintf(l->texto[7], TAM_VALOR, "-");
    }

    adicionar_linha(&modelo, l);
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}

void gerarRelatorioCatalogoMidias(Midia *midias, Filme *filmes, Serie *series,
                                  const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "CATALOGO DE MIDIAS");
  adicionar_campo(&modelo, "ID", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Nome", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Tipo", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Categoria", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Diretor", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Duracao (min)", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Lancamento", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Avaliacao", TIPO_DECIMAL, 0);
  adicionar_campo(&modelo, "Detalhes Especificos", TIPO_TEXTO, 0);
  configurar_modelo(&modelo);

  if (midias != NULL) {
    for (Midia *m = midias; m != NULL; m = m->prox) {
      Filme *f = buscar_filme(filmes, m->id);
      Serie *s = buscar_serie(series, m->id);
      Linha *l = nova_linha();
      if (!l)
        break;

      definir_numero(l, 0, m->id);
      snprintf(l->texto[1], TAM_VALOR, "%s", m->nome);
      snprintf(l->texto[2], TAM_VALOR, "%s",
               (f != NULL || m->tipo == 'F') ? "Filme" : "Serie");
      snprintf(l->texto[3], TAM_VALOR, "%s", m->categoria);
      snprintf(l->texto[4], TAM_VALOR, "%s", m->diretor);
      definir_numero(l, 5, m->duracao);
      snprintf(l->texto[6], TAM_VALOR, "%s", m->data_lancamento);
      definir_numero(l, 7, m->avaliacao_critica);

      if (f != NULL) {
        char bilheteria[TAM_VALOR];
        formatar_moeda(f->bilheteria, 2, bilheteria);
        snprintf(l->texto[8], TAM_VALOR, "Bilheteria: %.60s", bilheteria);
      } else if (s != NULL) {
        snprintf(l->texto[8], TAM_VALOR, "%d ep / %d temp", s->numero_episodios,
                 s->numero_temporadas);
      } else {
        snprintf(l->texto[8], TAM_VALOR, "N/D");
      }

      adicionar_linha(&modelo, l);
    }
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}

void gerarRelatorioCatalogoSeries(Midia *midias, Serie *series,
                                  const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "CATALOGO DE SERIES");
  adicionar_campo(&modelo, "ID", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Nome", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Categoria", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Diretor", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Duracao (min)", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Lancamento", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Avaliacao", TIPO_DECIMAL, 0);
  adicionar_campo(&modelo, "Episodios", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Temporadas", TIPO_INTEIRO, 0);
  configurar_modelo(&modelo);

  //as midias q n são serie são ignoradas
  for (Midia *m = midias; m != NULL; m = m->prox) {
    Serie *s = buscar_serie(series, m->id);
    if (s != NULL) {
      Linha *l = nova_linha();
      if (!l)
        break;

      definir_numero(l, 0, m->id);
      snprintf(l->texto[1], TAM_VALOR, "%s", m->nome);
      snprintf(l->texto[2], TAM_VALOR, "%s", m->categoria);
      snprintf(l->texto[3], TAM_VALOR, "%s", m->diretor);
      definir_numero(l, 4, m->duracao);
      snprintf(l->texto[5], TAM_VALOR, "%s", m->data_lancamento);
      definir_numero(l, 6, m->avaliacao_critica);
      definir_numero(l, 7, s->numero_episodios);
      definir_numero(l, 8, s->numero_temporadas);

      adicionar_linha(&modelo, l);
    }
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}

void gerarRelatorioCatalogoFilmes(Midia *midias, Filme *filmes,
                                  const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "CATALOGO DE FILMES");
  adicionar_campo(&modelo, "ID", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Nome", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Categoria", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Diretor", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Duracao (min)", TIPO_INTEIRO, 0);
  adicionar_campo(&modelo, "Lancamento", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Avaliacao", TIPO_DECIMAL, 0);
  adicionar_campo(&modelo, "Bilheteria", TIPO_MOEDA, 1);
  configurar_modelo(&modelo);

  for (Midia *m = midias; m != NULL; m = m->prox) {
    Filme *f = buscar_filme(filmes, m->id);
    if (f != NULL) {
      Linha *l = nova_linha();
      if (!l)
        break;

      definir_numero(l, 0, m->id);
      snprintf(l->texto[1], TAM_VALOR, "%s", m->nome);
      snprintf(l->texto[2], TAM_VALOR, "%s", m->categoria);
      snprintf(l->texto[3], TAM_VALOR, "%s", m->diretor);
      definir_numero(l, 4, m->duracao);
      snprintf(l->texto[5], TAM_VALOR, "%s", m->data_lancamento);
      definir_numero(l, 6, m->avaliacao_critica);
      definir_numero(l, 7, f->bilheteria);

      adicionar_linha(&modelo, l);
    }
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}
//
//
//
//A PARTIR DE AGORA RELATÓRIOS DO 5 AO 8
//
//
//
void gerarRelatorioConsumoMidiaUsuario(Usuario *usuarios, Midia *midias, MidiaConsumida *consumidas, const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "RELATORIO DE CONSUMO DE MIDIA POR USUARIO");
  adicionar_campo(&modelo, "Usuario", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Midia", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Favorito", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Avaliacao", TIPO_DECIMAL, 0);
  configurar_modelo(&modelo);

  for (MidiaConsumida *c = consumidas; c != NULL; c = c->prox) {
    Usuario *u = buscar_usuario(usuarios, c->id_usuario);
    Midia *m = buscar_midia(midias, c->id_midia);
    Linha *l = nova_linha();
    if (!l)
      break;

    snprintf(l->texto[0], TAM_VALOR, "%s", u ? u->nome : "Desconhecido");
    snprintf(l->texto[1], TAM_VALOR, "%s", m ? m->nome : "Desconhecida");
    snprintf(l->texto[2], TAM_VALOR, "%s", c->favorito ? "Sim" : "Nao");
    definir_numero(l, 3, c->avaliacao_usuario);

    adicionar_linha(&modelo, l);
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}

void gerarRelatorioConsumoSeriesUsuario(Usuario *usuarios, Midia *midias, Serie *series, MidiaConsumida *consumidas, const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "RELATORIO DE CONSUMO DE SERIES POR USUARIO");
  adicionar_campo(&modelo, "Usuario", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Serie", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Favorito", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Avaliacao", TIPO_DECIMAL, 0);
  configurar_modelo(&modelo);

  for (MidiaConsumida *c = consumidas; c != NULL; c = c->prox) {
    Midia *m = buscar_midia(midias, c->id_midia);
    if (m && buscar_serie(series, m->id)) {
      Usuario *u = buscar_usuario(usuarios, c->id_usuario);
      Linha *l = nova_linha();
      if (!l)
        break;

      snprintf(l->texto[0], TAM_VALOR, "%s", u ? u->nome : "Desconhecido");
      snprintf(l->texto[1], TAM_VALOR, "%s", m->nome);
      snprintf(l->texto[2], TAM_VALOR, "%s", c->favorito ? "Sim" : "Nao");
      definir_numero(l, 3, c->avaliacao_usuario);

      adicionar_linha(&modelo, l);
    }
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}

void gerarRelatorioConsumoFilmesUsuario(Usuario *usuarios, Midia *midias, Filme *filmes, MidiaConsumida *consumidas, const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "RELATORIO DE CONSUMO DE FILMES POR USUARIO");
  adicionar_campo(&modelo, "Usuario", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Filme", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Favorito", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Avaliacao", TIPO_DECIMAL, 0);
  configurar_modelo(&modelo);

  for (MidiaConsumida *c = consumidas; c != NULL; c = c->prox) {
    Midia *m = buscar_midia(midias, c->id_midia);
    if (m && buscar_filme(filmes, m->id)) {
      Usuario *u = buscar_usuario(usuarios, c->id_usuario);
      Linha *l = nova_linha();
      if (!l)
        break;

      snprintf(l->texto[0], TAM_VALOR, "%s", u ? u->nome : "Desconhecido");
      snprintf(l->texto[1], TAM_VALOR, "%s", m->nome);
      snprintf(l->texto[2], TAM_VALOR, "%s", c->favorito ? "Sim" : "Nao");
      definir_numero(l, 3, c->avaliacao_usuario);

      adicionar_linha(&modelo, l);
    }
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}

void gerarRelatorioFaturamentoPagamento(Assinatura *assinaturas, const char *caminho_saida) {
  FILE *saida = abrir_saida(caminho_saida);
  if (!saida)
    return;

  Modelo modelo;
  iniciar_modelo(&modelo, "RELATORIO DE FATURAMENTO POR METODO DE PAGAMENTO");
  adicionar_campo(&modelo, "Metodo", TIPO_TEXTO, 0);
  adicionar_campo(&modelo, "Assinaturas", TIPO_INTEIRO, 1);
  adicionar_campo(&modelo, "Total", TIPO_MOEDA, 1);
  configurar_modelo(&modelo);

  // agrupa por metodo numa lista temporaria (antes eram vetores fixos de 30 posicoes)
  Linha *metodos = NULL, *ultimo = NULL;
  for (Assinatura *a = assinaturas; a != NULL; a = a->prox) {
    Linha *l;
    for (l = metodos; l != NULL; l = l->prox)
      if (strcmp(l->texto[0], a->metodo_pagamento) == 0)
        break;

    if (l == NULL) {
      l = nova_linha();
      if (!l)
        break;
      snprintf(l->texto[0], TAM_VALOR, "%s", a->metodo_pagamento);
      definir_numero(l, 1, 0);
      definir_numero(l, 2, 0);
      if (ultimo == NULL)
        metodos = l;
      else
        ultimo->prox = l;
      ultimo = l;
    }

    l->numero[1] += 1;
    l->numero[2] += a->valor;
  }

  // passa as linhas para o modelo (adicionar_linha mexe no prox, por isso guarda antes)
  while (metodos != NULL) {
    Linha *proxima = metodos->prox;
    adicionar_linha(&modelo, metodos);
    metodos = proxima;
  }

  finalizar_relatorio(saida, &modelo, caminho_saida);
}
