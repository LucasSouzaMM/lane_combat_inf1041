#include "Estados.h"

#include <ncurses.h>

#include <algorithm>
#include <cstdlib>
#include <memory>

#include "Mapas.h"

namespace {

constexpr int X_PAINEL = 58;
constexpr int OBSTACULO = 4;  // índice da escolha "obstáculo"
const char* NOMES_DIFICULDADE[] = {"Fácil", "Normal", "Difícil"};

bool confirmar(int tecla) {
  return tecla == '\n' || tecla == KEY_ENTER || tecla == ' ';
}

const char* nome(TipoTorre tipo) {
  switch (tipo) {
  case TipoTorre::Simples: return "Simples";
  case TipoTorre::Area: return "Área";
  case TipoTorre::Lenta: return "Lenta";
  case TipoTorre::Longa: return "Longa";
  }
  return "";
}

char simbolo(TipoTorre tipo) {
  switch (tipo) {
  case TipoTorre::Simples: return 'S';
  case TipoTorre::Area: return 'A';
  case TipoTorre::Lenta: return 'L';
  case TipoTorre::Longa: return 'X';
  }
  return '?';
}

char simbolo(TipoInimigo tipo) {
  switch (tipo) {
  case TipoInimigo::Normal: return 'o';
  case TipoInimigo::Rapido: return 'r';
  case TipoInimigo::Tanque: return 'O';
  }
  return '?';
}

void escrever(int y, int x, int cor, const char* texto, attr_t extra = A_NORMAL) {
  attron(COLOR_PAIR(cor) | extra);
  mvaddstr(y, x, texto);
  attroff(COLOR_PAIR(cor) | extra);
}

void escrever(int y, int x, int cor, char c, attr_t extra = A_NORMAL) {
  char texto[2] = {c, 0};
  escrever(y, x, cor, texto, extra);
}

// O traço de um disparo: um caractere só, pela direção geral do tiro.
char traco(int dx, int dy) {
  if (2 * std::abs(dy) < std::abs(dx)) return '-';
  if (2 * std::abs(dx) < std::abs(dy)) return '|';
  return (dx > 0) == (dy > 0) ? '\\' : '/';
}

}  // namespace

// --- Menu ---

EstadoMenu::EstadoMenu(Jogo& jogo) : jogo_(jogo) {}

void EstadoMenu::tratarEntrada(int tecla) {
  int direcao = tecla == KEY_RIGHT ? 1 : tecla == KEY_LEFT ? -1 : 0;
  if (tecla == KEY_UP || tecla == KEY_DOWN) {
    linha_ = 1 - linha_;
  } else if (direcao && linha_ == 0) {
    mapa_ = std::clamp(mapa_ + direcao, 1, 3);
  } else if (direcao && linha_ == 1) {
    dificuldade_ = std::clamp(dificuldade_ + direcao, 0, 2);
  } else if (confirmar(tecla)) {
    // Lê o arquivo a cada partida: dá para mudar os valores sem fechar o jogo.
    try {
      Valores valores = lerValores(jogo_.caminhoValores());
      jogo_.mudarEstado(
          std::make_unique<EstadoPartida>(jogo_, mapa_, valores, dificuldade_));
    } catch (const std::exception& e) {
      erro_ = e.what();
    }
  } else if (tecla == 'q') {
    jogo_.sair();
  }
}

void EstadoMenu::desenhar() {
  escrever(3, 28, COR_TITULO, "L A N E   D E F E N S E", A_BOLD);
  mvaddstr(5, 14, "Defenda a base (@) dos inimigos que andam pelo caminho.");

  attron(linha_ == 0 ? A_REVERSE : A_NORMAL);
  mvprintw(8, 24, " Mapa:        < %d entrada%s > ", mapa_, mapa_ > 1 ? "s" : " ");
  attroff(A_REVERSE);
  attron(linha_ == 1 ? A_REVERSE : A_NORMAL);
  mvprintw(10, 24, " Dificuldade: < %s >", NOMES_DIFICULDADE[dificuldade_]);
  attroff(A_REVERSE);

  mvaddstr(13, 14, "Inimigos:   o Normal   r Rápido   O Tanque");
  mvaddstr(14, 14, "Torres no chão (.):     S Simples  A Área  L Lenta  X Longa");
  mvaddstr(15, 14, "Obstáculos no caminho (#):  =");
  mvaddstr(18, 14, "Setas: escolher    Enter: jogar    q: sair");
  mvprintw(20, 2, "Valores: %s", jogo_.caminhoValores().c_str());
  if (!erro_.empty()) escrever(22, 2, COR_INIMIGO, erro_.c_str(), A_BOLD);
}

// --- Partida ---

namespace {

// A dificuldade escolhida, com o ouro extra do mapa (mapaN.ouro).
Dificuldade dificuldadeNoMapa(const Valores& valores, int mapa, int dificuldade) {
  Dificuldade d = valores.dificuldades[dificuldade];
  d.ouroInicial += valores.ouroMapa[mapa - 1];
  return d;
}

}  // namespace

