#pragma once

#include "Inimigo.h"
#include "Valores.h"

// Factory: cria cada tipo de inimigo com os valores do arquivo, ajustados
// pela dificuldade.
class FabricaInimigos {
public:
  FabricaInimigos(const Valores& valores, const Dificuldade& dificuldade);
  Inimigo criar(TipoInimigo tipo, Posicao posicao) const;

private:
  std::array<ValoresInimigo, 3> inimigos_;
  int intervaloAtaque_;
  Dificuldade dificuldade_;
};
