#pragma once

#include <memory>
#include <vector>

#include "Ataques.h"

enum class TipoTorre { Simples, Area, Lenta, Longa };

struct ValoresTorre {
  int custo;
  double alcance;  // casas
  int recarga;     // ticks entre dois ataques
  int dano;
};

class Torre {
public:
  Torre(TipoTorre tipo, Posicao posicao, double alcance, int recarga,
        std::unique_ptr<EstrategiaAtaque> estrategia);
  TipoTorre tipo() const;
  Posicao posicao() const;
  double alcance() const;
  // Devolve as posições atingidas neste tick (vazio se não atacou).
  std::vector<Posicao> tick(std::vector<Inimigo>& inimigos, const Mapa& mapa);

private:
  TipoTorre tipo_;
  Posicao posicao_;
  double alcance_;
  int recarga_;
  int espera_ = 0;
  std::unique_ptr<EstrategiaAtaque> estrategia_;
};
