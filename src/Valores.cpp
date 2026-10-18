#include "Valores.h"

#include <cmath>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace {

std::runtime_error erro(int linha, const std::string& mensagem) {
  return std::runtime_error("linha " + std::to_string(linha) + ": " + mensagem);
}

int ticks(double segundos) {
  return static_cast<int>(std::lround(segundos * TICKS_POR_SEGUNDO));
}

// "N8", "R5@2": tipo, quantidade e entrada (a partir de 1; padrão 1).
Grupo lerGrupo(const std::string& texto, int numEntradas, int linha) {
  static const std::regex formato("([NRT])([0-9]+)(?:@([0-9]+))?");
  std::smatch partes;
  if (!std::regex_match(texto, partes, formato))
    throw erro(linha, "grupo inválido \"" + texto + "\" (exemplos: N8, R5@2)");
  int entrada = partes[3].matched ? std::stoi(partes[3]) : 1;
  if (entrada < 1 || entrada > numEntradas)
    throw erro(linha, "o mapa " + std::to_string(numEntradas) +
                          " não tem a entrada " + std::to_string(entrada));
  const std::string tipos = "NRT";
  return {static_cast<TipoInimigo>(tipos.find(partes[1].str()[0])),
          std::stoi(partes[2]), entrada - 1};
}

}  // namespace

Valores lerValores(std::istream& entrada) {
  std::map<std::string, std::pair<double, int>> numeros;  // valor, linha
  Valores valores{};
  std::string texto;
  for (int linha = 1; std::getline(entrada, texto); ++linha) {
    std::istringstream campos(texto.substr(0, texto.find('#')));
    std::string chave;
    if (!(campos >> chave)) continue;

    bool ehOnda = false;
    for (int mapa = 1; mapa <= 3; ++mapa)
      if (chave == "mapa" + std::to_string(mapa) + ".onda") {
        Onda onda;
        for (std::string grupo; campos >> grupo;)
          onda.push_back(lerGrupo(grupo, mapa, linha));
        if (onda.empty()) throw erro(linha, "onda sem grupos");
        valores.ondas[mapa - 1].push_back(onda);
        ehOnda = true;
      }
    if (ehOnda) continue;

    double valor;
    std::string resto;
    if (!(campos >> valor) || campos >> resto)
      throw erro(linha, "esperava um número depois de " + chave);
    if (valor < 0) throw erro(linha, chave + " não pode ser negativo");
    if (!numeros.emplace(chave, std::make_pair(valor, linha)).second)
      throw erro(linha, "chave repetida: " + chave);
  }

  auto pegar = [&](const std::string& chave) {
    auto it = numeros.find(chave);
    if (it == numeros.end())
      throw std::runtime_error("falta a chave " + chave);
    double valor = it->second.first;
    numeros.erase(it);
    return valor;
  };

  valores.vidaJogador = pegar("jogador.vida");
  valores.bonusOnda = pegar("ondas.bonus");
  valores.inicioOndas = ticks(pegar("ondas.inicio"));
  valores.intervaloOndas = ticks(pegar("ondas.intervalo"));
  valores.espacamento = ticks(pegar("ondas.espacamento"));

  const char* inimigos[] = {"normal", "rapido", "tanque"};
  for (int i = 0; i < 3; ++i) {
    std::string prefixo = std::string("inimigo.") + inimigos[i] + ".";
    valores.inimigos[i] = {static_cast<int>(pegar(prefixo + "vida")),
                           pegar(prefixo + "velocidade") / TICKS_POR_SEGUNDO,
                           static_cast<int>(pegar(prefixo + "dano")),
                           static_cast<int>(pegar(prefixo + "ouro"))};
  }

  const char* torres[] = {"simples", "area", "lenta", "longa"};
  for (int i = 0; i < 4; ++i) {
    std::string prefixo = std::string("torre.") + torres[i] + ".";
    valores.torres[i] = {static_cast<int>(pegar(prefixo + "custo")),
                         pegar(prefixo + "alcance"),
                         ticks(pegar(prefixo + "recarga")),
                         static_cast<int>(pegar(prefixo + "dano"))};
  }
  valores.lentidao = pegar("torre.lenta.lentidao") / 100;
  valores.duracaoLentidao = ticks(pegar("torre.lenta.duracao"));

  valores.custoObstaculo = pegar("obstaculo.custo");
  valores.vidaObstaculo = pegar("obstaculo.vida");
  valores.intervaloAtaque = ticks(pegar("obstaculo.intervaloAtaque"));

  const char* chaves[] = {"facil", "normal", "dificil"};
  const char* nomes[] = {"Fácil", "Normal", "Difícil"};
  for (int i = 0; i < 3; ++i) {
    std::string prefixo = std::string("dificuldade.") + chaves[i] + ".";
    valores.dificuldades[i] = {nomes[i],
                               static_cast<int>(pegar(prefixo + "vida")),
                               static_cast<int>(pegar(prefixo + "velocidade")),
                               static_cast<int>(pegar(prefixo + "ouro"))};
  }

  for (int mapa = 1; mapa <= 3; ++mapa)
    valores.ouroMapa[mapa - 1] = pegar("mapa" + std::to_string(mapa) + ".ouro");

  if (!numeros.empty())
    throw erro(numeros.begin()->second.second,
               "chave desconhecida: " + numeros.begin()->first);
  for (int mapa = 1; mapa <= 3; ++mapa)
    if (valores.ondas[mapa - 1].empty())
      throw std::runtime_error("o mapa " + std::to_string(mapa) +
                               " não tem ondas (mapa" + std::to_string(mapa) +
                               ".onda)");
  return valores;
}

Valores lerValores(const std::string& caminho) {
  std::ifstream arquivo(caminho);
  if (!arquivo) throw std::runtime_error("não abriu " + caminho);
  try {
    return lerValores(arquivo);
  } catch (const std::runtime_error& e) {
    throw std::runtime_error(caminho + ", " + e.what());
  }
}
