#include "Ataques.h"

namespace {

bool noAlcance(const Inimigo& inimigo, Posicao centro, double alcance) {
  return inimigo.vivo() && dentroDoAlcance(centro, alcance, inimigo.posicao());
}

// O alvo é o inimigo do alcance mais perto do Jogador pelo caminho.
Inimigo* alvo(Posicao centro, double alcance, std::vector<Inimigo>& inimigos,
              const Mapa& mapa) {
  Inimigo* melhor = nullptr;
  for (Inimigo& inimigo : inimigos)
    if (noAlcance(inimigo, centro, alcance) &&
        (!melhor || mapa.distancia(inimigo.posicao()) <
                        mapa.distancia(melhor->posicao())))
      melhor = &inimigo;
  return melhor;
}

}  // namespace

AtaqueSimples::AtaqueSimples(int dano) : dano_(dano) {}

std::vector<Posicao> AtaqueSimples::atacar(Posicao centro, double alcance,
                                           std::vector<Inimigo>& inimigos,
                                           const Mapa& mapa) {
  Inimigo* inimigo = alvo(centro, alcance, inimigos, mapa);
  if (!inimigo) return {};
  inimigo->receberDano(dano_);
  return {inimigo->posicao()};
}

AtaqueArea::AtaqueArea(int dano) : dano_(dano) {}

std::vector<Posicao> AtaqueArea::atacar(Posicao centro, double alcance,
                                        std::vector<Inimigo>& inimigos,
                                        const Mapa&) {
  std::vector<Posicao> atingidos;
  for (Inimigo& inimigo : inimigos)
    if (noAlcance(inimigo, centro, alcance)) {
      inimigo.receberDano(dano_);
      atingidos.push_back(inimigo.posicao());
    }
  return atingidos;
}

AtaqueLento::AtaqueLento(int dano, double lentidao, int duracao)
    : dano_(dano), lentidao_(lentidao), duracao_(duracao) {}

std::vector<Posicao> AtaqueLento::atacar(Posicao centro, double alcance,
                                         std::vector<Inimigo>& inimigos,
                                         const Mapa& mapa) {
  Inimigo* inimigo = alvo(centro, alcance, inimigos, mapa);
  if (!inimigo) return {};
  inimigo->receberDano(dano_);
  inimigo->desacelerar(lentidao_, duracao_);
  return {inimigo->posicao()};
}
