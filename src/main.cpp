#include "Jogo.h"

// Uso: td [arquivo de valores]. Sem argumento, usa o valores.txt do projeto.
int main(int argc, char** argv) {
  Jogo jogo(argc > 1 ? argv[1] : VALORES_TXT);
  jogo.executar();
}
