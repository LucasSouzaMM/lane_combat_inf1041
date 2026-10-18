#include "Mapa.h"

#include <climits>
#include <queue>

bool dentroDoAlcance(Posicao centro, double alcance, Posicao alvo) {
  int dx = alvo.x - centro.x, dy = alvo.y - centro.y;
  return dx * dx + dy * dy <= alcance * alcance;
}

Mapa::Mapa(std::vector<std::string> linhas) : linhas_(std::move(linhas)) {
  for (int y = 0; y < altura(); ++y)
    for (int x = 0; x < largura(); ++x) {
      Posicao p{x, y};
      if (em(p) == '@')
        base_ = p;
      bool borda = x == 0 || y == 0 || x == largura() - 1 || y == altura() - 1;
      if (borda && em(p) == '#')
        entradas_.push_back(p);
    }

  // Busca em largura a partir da base. Cada casa aponta para a vizinha mais
  // perto da base. Os caminhos não podem encostar um no outro, senão a busca
  // pega um atalho entre eles.
  distancia_.assign(largura() * altura(), INT_MAX);
  proximo_.assign(largura() * altura(), base_);
  distancia_[indice(base_)] = 0;
  std::queue<Posicao> fila;
  fila.push(base_);
  while (!fila.empty()) {
    Posicao atual = fila.front();
    fila.pop();
    const Posicao vizinhos[] = {{atual.x + 1, atual.y},
                                {atual.x - 1, atual.y},
                                {atual.x, atual.y + 1},
                                {atual.x, atual.y - 1}};
    for (Posicao v : vizinhos)
      if (dentro(v) && em(v) == '#' && distancia_[indice(v)] == INT_MAX) {
        distancia_[indice(v)] = distancia_[indice(atual)] + 1;
        proximo_[indice(v)] = atual;
        fila.push(v);
      }
  }
}

int Mapa::largura() const {
  return linhas_.empty() ? 0 : static_cast<int>(linhas_[0].size());
}

int Mapa::altura() const { return static_cast<int>(linhas_.size()); }

char Mapa::em(Posicao posicao) const { return linhas_[posicao.y][posicao.x]; }

bool Mapa::dentro(Posicao p) const {
  return p.x >= 0 && p.y >= 0 && p.x < largura() && p.y < altura();
}

Posicao Mapa::base() const { return base_; }

const std::vector<Posicao> &Mapa::entradas() const { return entradas_; }

Posicao Mapa::proximo(Posicao posicao) const {
  return proximo_[indice(posicao)];
}

int Mapa::distancia(Posicao posicao) const {
  return distancia_[indice(posicao)];
}

int Mapa::indice(Posicao posicao) const {
  return posicao.y * largura() + posicao.x;
}
