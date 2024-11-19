# RiverXGH

RiverXGH é um jogo inspirado no clássico River Raid, desenvolvido utilizando a biblioteca gráfica raylib. Neste jogo, você controla um avião que precisa sobreviver ao longo de um rio cheio de obstáculos e inimigos, enquanto gerencia recursos como combustível.

## 📜 Índice
- [Descrição](#descricao)
- [Funcionalidades](#funcionalidades)
- [Como Jogar](#como-jogar)
- [Requisitos](#requisitos)
- [Como Executar](#como-executar)

## 🛶 Descrição
RiverXGH combina nostalgia e modernidade, trazendo o gameplay clássico de River Raid com gráficos e mecânicas aprimoradas. O objetivo é atravessar o rio enquanto desvia de inimigos e coleta combustível para continuar sua jornada.

## ✨ Funcionalidades
- Controles simples e intuitivos.
- Gerenciamento de combustível: O avião consome combustível constantemente, exigindo que o jogador colete reabastecimentos no caminho.
- Pontuação dinâmica: Pontos são acumulados com base na distância percorrida e nos inimigos destruídos.

## 🎮 Como Jogar
- Mover o avião: Use A e D para mover para a esquerda ou direita.
- Atirar: Pressione a barra de espaço para disparar tiros e destruir obstáculos.
- Combustível: Sobrevoe os ícones de combustível para reabastecer ou destrua-os para pontuar.
- Evitar colisões: Colida com inimigos ou fique sem combustível para perder vidas.

## 🖥️ Requisitos
- Compilador C/C++ (compatível com C99 ou superior).
- Biblioteca raylib (versão 4.0 ou superior).
- Sistemas Operacionais (Testados): Windows, Linux e macOS.
    
## 🚀 Como Executar
### 1. Clone o repositório
  ```
  git clone https://github.com/rafael-zzz/RiverXGH
  cd RiverXGH
  ```
### 2. Compile e execute o jogo
  ```
  gcc -o RiverXGH main.c -lraylib -lm -lpthread -ldl -lrt -lX11 && ./RiverXGH
  ```
