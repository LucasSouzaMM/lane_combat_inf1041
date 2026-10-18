#undef NDEBUG // builds Release definem NDEBUG, que desliga o assert; aqui ele
              // fica sempre ligado
#include <cassert>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "Ataques.h"
#include "FabricaInimigos.h"
#include "GerenciadorOndas.h"
#include "Inimigo.h"
#include "Jogador.h"
#include "Mapa.h"
#include "Mapas.h"
#include "Obstaculo.h"
#include "Partida.h"
#include "Torre.h"
#include "Valores.h"

// Base em (0, 1), entrada em (5, 1). Distância da entrada até a base: 5.
Mapa mapaReto() { return Mapa({"......", "@#####", "......"}); }

Inimigo inimigo(int vida, double velocidade, Posicao posicao, int dano = 1) {
  return Inimigo(TipoInimigo::Normal, {vida, velocidade, dano, 0}, posicao, 10);
}

Valores valoresDoArquivo() { return lerValores(VALORES_TXT); }

const Dificuldade& normal() {
  static Valores valores = valoresDoArquivo();
  return valores.dificuldades[1];
}

// --- Jogador ---

void teste_gastar_com_ouro_suficiente() {
  Jogador jogador(20, 100);
  assert(jogador.gastar(30));
  assert(jogador.ouro() == 70);
}

void teste_gastar_sem_ouro_suficiente_nao_muda_o_ouro() {
  Jogador jogador(20, 20);
  assert(!jogador.gastar(30));
  assert(jogador.ouro() == 20);
}

void teste_jogador_sem_vida_morre() {
  Jogador jogador(3, 0);
  jogador.receberDano(3);
  assert(!jogador.vivo());
}

// --- Mapa ---

void teste_mapa_acha_base_e_entrada() {
  Mapa mapa = mapaReto();
  assert(mapa.base() == (Posicao{0, 1}));
  assert(mapa.entradas().size() == 1);
  assert(mapa.entradas()[0] == (Posicao{5, 1}));
  assert(mapa.distancia({5, 1}) == 5);
}

void teste_caminho_leva_da_entrada_ate_a_base() {
  Mapa mapa = mapaReto();
  Posicao p{5, 1};
  for (int i = 0; i < 5; ++i) p = mapa.proximo(p);
  assert(p == mapa.base());
}

void teste_mapas_do_jogo_tem_n_entradas_que_chegam_na_base() {
  for (int numero = 1; numero <= 3; ++numero) {
    Mapa mapa = criarMapa(numero);
    assert(static_cast<int>(mapa.entradas().size()) == numero);
    for (Posicao p : mapa.entradas()) {
      int passos = mapa.distancia(p);
      for (int i = 0; i < passos; ++i) p = mapa.proximo(p);
      assert(p == mapa.base());
    }
  }
}

void teste_alcance_e_um_circulo() {
  assert(dentroDoAlcance({0, 0}, 4, {4, 0}));
  assert(dentroDoAlcance({0, 0}, 4, {2, 3}));   // 4 + 9 <= 16
  assert(!dentroDoAlcance({0, 0}, 4, {3, 3}));  // 18 > 16
}

// --- Inimigo ---

void teste_inimigo_novo_esta_vivo() { assert(inimigo(10, 1, {5, 1}).vivo()); }

void teste_dano_igual_a_vida_mata() {
  Inimigo i = inimigo(10, 1, {5, 1});
  i.receberDano(10);
  assert(!i.vivo());
}

void teste_inimigo_com_meia_casa_por_tick_anda_a_cada_2_ticks() {
  Mapa mapa = mapaReto();
  Inimigo i = inimigo(10, 0.5, {5, 1});
  i.tick(mapa, nullptr);
  assert(i.posicao() == (Posicao{5, 1}));
  i.tick(mapa, nullptr);
  assert(i.posicao() == (Posicao{4, 1}));
}

void teste_inimigo_ataca_obstaculo_em_vez_de_andar() {
  Mapa mapa = mapaReto();
  Inimigo i = inimigo(10, 1, {5, 1}, 4);
  Obstaculo obstaculo({4, 1}, 10);
  i.tick(mapa, &obstaculo);
  assert(i.posicao() == (Posicao{5, 1}));
  assert(obstaculo.vida() == 6);
}

void teste_inimigo_espera_o_intervalo_entre_golpes() {
  Mapa mapa = mapaReto();
  Inimigo i = inimigo(10, 1, {5, 1}, 1);  // intervalo de 10 ticks
  Obstaculo obstaculo({4, 1}, 100);
  for (int t = 0; t < 11; ++t) i.tick(mapa, &obstaculo);
  assert(obstaculo.vida() == 98);  // golpes nos ticks 1 e 11
}

