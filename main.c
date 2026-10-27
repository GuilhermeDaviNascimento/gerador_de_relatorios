
#include <stdio.h>
#include "entidades.h"
#include "interface.h"
#include "leitura.h"
#include "relatorio.h"

int main() {

  Usuario *usuarios = ler_usuarios("input/usuarios.txt");
  Assinatura *assinaturas = ler_assinaturas("input/assinaturas.txt");
  Midia *midias = ler_midias("input/midias.txt");
  Filme *filmes = ler_filmes("input/filmes.txt");
  Serie *series = ler_series("input/series.txt");
  MidiaConsumida *consumidas = ler_midias_consumidas("input/midias_consumidas.txt");

  int opcao = 0;
  while (opcao != 9) {

    opcao = menu_principal();
    switch (opcao) {
    case 1: {
      gerarRelatorioUsuariosAssinaturas(usuarios, assinaturas,
                                        "output/relatorio-usuarios.txt");
    } break;
    case 2: {
      gerarRelatorioCatalogoMidias(midias, filmes, series,
                                   "output/relatorio-midias.txt");
    } break;
    case 3: {
      gerarRelatorioCatalogoSeries(midias, series,
                                   "output/relatorio-series.txt");
    } break;
    case 4: {
      gerarRelatorioCatalogoFilmes(midias, filmes,
                                   "output/relatorio-filmes.txt");
    } break;
    case 5:
      gerarRelatorioConsumoMidiaUsuario(usuarios, midias, consumidas, "output/relatorio-consumo-midias.txt");
      break;
    case 6:
      gerarRelatorioConsumoSeriesUsuario(usuarios, midias, series, consumidas, "output/relatorio-consumo-series.txt");
      break;
    case 7:
      gerarRelatorioConsumoFilmesUsuario(usuarios, midias, filmes, consumidas, "output/relatorio-consumo-filmes.txt");
      break;
    case 8:
      gerarRelatorioFaturamentoPagamento(assinaturas, "output/relatorio-faturamento.txt");
      break;
    case 9: //caso 9 é sair do menu (o while termina e a memoria é liberada)
      break;
    default:
      printf("Opcao invalida!\n"); //tratamento de erros que a roberta comentou
      break;
    }
  }

  liberar_usuarios(usuarios);
  liberar_assinaturas(assinaturas);
  liberar_midias(midias);
  liberar_filmes(filmes);
  liberar_series(series);
  liberar_midias_consumidas(consumidas);

  return 0;
}
// compilar: gcc *.c -o main
// rodar de dentro desta pasta, para achar input/ e output/: ./main (Linux) ou main.exe (Windows)
