#pragma once

#include <vector>

#include "Inimigo.h"

// `quantidade` inimigos do mesmo tipo, um de cada vez, pela mesma entrada.
struct Grupo {
  TipoInimigo tipo;
  int quantidade;
  int entrada;  // índice em Mapa::entradas()
};

// Os grupos de uma mesma entrada entram um depois do outro. Entradas
// diferentes criam inimigos ao mesmo tempo.
using Onda = std::vector<Grupo>;

class GerenciadorOndas {
public:
  // Tempos em ticks: `inicio` antes da onda 1, `intervalo` entre o fim de uma
  // onda e a próxima, `espacamento` entre dois inimigos da mesma entrada.
  GerenciadorOndas(std::vector<Onda> ondas, int inicio, int intervalo,
                   int espacamento);
  // Os inimigos que entram neste tick (um Grupo por inimigo).
  std::vector<Grupo> tick();
  // Começa a próxima onda agora, se estiver esperando entre ondas.
  void chamarProxima();
  bool terminou() const;  // todos os inimigos já entraram
  bool entreOndas() const;
  int ticksParaProxima() const;
  int ondasCompletas() const;  // ondas que já terminaram de entrar
  int ondaAtual() const;       // de 1 a totalOndas()
  int totalOndas() const;

private:
  struct Fila {
    std::vector<Grupo> grupos;
    int grupo = 0;
    int criados = 0;
    int espera = 0;
  };
  void iniciarOnda();

  std::vector<Onda> ondas_;
  int intervalo_;
  int espacamento_;
  int onda_ = 0;
  int espera_;
  bool entreOndas_ = true;
  std::vector<Fila> filas_;  // uma por entrada, da onda atual
};
