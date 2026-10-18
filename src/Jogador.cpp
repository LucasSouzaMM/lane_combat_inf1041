#include "Jogador.h"

Jogador::Jogador(int vida, int ouro) : vida_(vida), ouro_(ouro) {}

int Jogador::vida() const { return vida_; }

int Jogador::ouro() const { return ouro_; }

bool Jogador::vivo() const { return vida_ > 0; }

bool Jogador::gastar(int valor) {
  if (valor > ouro_) return false;
  ouro_ -= valor;
  return true;
}

void Jogador::ganhar(int valor) { ouro_ += valor; }

void Jogador::receberDano(int dano) { vida_ -= dano; }
