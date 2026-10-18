#pragma once

#include <memory>
#include <string>

// Pares de cor do ncurses.
enum Cor {
  COR_INIMIGO = 1,
  COR_LENTO,
  COR_TORRE,
  COR_OBSTACULO,
  COR_BASE,
  COR_CAMINHO,
  COR_ALCANCE,
  COR_TITULO
};

// State: uma tela do jogo.
class EstadoJogo {
public:
  virtual ~EstadoJogo() = default;
  virtual void tratarEntrada(int tecla) = 0;
  virtual void atualizar() {}
  virtual void desenhar() = 0;
};

class Jogo {
public:
  explicit Jogo(std::string caminhoValores);
  void executar();
  // A troca acontece no fim do tick, para o estado atual não ser destruído
  // enquanto um método dele ainda está rodando.
  void mudarEstado(std::unique_ptr<EstadoJogo> estado);
  void sair();
  const std::string& caminhoValores() const;

private:
  std::string caminhoValores_;
  std::unique_ptr<EstadoJogo> estado_;
  std::unique_ptr<EstadoJogo> proximo_;
  bool sair_ = false;
};
