#pragma once

class Jogador {
public:
  Jogador(int vida, int ouro);
  int vida() const;
  int ouro() const;
  bool vivo() const;
  bool gastar(int valor);
  void ganhar(int valor);
  void receberDano(int dano);

private:
  int vida_;
  int ouro_;
};
