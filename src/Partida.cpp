#include "Partida.h"

#include <algorithm>
#include <memory>
#include <utility>

Partida::Partida(Mapa mapa, const Valores &valores,
                 const Dificuldade &dificuldade, std::vector<Onda> ondas)
    : mapa_(std::move(mapa)), valores_(valores),
      jogador_(valores.vidaJogador, dificuldade.ouroInicial),
      ondas_(std::move(ondas), valores.inicioOndas, valores.intervaloOndas,
             valores.espacamento),
      fabrica_(valores, dificuldade) {}

Torre Partida::criarTorre(TipoTorre tipo, Posicao posicao) const {
  const ValoresTorre &v = valores(tipo);
  std::unique_ptr<EstrategiaAtaque> estrategia;
  switch (tipo) {
  case TipoTorre::Area:
    estrategia = std::make_unique<AtaqueArea>(v.dano);
    break;
  case TipoTorre::Lenta:
    estrategia = std::make_unique<AtaqueLento>(v.dano, valores_.lentidao,
                                               valores_.duracaoLentidao);
    break;
  case TipoTorre::Simples:
  case TipoTorre::Longa:
    estrategia = std::make_unique<AtaqueSimples>(v.dano);
    break;
  }
  return Torre(tipo, posicao, v.alcance, v.recarga, std::move(estrategia));
}

void Partida::tick() {
  if (terminou())
    return;

  for (Disparo &disparo : disparos_)
    --disparo.ticks;
  disparos_.erase(std::remove_if(disparos_.begin(), disparos_.end(),
                                 [](const Disparo &d) { return d.ticks <= 0; }),
                  disparos_.end());

  int completas = ondas_.ondasCompletas();
  for (const Grupo &grupo : ondas_.tick())
    inimigos_.push_back(
        fabrica_.criar(grupo.tipo, mapa_.entradas()[grupo.entrada]));
  if (ondas_.ondasCompletas() > completas)
    jogador_.ganhar(valores_.bonusOnda);

  for (Torre &torre : torres_)
    for (Posicao alvo : torre.tick(inimigos_, mapa_))
      disparos_.push_back(
          {torre.posicao(), alvo, torre.tipo(), DURACAO_DISPARO});

  for (Inimigo &inimigo : inimigos_)
    if (inimigo.vivo())
      inimigo.tick(mapa_, bloqueio(mapa_.proximo(inimigo.posicao())));

  obstaculos_.erase(
      std::remove_if(obstaculos_.begin(), obstaculos_.end(),
                     [](const Obstaculo &o) { return !o.vivo(); }),
      obstaculos_.end());

  std::vector<Inimigo> restantes;
  for (const Inimigo &inimigo : inimigos_) {
    if (!inimigo.vivo())
      jogador_.ganhar(inimigo.ouro());
    else if (inimigo.posicao() == mapa_.base())
      jogador_.receberDano(inimigo.dano());
    else
      restantes.push_back(inimigo);
  }
  inimigos_ = std::move(restantes);
}

bool Partida::livreParaTorre(Posicao posicao) const {
  return mapa_.dentro(posicao) && mapa_.em(posicao) == '.' && !torreEm(posicao);
}

bool Partida::livreParaObstaculo(Posicao posicao) const {
  const std::vector<Posicao> &entradas = mapa_.entradas();
  return mapa_.dentro(posicao) && mapa_.em(posicao) == '#' &&
         !obstaculoEm(posicao) &&
         std::find(entradas.begin(), entradas.end(), posicao) == entradas.end();
}

bool Partida::construirTorre(Posicao posicao, TipoTorre tipo) {
  if (!livreParaTorre(posicao) || !jogador_.gastar(valores(tipo).custo))
    return false;
  torres_.push_back(criarTorre(tipo, posicao));
  return true;
}

bool Partida::construirObstaculo(Posicao posicao) {
  if (!livreParaObstaculo(posicao) || !jogador_.gastar(valores_.custoObstaculo))
    return false;
  obstaculos_.emplace_back(posicao, valores_.vidaObstaculo);
  return true;
}

void Partida::chamarOnda() { ondas_.chamarProxima(); }

bool Partida::terminou() const { return !jogador_.vivo() || venceu(); }

bool Partida::venceu() const {
  return jogador_.vivo() && ondas_.terminou() && inimigos_.empty();
}

const ValoresTorre &Partida::valores(TipoTorre tipo) const {
  return valores_.torres[static_cast<int>(tipo)];
}

int Partida::custoObstaculo() const { return valores_.custoObstaculo; }

const Torre *Partida::torreEm(Posicao posicao) const {
  for (const Torre &torre : torres_)
    if (torre.posicao() == posicao)
      return &torre;
  return nullptr;
}

const Obstaculo *Partida::obstaculoEm(Posicao posicao) const {
  for (const Obstaculo &obstaculo : obstaculos_)
    if (obstaculo.posicao() == posicao)
      return &obstaculo;
  return nullptr;
}

Obstaculo *Partida::bloqueio(Posicao posicao) {
  return const_cast<Obstaculo *>(std::as_const(*this).obstaculoEm(posicao));
}

const Jogador &Partida::jogador() const { return jogador_; }

const Mapa &Partida::mapa() const { return mapa_; }

const GerenciadorOndas &Partida::ondas() const { return ondas_; }

const std::vector<Inimigo> &Partida::inimigos() const { return inimigos_; }

const std::vector<Torre> &Partida::torres() const { return torres_; }

const std::vector<Disparo> &Partida::disparos() const { return disparos_; }
