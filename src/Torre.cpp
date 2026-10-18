#include "Torre.h"

Torre::Torre(TipoTorre tipo, Posicao posicao, double alcance, int recarga,
             std::unique_ptr<EstrategiaAtaque> estrategia)
    : tipo_(tipo), posicao_(posicao), alcance_(alcance), recarga_(recarga),
      estrategia_(std::move(estrategia)) {}

TipoTorre Torre::tipo() const { return tipo_; }

Posicao Torre::posicao() const { return posicao_; }

double Torre::alcance() const { return alcance_; }

std::vector<Posicao> Torre::tick(std::vector<Inimigo>& inimigos,
                                 const Mapa& mapa) {
  if (espera_ > 0) {
    --espera_;
    return {};
  }
  std::vector<Posicao> atingidos =
      estrategia_->atacar(posicao_, alcance_, inimigos, mapa);
  if (!atingidos.empty()) espera_ = recarga_ - 1;  // um ataque a cada `recarga` ticks
  return atingidos;
}
