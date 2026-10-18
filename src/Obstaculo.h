#pragma once

#include "Mapa.h"

class Obstaculo {
public:
  Obstaculo(Posicao posicao, int vida);
  Posicao posicao() const;
  int vida() const;
  bool vivo() const;
  void receberDano(int dano);

private:
  Posicao posicao_;
  int vida_;
};
