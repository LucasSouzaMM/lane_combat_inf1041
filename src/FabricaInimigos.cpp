#include "FabricaInimigos.h"

FabricaInimigos::FabricaInimigos(const Valores& valores,
                                 const Dificuldade& dificuldade)
    : inimigos_(valores.inimigos), intervaloAtaque_(valores.intervaloAtaque),
      dificuldade_(dificuldade) {}

Inimigo FabricaInimigos::criar(TipoInimigo tipo, Posicao posicao) const {
  ValoresInimigo valores = inimigos_[static_cast<int>(tipo)];
  valores.vida = valores.vida * dificuldade_.vidaPct / 100;
  valores.velocidade = valores.velocidade * dificuldade_.velocidadePct / 100;
  return Inimigo(tipo, valores, posicao, intervaloAtaque_);
}