void teste_inimigo_lento_anda_mais_devagar() {
  Mapa mapa = mapaReto();
  Inimigo i = inimigo(10, 0.5, {5, 1});
  i.desacelerar(0.5, 100);
  for (int t = 0; t < 3; ++t) i.tick(mapa, nullptr);
  assert(i.posicao() == (Posicao{5, 1}));
  i.tick(mapa, nullptr);
  assert(i.posicao() == (Posicao{4, 1}));
}

void teste_lentidao_acaba_depois_da_duracao() {
  Mapa mapa = mapaReto();
  Inimigo i = inimigo(10, 0.1, {5, 1});
  i.desacelerar(0.5, 2);
  i.tick(mapa, nullptr);
  i.tick(mapa, nullptr);
  assert(!i.lento());
}

// --- Estratégias de ataque ---

void teste_ataque_simples_acerta_o_mais_perto_da_base() {
  Mapa mapa = mapaReto();
  std::vector<Inimigo> inimigos{inimigo(10, 0, {4, 1}), inimigo(10, 0, {2, 1}),
                                inimigo(10, 0, {3, 1})};
  AtaqueSimples ataque(3);
  std::vector<Posicao> atingidos = ataque.atacar({3, 0}, 2, inimigos, mapa);
  assert(atingidos.size() == 1 && atingidos[0] == (Posicao{2, 1}));
  assert(inimigos[1].vida() == 7);
  assert(inimigos[0].vida() == 10 && inimigos[2].vida() == 10);
}

void teste_ataque_sem_alvo_no_alcance_nao_acerta_nada() {
  Mapa mapa = mapaReto();
  std::vector<Inimigo> inimigos{inimigo(10, 0, {5, 1})};
  AtaqueSimples ataque(3);
  assert(ataque.atacar({0, 0}, 2, inimigos, mapa).empty());
  assert(inimigos[0].vida() == 10);
}

void teste_ataque_area_acerta_todos_no_alcance() {
  Mapa mapa = mapaReto();
  std::vector<Inimigo> inimigos{inimigo(10, 0, {1, 1}), inimigo(10, 0, {2, 1}),
                                inimigo(10, 0, {5, 1})};
  AtaqueArea ataque(2);
  assert(ataque.atacar({2, 0}, 1.5, inimigos, mapa).size() == 2);
  assert(inimigos[0].vida() == 8 && inimigos[1].vida() == 8);
  assert(inimigos[2].vida() == 10);
}

void teste_ataque_lento_desacelera_o_alvo() {
  Mapa mapa = mapaReto();
  std::vector<Inimigo> inimigos{inimigo(10, 0, {2, 1})};
  AtaqueLento ataque(1, 0.5, 10);
  ataque.atacar({2, 0}, 1, inimigos, mapa);
  assert(inimigos[0].lento());
}

// --- Torre ---

void teste_torre_espera_a_recarga_entre_ataques() {
  Mapa mapa = mapaReto();
  std::vector<Inimigo> inimigos{inimigo(100, 0, {2, 1})};
  Torre torre(TipoTorre::Simples, {2, 0}, 1, 2,
              std::make_unique<AtaqueSimples>(1));
  for (int i = 0; i < 4; ++i) torre.tick(inimigos, mapa);
  assert(inimigos[0].vida() == 98);  // ataca nos ticks 1 e 3
}

void teste_torre_sem_alvo_nao_entra_em_recarga() {
  Mapa mapa = mapaReto();
  std::vector<Inimigo> inimigos;
  Torre torre(TipoTorre::Simples, {2, 0}, 1, 10,
              std::make_unique<AtaqueSimples>(1));
  torre.tick(inimigos, mapa);
  inimigos.push_back(inimigo(100, 0, {2, 1}));
  torre.tick(inimigos, mapa);
  assert(inimigos[0].vida() == 99);
}

// --- GerenciadorOndas ---

int contarCriados(GerenciadorOndas& ondas, int ticks) {
  int criados = 0;
  for (int i = 0; i < ticks; ++i) criados += ondas.tick().size();
  return criados;
}

void teste_grupo_de_3_cria_3_inimigos_um_por_vez() {
  GerenciadorOndas ondas({{{TipoInimigo::Normal, 3, 0}}}, 0, 0, 2);
  assert(contarCriados(ondas, 1) == 1);
  assert(contarCriados(ondas, 1) == 0);  // espaçamento
  assert(contarCriados(ondas, 100) == 2);
  assert(ondas.terminou());
}