EstadoPartida::EstadoPartida(Jogo& jogo, int mapa, const Valores& valores,
                             int dificuldade)
    : jogo_(jogo), partida_(criarMapa(mapa), valores,
                            dificuldadeNoMapa(valores, mapa, dificuldade),
                            valores.ondas[mapa - 1]),
      cursor_{partida_.mapa().largura() / 2, partida_.mapa().altura() / 2} {}

void EstadoPartida::moverCursor(int dx, int dy) {
  const Mapa& mapa = partida_.mapa();
  cursor_.x = std::clamp(cursor_.x + dx, 0, mapa.largura() - 1);
  cursor_.y = std::clamp(cursor_.y + dy, 0, mapa.altura() - 1);
}

void EstadoPartida::tratarEntrada(int tecla) {
  switch (tecla) {
  case KEY_UP: moverCursor(0, -1); break;
  case KEY_DOWN: moverCursor(0, 1); break;
  case KEY_LEFT: moverCursor(-1, 0); break;
  case KEY_RIGHT: moverCursor(1, 0); break;
  case KEY_SR: moverCursor(0, -5); break;  // shift + setas
  case KEY_SF: moverCursor(0, 5); break;
  case KEY_SLEFT: moverCursor(-5, 0); break;
  case KEY_SRIGHT: moverCursor(5, 0); break;
  case '1': case '2': case '3': case '4': case '5': escolha_ = tecla - '1'; break;
  case ' ': case '\n': case KEY_ENTER: construir(); break;
  case 'n': partida_.chamarOnda(); break;
  case 'p': pausado_ = !pausado_; break;
  case 'q': jogo_.mudarEstado(std::make_unique<EstadoMenu>(jogo_)); break;
  }
}

void EstadoPartida::avisar(const std::string& mensagem) {
  mensagem_ = mensagem;
  mensagemTicks_ = 2 * TICKS_POR_SEGUNDO;
}

void EstadoPartida::construir() {
  if (pausado_) return;
  if (escolha_ == OBSTACULO) {
    if (!partida_.livreParaObstaculo(cursor_))
      avisar("Obstáculos vão em uma casa livre do caminho (#).");
    else if (!partida_.construirObstaculo(cursor_))
      avisar("Ouro insuficiente.");
    return;
  }
  if (!partida_.livreParaTorre(cursor_))
    avisar("Torres vão em uma casa livre do chão (.).");
  else if (!partida_.construirTorre(cursor_, static_cast<TipoTorre>(escolha_)))
    avisar("Ouro insuficiente.");
}

void EstadoPartida::atualizar() {
  if (mensagemTicks_ > 0) --mensagemTicks_;
  if (pausado_) return;
  partida_.tick();
  if (partida_.terminou())
    jogo_.mudarEstado(std::make_unique<EstadoFim>(
        jogo_, partida_.venceu(), std::max(0, partida_.jogador().vida()),
        partida_.ondas().ondaAtual(), partida_.ondas().totalOndas()));
}

void EstadoPartida::desenharMapa() const {
  const Mapa& mapa = partida_.mapa();

  // O alcance mostrado: o da torre sob o cursor ou, se dá para construir a
  // torre escolhida ali, o que ela teria.
  double alcance = -1;
  if (const Torre* torre = partida_.torreEm(cursor_))
    alcance = torre->alcance();
  else if (escolha_ != OBSTACULO && partida_.livreParaTorre(cursor_))
    alcance = partida_.valores(static_cast<TipoTorre>(escolha_)).alcance;

  for (int y = 0; y < mapa.altura(); ++y)
    for (int x = 0; x < mapa.largura(); ++x) {
      char c = mapa.em({x, y});
      bool noAlcance = dentroDoAlcance(cursor_, alcance, {x, y});
      if (c == '@')
        escrever(y, x, COR_BASE, c, A_BOLD);
      else if (noAlcance)
        escrever(y, x, COR_ALCANCE, c, A_BOLD);
      else if (c == '#')
        escrever(y, x, COR_CAMINHO, c);
      else
        escrever(y, x, 0, c, A_DIM);
    }

  // O rastro vai da torre até o alvo, sem as duas pontas.
  for (const Disparo& d : partida_.disparos()) {
    int dx = d.para.x - d.de.x, dy = d.para.y - d.de.y;
    int passos = std::max(std::abs(dx), std::abs(dy));
    int cor = d.tipo == TipoTorre::Lenta ? COR_LENTO : 0;
    for (int i = 1; i < passos; ++i)
      escrever(d.de.y + dy * i / passos, d.de.x + dx * i / passos, cor,
               traco(dx, dy), A_BOLD);
  }

  for (const Torre& torre : partida_.torres())
    escrever(torre.posicao().y, torre.posicao().x, COR_TORRE,
             simbolo(torre.tipo()), A_BOLD);

  for (const Inimigo& inimigo : partida_.inimigos()) {
    Posicao p = inimigo.posicao();
    if (partida_.obstaculoEm(p)) continue;
    bool atingido = std::any_of(
        partida_.disparos().begin(), partida_.disparos().end(),
        [&](const Disparo& d) { return d.para == p && d.ticks == Partida::DURACAO_DISPARO; });
    escrever(p.y, p.x, inimigo.lento() ? COR_LENTO : COR_INIMIGO,
             simbolo(inimigo.tipo()), A_BOLD | (atingido ? A_REVERSE : A_NORMAL));
  }

  for (int y = 0; y < mapa.altura(); ++y)
    for (int x = 0; x < mapa.largura(); ++x)
      if (partida_.obstaculoEm({x, y})) escrever(y, x, COR_OBSTACULO, '=', A_BOLD);

  mvchgat(cursor_.y, cursor_.x, 1, A_REVERSE, 0, nullptr);
}

