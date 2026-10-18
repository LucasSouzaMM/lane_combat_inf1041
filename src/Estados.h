#pragma once

#include <string>

#include "Jogo.h"
#include "Partida.h"
#include "Valores.h"

class EstadoMenu : public EstadoJogo {
public:
  explicit EstadoMenu(Jogo& jogo);
  void tratarEntrada(int tecla) override;
  void desenhar() override;

private:
  Jogo& jogo_;
  int linha_ = 0;        // 0: mapa, 1: dificuldade
  int mapa_ = 1;         // 1 a 3
  int dificuldade_ = 1;  // 0: fácil, 1: normal, 2: difícil
  std::string erro_;     // erro ao ler o arquivo de valores
};

class EstadoPartida : public EstadoJogo {
public:
  EstadoPartida(Jogo& jogo, int mapa, const Valores& valores, int dificuldade);
  void tratarEntrada(int tecla) override;
  void atualizar() override;
  void desenhar() override;

private:
  void construir();
  void avisar(const std::string& mensagem);
  void moverCursor(int dx, int dy);
  void desenharMapa() const;
  void desenharPainel() const;

  Jogo& jogo_;
  Partida partida_;
  Posicao cursor_;
  int escolha_ = 0;  // 0 a 3: TipoTorre; 4: obstáculo
  bool pausado_ = false;
  std::string mensagem_;
  int mensagemTicks_ = 0;
};

class EstadoFim : public EstadoJogo {
public:
  EstadoFim(Jogo& jogo, bool venceu, int vida, int onda, int totalOndas);
  void tratarEntrada(int tecla) override;
  void desenhar() override;

private:
  Jogo& jogo_;
  bool venceu_;
  int vida_;
  int onda_;
  int totalOndas_;
};
