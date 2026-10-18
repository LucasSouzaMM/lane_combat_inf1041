#pragma once

#include <array>
#include <istream>
#include <string>
#include <vector>

#include "GerenciadorOndas.h"
#include "Inimigo.h"
#include "Torre.h"

constexpr int TICKS_POR_SEGUNDO = 10;

// Percentuais aplicados aos valores dos inimigos, e o ouro inicial (FE02).
struct Dificuldade {
  std::string nome;
  int vidaPct;
  int velocidadePct;
  int ouroInicial;
};

// Todos os números do jogo. Vêm de valores.txt, já convertidos para ticks.
struct Valores {
  int vidaJogador;
  int bonusOnda;
  int inicioOndas;
  int intervaloOndas;
  int espacamento;
  std::array<ValoresInimigo, 3> inimigos;  // índice: TipoInimigo
  std::array<ValoresTorre, 4> torres;      // índice: TipoTorre
  double lentidao;                         // fração da velocidade
  int duracaoLentidao;
  int custoObstaculo;
  int vidaObstaculo;
  int intervaloAtaque;  // entre dois golpes de um inimigo no obstáculo
  std::array<Dificuldade, 3> dificuldades;
  std::array<std::vector<Onda>, 3> ondas;  // índice: mapa - 1
  std::array<int, 3> ouroMapa;             // somado ao ouro inicial
};

// Lança std::runtime_error com o número da linha do erro.
Valores lerValores(std::istream& entrada);
Valores lerValores(const std::string& caminho);
