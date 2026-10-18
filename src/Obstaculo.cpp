#include "Obstaculo.h"

Obstaculo::Obstaculo(Posicao posicao, int vida) : posicao_(posicao), vida_(vida) {}

Posicao Obstaculo::posicao() const { return posicao_; }

int Obstaculo::vida() const { return vida_; }

bool Obstaculo::vivo() const { return vida_ > 0; }

void Obstaculo::receberDano(int dano) { vida_ -= dano; }
