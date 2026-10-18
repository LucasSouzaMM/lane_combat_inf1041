#include "Jogo.h"

#include <ncurses.h>

#include <chrono>
#include <clocale>
#include <thread>

#include "Estados.h"
#include "Valores.h"

Jogo::Jogo(std::string caminhoValores)
    : caminhoValores_(std::move(caminhoValores)) {}

void Jogo::mudarEstado(std::unique_ptr<EstadoJogo> estado) {
  proximo_ = std::move(estado);
}

void Jogo::sair() { sair_ = true; }

const std::string& Jogo::caminhoValores() const { return caminhoValores_; }

void Jogo::executar() {
  std::setlocale(LC_ALL, "");  // acentos em UTF-8
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(COR_INIMIGO, COLOR_RED, -1);
    init_pair(COR_LENTO, COLOR_CYAN, -1);
    init_pair(COR_TORRE, COLOR_GREEN, -1);
    init_pair(COR_OBSTACULO, COLOR_YELLOW, -1);
    init_pair(COR_BASE, COLOR_MAGENTA, -1);
    init_pair(COR_CAMINHO, COLOR_YELLOW, -1);
    init_pair(COR_ALCANCE, COLOR_BLUE, -1);
    init_pair(COR_TITULO, COLOR_YELLOW, -1);
  }

  estado_ = std::make_unique<EstadoMenu>(*this);
  // Tick fixo (RNF04): a velocidade do jogo não depende da máquina.
  const auto passo = std::chrono::milliseconds(1000 / TICKS_POR_SEGUNDO);
  auto proximoTick = std::chrono::steady_clock::now();
  while (!sair_) {
    int tecla;
    while (!proximo_ && !sair_ && (tecla = getch()) != ERR)
      estado_->tratarEntrada(tecla);
    if (!proximo_ && !sair_) estado_->atualizar();
    if (proximo_) estado_ = std::move(proximo_);

    erase();
    if (COLS < 80 || LINES < 24)
      mvprintw(0, 0, "Aumente o terminal para 80x24 (agora %dx%d).", COLS, LINES);
    else
      estado_->desenhar();
    refresh();

    proximoTick += passo;
    std::this_thread::sleep_until(proximoTick);
  }
  endwin();
}