void teste_entradas_diferentes_criam_ao_mesmo_tempo() {
  GerenciadorOndas ondas(
      {{{TipoInimigo::Normal, 1, 0}, {TipoInimigo::Rapido, 1, 1}}}, 0, 0, 5);
  std::vector<Grupo> criados = ondas.tick();
  assert(criados.size() == 2);
  assert(criados[1].tipo == TipoInimigo::Rapido && criados[1].entrada == 1);
}

void teste_grupos_da_mesma_entrada_entram_em_sequencia() {
  GerenciadorOndas ondas(
      {{{TipoInimigo::Normal, 1, 0}, {TipoInimigo::Rapido, 1, 0}}}, 0, 0, 1);
  assert(ondas.tick()[0].tipo == TipoInimigo::Normal);
  assert(ondas.tick()[0].tipo == TipoInimigo::Rapido);
}

void teste_proxima_onda_espera_o_intervalo() {
  GerenciadorOndas ondas(
      {{{TipoInimigo::Normal, 1, 0}}, {{TipoInimigo::Normal, 1, 0}}}, 0, 5, 0);
  assert(contarCriados(ondas, 1) == 1);
  assert(ondas.entreOndas() && ondas.ondasCompletas() == 1);
  assert(contarCriados(ondas, 5) == 0);
  assert(contarCriados(ondas, 1) == 1);
}

void teste_chamar_proxima_onda_pula_a_espera() {
  GerenciadorOndas ondas({{{TipoInimigo::Normal, 1, 0}}}, 50, 0, 0);
  ondas.chamarProxima();
  assert(contarCriados(ondas, 1) == 1);
}

// --- Valores ---

bool falhaAoLer(const std::string& texto) {
  std::istringstream entrada(texto);
  try {
    lerValores(entrada);
  } catch (const std::runtime_error&) {
    return true;
  }
  return false;
}

void teste_arquivo_de_valores_do_jogo_e_valido() {
  Valores valores = valoresDoArquivo();
  assert(valores.ondas[0].size() == 8);
  assert(valores.espacamento == 8);  // 0,8 s
  assert(valores.ouroMapa[1] == 100);
}

void teste_chave_faltando_ou_desconhecida_e_erro() {
  assert(falhaAoLer("jogador.vida 20\n"));
  std::ifstream arquivo(VALORES_TXT);
  std::stringstream texto;
  texto << arquivo.rdbuf();
  assert(!falhaAoLer(texto.str()));
  assert(falhaAoLer(texto.str() + "torre.simples.dano 3\n"));  // repetida
  assert(falhaAoLer(texto.str() + "torre.simpels.dano 3\n"));  // desconhecida
  assert(falhaAoLer(texto.str() + "mapa1.onda N8@2\n"));       // entrada 2
  assert(falhaAoLer(texto.str() + "mapa1.onda X3\n"));         // tipo X
}

// --- FabricaInimigos ---

void teste_dificuldade_muda_vida_e_velocidade() {
  Valores valores = valoresDoArquivo();
  Inimigo facil = FabricaInimigos(valores, valores.dificuldades[0])
                      .criar(TipoInimigo::Tanque, {0, 0});
  Inimigo dificil = FabricaInimigos(valores, valores.dificuldades[2])
                        .criar(TipoInimigo::Tanque, {0, 0});
  assert(dificil.vida() > facil.vida());
}

// --- Partida ---

Partida partidaReta(std::vector<Onda> ondas = {}) {
  return Partida(mapaReto(), valoresDoArquivo(), normal(), std::move(ondas));
}

void teste_torre_so_no_chao_e_obstaculo_so_no_caminho() {
  Partida partida = partidaReta();
  assert(!partida.construirTorre({2, 1}, TipoTorre::Simples));
  assert(!partida.construirObstaculo({2, 0}));
  assert(!partida.construirObstaculo({5, 1}));  // entrada
  assert(partida.construirTorre({2, 0}, TipoTorre::Simples));
  assert(partida.construirObstaculo({2, 1}));
}

void teste_construir_torre_gasta_o_custo() {
  Partida partida = partidaReta();
  partida.construirTorre({2, 0}, TipoTorre::Simples);
  assert(partida.jogador().ouro() ==
         normal().ouroInicial - partida.valores(TipoTorre::Simples).custo);
}

