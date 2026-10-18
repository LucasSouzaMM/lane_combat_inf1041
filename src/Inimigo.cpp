#include "Inimigo.h"

#include <algorithm>

#include "Obstaculo.h"

Inimigo::Inimigo(TipoInimigo tipo, ValoresInimigo valores, Posicao posicao,
                 int intervaloAtaque)
    : tipo_(tipo), valores_(valores), posicao_(posicao),
      intervaloAtaque_(intervaloAtaque) {}

bool Inimigo::vivo() const { return valores_.vida > 0; }

bool Inimigo::lento() const { return lentoPor_ > 0; }

int Inimigo::vida() const { return valores_.vida; }

int Inimigo::dano() const { return valores_.dano; }

int Inimigo::ouro() const { return valores_.ouro; }

Posicao Inimigo::posicao() const { return posicao_; }

TipoInimigo Inimigo::tipo() const { return tipo_; }

void Inimigo::receberDano(int dano) { valores_.vida -= dano; }

void Inimigo::desacelerar(double fracao, int ticks) {
  lentidao_ = fracao;
  lentoPor_ = ticks;
}

void Inimigo::tick(const Mapa& mapa, Obstaculo* frente) {
  double fator = lento() ? 1 - lentidao_ : 1;
  if (lentoPor_ > 0) --lentoPor_;
  if (frente) {
    if (esperaAtaque_ > 0) {
      --esperaAtaque_;
    } else {
      frente->receberDano(valores_.dano);
      esperaAtaque_ = intervaloAtaque_ - 1;
    }
    return;
  }
  // ponytail: no máximo uma casa por tick (10 casas/s), porque `frente` só
  // vale para a próxima casa.
  progresso_ += valores_.velocidade * fator;
  if (progresso_ >= 1) {
    progresso_ = std::min(progresso_ - 1, 0.99);
    posicao_ = mapa.proximo(posicao_);
  }
}
