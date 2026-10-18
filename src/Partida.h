#pragma once

#include <vector>

#include "FabricaInimigos.h"
#include "GerenciadorOndas.h"
#include "Jogador.h"
#include "Mapa.h"
#include "Obstaculo.h"
#include "Torre.h"
#include "Valores.h"

// O rastro de um ataque, só para a tela desenhar.
struct Disparo {
  Posicao de;
  Posicao para;
  TipoTorre tipo;
  int ticks;  // quanto tempo ainda aparece
};

// Toda a lógica de uma partida, sem ncurses.
class Partida {
public:
  static constexpr int DURACAO_DISPARO = 2;

  Partida(Mapa mapa, const Valores& valores, const Dificuldade& dificuldade,
          std::vector<Onda> ondas);
  void tick();
  // Torres vão no chão ('.') e obstáculos no caminho ('#'), fora das entradas.
  bool livreParaTorre(Posicao posicao) const;
  bool livreParaObstaculo(Posicao posicao) const;
  // Falham se a casa não está livre ou se falta ouro. Não gastam nada nesse caso.
  bool construirTorre(Posicao posicao, TipoTorre tipo);
  bool construirObstaculo(Posicao posicao);
  void chamarOnda();
  bool terminou() const;
  bool venceu() const;

  const ValoresTorre& valores(TipoTorre tipo) const;
  int custoObstaculo() const;
  const Torre* torreEm(Posicao posicao) const;
  const Obstaculo* obstaculoEm(Posicao posicao) const;
  const Jogador& jogador() const;
  const Mapa& mapa() const;
  const GerenciadorOndas& ondas() const;
  const std::vector<Inimigo>& inimigos() const;
  const std::vector<Torre>& torres() const;
  const std::vector<Disparo>& disparos() const;

private:
  Torre criarTorre(TipoTorre tipo, Posicao posicao) const;
  Obstaculo* bloqueio(Posicao posicao);  // obstáculo nesta casa, se houver

  Mapa mapa_;
  Valores valores_;
  Jogador jogador_;
  GerenciadorOndas ondas_;
  FabricaInimigos fabrica_;
  std::vector<Inimigo> inimigos_;
  std::vector<Torre> torres_;
  std::vector<Obstaculo> obstaculos_;
  std::vector<Disparo> disparos_;
};
