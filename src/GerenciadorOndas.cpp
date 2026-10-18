#include "GerenciadorOndas.h"

#include <algorithm>

GerenciadorOndas::GerenciadorOndas(std::vector<Onda> ondas, int inicio,
                                   int intervalo, int espacamento)
    : ondas_(std::move(ondas)), intervalo_(intervalo),
      espacamento_(espacamento), espera_(inicio) {}

void GerenciadorOndas::iniciarOnda() {
  entreOndas_ = false;
  filas_.clear();
  for (const Grupo& grupo : ondas_[onda_]) {
    if (grupo.entrada >= static_cast<int>(filas_.size()))
      filas_.resize(grupo.entrada + 1);
    filas_[grupo.entrada].grupos.push_back(grupo);
  }
}

std::vector<Grupo> GerenciadorOndas::tick() {
  if (terminou()) return {};
  if (entreOndas_) {
    if (espera_ > 0) {
      --espera_;
      return {};
    }
    iniciarOnda();
  }

  std::vector<Grupo> criados;
  for (Fila& fila : filas_) {
    if (fila.grupo == static_cast<int>(fila.grupos.size())) continue;
    if (fila.espera > 0) {
      --fila.espera;
      continue;
    }
    const Grupo& grupo = fila.grupos[fila.grupo];
    criados.push_back(grupo);
    fila.espera = espacamento_ - 1;  // um inimigo a cada `espacamento` ticks
    if (++fila.criados == grupo.quantidade) {
      fila.criados = 0;
      ++fila.grupo;
    }
  }

  bool acabou = std::all_of(filas_.begin(), filas_.end(), [](const Fila& f) {
    return f.grupo == static_cast<int>(f.grupos.size());
  });
  if (acabou) {
    ++onda_;
    entreOndas_ = true;
    espera_ = intervalo_;
  }
  return criados;
}

void GerenciadorOndas::chamarProxima() {
  if (entreOndas_) espera_ = 0;
}

bool GerenciadorOndas::terminou() const { return onda_ == totalOndas(); }

bool GerenciadorOndas::entreOndas() const { return entreOndas_ && !terminou(); }

int GerenciadorOndas::ticksParaProxima() const { return espera_; }

int GerenciadorOndas::ondasCompletas() const { return onda_; }

int GerenciadorOndas::ondaAtual() const {
  return std::min(onda_ + 1, totalOndas());
}

int GerenciadorOndas::totalOndas() const {
  return static_cast<int>(ondas_.size());
}
