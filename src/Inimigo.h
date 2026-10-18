#pragma once

#include "Mapa.h"

class Obstaculo;

enum class TipoInimigo { Normal, Rapido, Tanque };

struct ValoresInimigo {
  int vida;
  double velocidade;  // casas por tick
  int dano;
  int ouro;
};

class Inimigo {
public:
  // `intervaloAtaque`: ticks entre dois golpes no obstáculo da frente.
  Inimigo(TipoInimigo tipo, ValoresInimigo valores, Posicao posicao,
          int intervaloAtaque);
  bool vivo() const;
  bool lento() const;
  int vida() const;
  int dano() const;
  int ouro() const;
  Posicao posicao() const;
  TipoInimigo tipo() const;
  void receberDano(int dano);
  // Tira `fracao` da velocidade pelos próximos `ticks` ticks. Um novo acerto
  // reinicia o tempo. Os efeitos não se acumulam.
  void desacelerar(double fracao, int ticks);
  // Anda pelo caminho. Se há um obstáculo na casa da frente, para e o ataca.
  void tick(const Mapa& mapa, Obstaculo* frente);

private:
  TipoInimigo tipo_;
  ValoresInimigo valores_;
  Posicao posicao_;
  int intervaloAtaque_;
  double progresso_ = 0;  // fração da casa atual já andada
  int esperaAtaque_ = 0;
  int lentoPor_ = 0;
  double lentidao_ = 0;
};
