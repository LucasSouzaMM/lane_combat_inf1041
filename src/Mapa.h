#pragma once

#include <string>
#include <vector>

struct Posicao {
  int x;
  int y;
  bool operator==(const Posicao& outra) const {
    return x == outra.x && y == outra.y;
  }
  bool operator!=(const Posicao& outra) const { return !(*this == outra); }
};

// Alcance em círculo: dx² + dy² <= alcance².
bool dentroDoAlcance(Posicao centro, double alcance, Posicao alvo);

// Grade de casas: '.' é chão (aceita torres), '#' é caminho (aceita
// obstáculos) e '@' é o Jogador. As entradas são as casas de caminho na borda.
class Mapa {
public:
  explicit Mapa(std::vector<std::string> linhas);
  int largura() const;
  int altura() const;
  char em(Posicao posicao) const;
  bool dentro(Posicao posicao) const;
  Posicao base() const;
  // Em ordem de leitura: de cima para baixo, da esquerda para a direita.
  const std::vector<Posicao>& entradas() const;
  // A casa seguinte no caminho em direção à base. Na base, a própria base.
  Posicao proximo(Posicao posicao) const;
  // Número de casas até a base pelo caminho.
  int distancia(Posicao posicao) const;

private:
  int indice(Posicao posicao) const;

  std::vector<std::string> linhas_;
  Posicao base_{0, 0};
  std::vector<Posicao> entradas_;
  std::vector<int> distancia_;
  std::vector<Posicao> proximo_;
};