void teste_construir_em_casa_ocupada_nao_gasta_ouro() {
  Partida partida = partidaReta();
  partida.construirTorre({2, 0}, TipoTorre::Simples);
  int ouro = partida.jogador().ouro();
  assert(!partida.construirTorre({2, 0}, TipoTorre::Simples));
  assert(partida.jogador().ouro() == ouro);
}

void teste_construir_sem_ouro_falha() {
  Partida partida = partidaReta();
  int construidas = 0;
  for (int x = 0; x < 6; ++x) construidas += partida.construirTorre({x, 0}, TipoTorre::Area);
  assert(construidas == normal().ouroInicial / partida.valores(TipoTorre::Area).custo);
}

void teste_inimigo_que_chega_tira_vida_e_sai() {
  Partida partida = partidaReta({{{TipoInimigo::Normal, 1, 0}}});
  partida.chamarOnda();
  for (int t = 0; t < 1000 && !partida.terminou(); ++t) partida.tick();
  assert(partida.inimigos().empty());
  assert(partida.jogador().vida() == valoresDoArquivo().vidaJogador - 1);
}

void teste_inimigo_morto_da_ouro_e_onda_completa_da_bonus() {
  Valores valores = valoresDoArquivo();
  Partida partida = partidaReta({{{TipoInimigo::Normal, 1, 0}}});
  partida.construirTorre({4, 0}, TipoTorre::Longa);  // 15 de dano mata o normal
  int ouro = partida.jogador().ouro();
  partida.chamarOnda();
  partida.tick();
  partida.tick();
  assert(partida.inimigos().empty());
  assert(partida.jogador().ouro() == ouro + valores.bonusOnda + valores.inimigos[0].ouro);
  assert(!partida.disparos().empty());
}

void teste_sem_ondas_e_sem_inimigos_e_vitoria() {
  Partida partida = partidaReta();
  assert(partida.venceu() && partida.terminou());
}

void teste_sem_defesa_o_jogador_perde_no_mapa_1() {
  Valores valores = valoresDoArquivo();
  Partida partida(criarMapa(1), valores, normal(), valores.ondas[0]);
  for (int i = 0; i < 100000 && !partida.terminou(); ++i) partida.tick();
  assert(partida.terminou() && !partida.venceu());
}

int main() {
  teste_gastar_com_ouro_suficiente();
  teste_gastar_sem_ouro_suficiente_nao_muda_o_ouro();
  teste_jogador_sem_vida_morre();
  teste_mapa_acha_base_e_entrada();
  teste_caminho_leva_da_entrada_ate_a_base();
  teste_mapas_do_jogo_tem_n_entradas_que_chegam_na_base();
  teste_alcance_e_um_circulo();
  teste_inimigo_novo_esta_vivo();
  teste_dano_igual_a_vida_mata();
  teste_inimigo_com_meia_casa_por_tick_anda_a_cada_2_ticks();
  teste_inimigo_ataca_obstaculo_em_vez_de_andar();
  teste_inimigo_espera_o_intervalo_entre_golpes();
  teste_inimigo_lento_anda_mais_devagar();
  teste_lentidao_acaba_depois_da_duracao();
  teste_ataque_simples_acerta_o_mais_perto_da_base();
  teste_ataque_sem_alvo_no_alcance_nao_acerta_nada();
  teste_ataque_area_acerta_todos_no_alcance();
  teste_ataque_lento_desacelera_o_alvo();
  teste_torre_espera_a_recarga_entre_ataques();
  teste_torre_sem_alvo_nao_entra_em_recarga();
  teste_grupo_de_3_cria_3_inimigos_um_por_vez();
  teste_entradas_diferentes_criam_ao_mesmo_tempo();
  teste_grupos_da_mesma_entrada_entram_em_sequencia();
  teste_proxima_onda_espera_o_intervalo();
  teste_chamar_proxima_onda_pula_a_espera();
  teste_arquivo_de_valores_do_jogo_e_valido();
  teste_chave_faltando_ou_desconhecida_e_erro();
  teste_dificuldade_muda_vida_e_velocidade();
  teste_torre_so_no_chao_e_obstaculo_so_no_caminho();
  teste_construir_torre_gasta_o_custo();
  teste_construir_em_casa_ocupada_nao_gasta_ouro();
  teste_construir_sem_ouro_falha();
  teste_inimigo_que_chega_tira_vida_e_sai();
  teste_inimigo_morto_da_ouro_e_onda_completa_da_bonus();
  teste_sem_ondas_e_sem_inimigos_e_vitoria();
  teste_sem_defesa_o_jogador_perde_no_mapa_1();
  std::puts("todos os testes passaram");
}
