#pragma once

#include <vector>

#include "Inimigo.h"
#include "Mapa.h"

// Strategy: escolhe os alvos no alcance da torre e aplica o efeito.
class EstrategiaAtaque {
public:
  virtual ~EstrategiaAtaque() = default;
  // Devolve as posições atingidas. Vazio: não havia alvo, e a torre não entra
  // em recarga.
  virtual std::vector<Posicao> atacar(Posicao centro, double alcance,
                                      std::vector<Inimigo>& inimigos,
                                      const Mapa& mapa) = 0;
};

// Dano no inimigo do alcance mais perto do Jogador.
class AtaqueSimples : public EstrategiaAtaque {
public:
  explicit AtaqueSimples(int dano);
  std::vector<Posicao> atacar(Posicao centro, double alcance,
                              std::vector<Inimigo>& inimigos,
                              const Mapa& mapa) override;

private:
  int dano_;
};

// Dano em todos os inimigos do alcance.
class AtaqueArea : public EstrategiaAtaque {
public:
  explicit AtaqueArea(int dano);
  std::vector<Posicao> atacar(Posicao centro, double alcance,
                              std::vector<Inimigo>& inimigos,
                              const Mapa& mapa) override;

private:
  int dano_;
};

// Como o AtaqueSimples, e também deixa o alvo lento.
class AtaqueLento : public EstrategiaAtaque {
public:
  AtaqueLento(int dano, double lentidao, int duracao);
  std::vector<Posicao> atacar(Posicao centro, double alcance,
                              std::vector<Inimigo>& inimigos,
                              const Mapa& mapa) override;

private:
  int dano_;
  double lentidao_;
  int duracao_;
};
