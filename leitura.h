#ifndef LEITURA_H
#define LEITURA_H

#include "estruturas.h"

Assinatura *ler_assinaturas(const char *caminho);
Usuario *ler_usuarios(const char *caminho);
Midia *ler_midias(const char *caminho);
Filme *ler_filmes(const char *caminho);
Serie *ler_series(const char *caminho);
MidiaConsumida *ler_midias_consumidas(const char *caminho);

#endif
