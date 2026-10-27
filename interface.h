#ifndef INTERFACE_H
#define INTERFACE_H

#include "relatorio.h"

int menu_principal(void);
int ler_linha(char *destino, int tamanho);
int ler_inteiro(const char *mensagem);

// pergunta ao usuario o que vai em cada area do modelo (cabecalho, corpo, rodape)
void configurar_modelo(Modelo *modelo);

#endif
