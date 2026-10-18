cmake_minimum_required(VERSION 3.20)
project(TowerDefense CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # ajuda o editor (clangd) a entender o projeto
add_compile_options(-Wall -Wextra)     # vale para todos os alvos abaixo

set(CURSES_NEED_WIDE TRUE)  # ncursesw: acentos em UTF-8
find_package(Curses REQUIRED)

# Lógica do jogo, sem ncurses. Usada pelo jogo e pelos testes.
# Cada classe nova acrescenta seu .cpp nesta lista.
add_library(logica STATIC
    src/Jogador.cpp
    src/Mapa.cpp
    src/Mapas.cpp
    src/Inimigo.cpp
    src/Obstaculo.cpp
    src/Ataques.cpp
    src/Torre.cpp
    src/FabricaInimigos.cpp
    src/GerenciadorOndas.cpp
    src/Valores.cpp
    src/Partida.cpp
)
target_include_directories(logica PUBLIC src)

# O jogo: lógica + ncurses. Lê valores.txt da pasta do projeto, ou o arquivo
# passado como argumento (./build/td outro.txt).
add_executable(td src/main.cpp src/Jogo.cpp src/Estados.cpp)
target_include_directories(td PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(td PRIVATE logica ${CURSES_LIBRARIES})
target_compile_definitions(td PRIVATE VALORES_TXT="${CMAKE_SOURCE_DIR}/valores.txt")

# Os testes: lógica, sem ncurses. Rodar com: ctest --test-dir build
enable_testing()
add_executable(testes tests/testes.cpp)
target_link_libraries(testes PRIVATE logica)
target_compile_definitions(testes PRIVATE VALORES_TXT="${CMAKE_SOURCE_DIR}/valores.txt")
add_test(NAME testes COMMAND testes)