void EstadoPartida::desenharPainel() const {
  const Jogador& jogador = partida_.jogador();
  const GerenciadorOndas& ondas = partida_.ondas();
  const int x = X_PAINEL;

  attron(A_BOLD);
  mvprintw(0, x, "HP   %d", std::max(0, jogador.vida()));
  mvprintw(1, x, "Ouro %d", jogador.ouro());
  mvprintw(2, x, "Onda %d/%d", ondas.ondaAtual(), ondas.totalOndas());
  attroff(A_BOLD);
  if (ondas.entreOndas()) {
    mvprintw(3, x, "Começa em %ds",
             (ondas.ticksParaProxima() + TICKS_POR_SEGUNDO - 1) / TICKS_POR_SEGUNDO);
    mvaddstr(4, x, "n: chamar agora");
  }
  if (pausado_) escrever(5, x, COR_TITULO, "PAUSADO", A_BOLD);

  mvaddstr(7, x, "Construir:");
  for (int i = 0; i <= OBSTACULO; ++i) {
    bool torre = i != OBSTACULO;
    int custo = torre ? partida_.valores(static_cast<TipoTorre>(i)).custo
                      : partida_.custoObstaculo();
    attr_t estilo = (i == escolha_ ? A_REVERSE : A_NORMAL) |
                    (custo > jogador.ouro() ? A_DIM : A_NORMAL);
    attron(estilo);
    mvprintw(8 + i, x, "%d %c %s", i + 1,
             torre ? simbolo(static_cast<TipoTorre>(i)) : '=',
             torre ? nome(static_cast<TipoTorre>(i)) : "Obstáculo");
    mvprintw(8 + i, x + 15, "%4d", custo);
    attroff(estilo);
  }

  // O que está sob o cursor.
  const Mapa& mapa = partida_.mapa();
  if (const Torre* torre = partida_.torreEm(cursor_)) {
    const ValoresTorre& v = partida_.valores(torre->tipo());
    mvprintw(14, x, "Torre %s", nome(torre->tipo()));
    mvprintw(15, x, "alc %.0f dano %d", v.alcance, v.dano);
    mvprintw(16, x, "recarga %.1fs", double(v.recarga) / TICKS_POR_SEGUNDO);
  } else if (const Obstaculo* obstaculo = partida_.obstaculoEm(cursor_)) {
    mvprintw(14, x, "Obstáculo");
    mvprintw(15, x, "vida %d", obstaculo->vida());
  } else if (mapa.em(cursor_) == '#') {
    mvaddstr(14, x, "Caminho");
  } else if (mapa.em(cursor_) == '@') {
    mvaddstr(14, x, "Base");
  } else {
    mvaddstr(14, x, "Chão");
  }

  mvaddstr(18, x, "Setas  cursor");
  mvaddstr(19, x, "1-5    escolher");
  mvaddstr(20, x, "Espaço construir");
  mvaddstr(21, x, "p pausa  q menu");
}

void EstadoPartida::desenhar() {
  desenharMapa();
  desenharPainel();
  if (mensagemTicks_ > 0) escrever(22, 0, COR_INIMIGO, mensagem_.c_str(), A_BOLD);
}

// --- Fim ---

EstadoFim::EstadoFim(Jogo& jogo, bool venceu, int vida, int onda, int totalOndas)
    : jogo_(jogo), venceu_(venceu), vida_(vida), onda_(onda), totalOndas_(totalOndas) {}

void EstadoFim::tratarEntrada(int tecla) {
  if (confirmar(tecla))
    jogo_.mudarEstado(std::make_unique<EstadoMenu>(jogo_));
  else if (tecla == 'q')
    jogo_.sair();
}

void EstadoFim::desenhar() {
  escrever(6, 35, venceu_ ? COR_TORRE : COR_INIMIGO, venceu_ ? "VITÓRIA!" : "DERROTA", A_BOLD);
  if (venceu_)
    mvprintw(9, 20, "Você sobreviveu às %d ondas com %d de HP.", totalOndas_, vida_);
  else
    mvprintw(9, 26, "A base caiu na onda %d de %d.", onda_, totalOndas_);
  mvaddstr(13, 24, "Enter: voltar ao menu    q: sair");
}
