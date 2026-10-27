#ifndef LEITURA_H
#define LEITURA_H

#include "entidades.h"

Assinatura *ler_assinaturas(const char *caminho);
Usuario *ler_usuarios(const char *caminho);
Midia *ler_midias(const char *caminho);
Filme *ler_filmes(const char *caminho);
Serie *ler_series(const char *caminho);
MidiaConsumida *ler_midias_consumidas(const char *caminho);

void liberar_assinaturas(Assinatura *lista);
void liberar_usuarios(Usuario *lista);
void liberar_midias(Midia *lista);
void liberar_filmes(Filme *lista);
void liberar_series(Serie *lista);
void liberar_midias_consumidas(MidiaConsumida *lista);

#endif
