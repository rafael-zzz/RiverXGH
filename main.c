#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Texture2D jogadorTexture;

// Estrutura para armazenar as informações do jogador
typedef struct Jogador {
    Vector2 posicao;   // Posição do jogador
    float velocidade;  // Velocidade de movimento do jogador
    int vida;          // Vida do jogador
    float combustivel; // Combustível do jogador
    int vidas;         // Número de vidas do jogador
} Jogador;

// Estrutura para armazenar as informações dos inimigos
typedef struct Inimigo {
    Vector2 posicao;   // Posição do inimigo
    struct Inimigo* prox; // Ponteiro para o próximo inimigo na lista
} Inimigo;

// Estrutura para armazenar as informações dos combustíveis
typedef struct Combustivel {
    Vector2 posicao;   // Posição do combustível
    struct Combustivel* prox; // Ponteiro para o próximo combustível na lista
} Combustivel;

// Estrutura para armazenar as informações dos projéteis
typedef struct Projetil {
    Vector2 posicao;   // Posição do projétil
    struct Projetil* prox; // Ponteiro para o próximo projétil na lista
} Projetil;

// Estrutura para armazenar todas as informações do jogo
typedef struct ObjetoJogo {
    Jogador jogador;    // Informações do jogador
    Inimigo* inimigos;  // Lista encadeada de inimigos
    Combustivel* combustiveis; // Lista encadeada de combustíveis
    Projetil* projeteis; // Lista encadeada de projéteis
    int pontuacao;      // Pontuação do jogo
    int gameOver;       // Estado do jogo (game over ou não)
} ObjetoJogo;

// Função para criar um novo inimigo
Inimigo* criarInimigo(float x, float y) {
    Inimigo* novo = (Inimigo*)malloc(sizeof(Inimigo)); // Aloca memória para um novo inimigo
    novo->posicao = (Vector2){x, y}; // Define a posição do inimigo
    novo->prox = NULL; // Inicializa o ponteiro para o próximo inimigo como NULL
    return novo;
}

// Função para inserir um inimigo na lista de inimigos
void inserirInimigo(Inimigo** head, float x, float y) {
    Inimigo* novo = criarInimigo(x, y); // Cria um novo inimigo
    novo->prox = *head; // Faz o novo inimigo apontar para o início da lista
    *head = novo; // Atualiza o início da lista para o novo inimigo
}

// Função para criar um novo projétil
Projetil* criarProjetil(float x, float y) {
    Projetil* novo = (Projetil*)malloc(sizeof(Projetil)); // Aloca memória para um novo projétil
    novo->posicao = (Vector2){x, y}; // Define a posição do projétil
    novo->prox = NULL; // Inicializa o ponteiro para o próximo projétil como NULL
    return novo;
}

// Função para inserir um projétil na lista de projéteis
void inserirProjetil(Projetil** head, float jogadorX, float jogadorY) {
    float jogadorWidth = 67;   // Largura do jogador
    float jogadorHeight = 86;  // Altura do jogador
    float projWidth = 5;       // Largura do projétil

    // Centraliza o projétil na largura do jogador
    float projX = jogadorX + (jogadorWidth / 2) - (projWidth / 2);

    // Ajusta a posição Y para o topo do jogador
    float projY = jogadorY - 10;  // Considerando que o projétil começa logo acima do jogador

    Projetil* novo = criarProjetil(projX, projY);  // Cria um novo projétil na posição ajustada
    novo->prox = *head;
    *head = novo;
}

// Função para criar um novo combustível
Combustivel* criarCombustivel(float x, float y) {
    Combustivel* novo = (Combustivel*)malloc(sizeof(Combustivel)); // Aloca memória para um novo combustível
    novo->posicao = (Vector2){x, y}; // Define a posição do combustível
    novo->prox = NULL; // Inicializa o ponteiro para o próximo combustível como NULL
    return novo;
}

// Função para inserir um combustível na lista de combustíveis
void inserirCombustivel(Combustivel** head, float x, float y) {
    Combustivel* novo = criarCombustivel(x, y); // Cria um novo combustível
    novo->prox = *head; // Faz o novo combustível apontar para o início da lista
    *head = novo; // Atualiza o início da lista para o novo combustível
}

// Função para atualizar a posição dos inimigos
void atualizarInimigos(Inimigo* head, int dificuldade) {
    Inimigo* atual = head;
    while (atual != NULL) {
        // Atualiza a posição do inimigo na direção Y, aumentando a velocidade conforme a dificuldade
        atual->posicao.y += 2.0f + (0.5f * dificuldade);  
        atual = atual->prox;  // Passa para o próximo inimigo na lista
    }
}

// Função para atualizar a posição dos combustíveis
void atualizarCombustiveis(Combustivel* head, int dificuldade) {
    Combustivel* atual = head;
    while (atual != NULL) {
        // Atualiza a posição do combustível na direção Y, aumentando a velocidade conforme a dificuldade
        atual->posicao.y += 2.0f + (0.5f * dificuldade);  
        atual = atual->prox;  // Passa para o próximo combustível na lista
    }
}

// Função para atualizar a posição dos projéteis e removê-los se saírem da tela
void atualizarProjeteis(Projetil** head) {
    Projetil* atual = *head;  // Ponteiro para o projétil atual
    Projetil* anterior = NULL;  // Ponteiro para o projétil anterior na lista

    while (atual != NULL) {
        atual->posicao.y -= 10.0f; // Atualiza a posição do projétil na direção Y

        // Verifica se o projétil saiu da tela
        if (atual->posicao.y < 0) {
            // Remove o projétil da lista
            if (anterior == NULL) {
                *head = atual->prox;  // Atualiza o início da lista se for o primeiro projétil
            } else {
                anterior->prox = atual->prox;  // Faz o projétil anterior apontar para o próximo, pulando o projétil atual
            }
            Projetil* temp = atual;  // Armazena o ponteiro do projétil a ser removido
            atual = atual->prox;  // Move para o próximo projétil na lista
            free(temp);  // Libera a memória do projétil removido
        } else {
            anterior = atual;  // Atualiza o ponteiro do projétil anterior
            atual = atual->prox;  // Move para o próximo projétil na lista
        }
    }
}

// Função para desenhar todos os inimigos na tela
void desenharInimigos(Inimigo* head) {
    Inimigo* atual = head;  // Ponteiro para o inimigo atual
    while (atual != NULL) {
        // Desenha cada inimigo como um retângulo vermelho na posição atual
        DrawRectangleV(atual->posicao, (Vector2){40, 40}, RED);  
        atual = atual->prox;  // Move para o próximo inimigo na lista
    }
}

// Função para desenhar todos os combustíveis na tela
void desenharCombustiveis(Combustivel* head) {
    Combustivel* atual = head;  // Ponteiro para o combustível atual
    while (atual != NULL) {
        // Desenha cada combustível como um retângulo verde na posição atual
        DrawRectangleV(atual->posicao, (Vector2){40, 40}, GREEN);  
        atual = atual->prox;  // Move para o próximo combustível na lista
    }
}

// Função para desenhar todos os projéteis na tela
void desenharProjeteis(Projetil* head) {
    Projetil* atual = head;  // Ponteiro para o projétil atual
    while (atual != NULL) {
        // Desenha cada projétil como um retângulo preto na posição atual
        DrawRectangleV(atual->posicao, (Vector2){5, 10}, BLACK);  
        atual = atual->prox;  // Move para o próximo projétil na lista
    }
}

// Função para liberar a memória de todos os inimigos da lista
void liberarInimigos(Inimigo* head) {
    Inimigo* tmp;  // Ponteiro temporário para armazenar o inimigo a ser removido
    while (head != NULL) {
        tmp = head;  // Armazena o ponteiro do inimigo atual
        head = head->prox;  // Move para o próximo inimigo na lista
        free(tmp);  // Libera a memória do inimigo atual
    }
}

// Função para liberar a memória de todos os combustíveis da lista
void liberarCombustiveis(Combustivel* head) {
    Combustivel* tmp;  // Ponteiro temporário para armazenar o combustível a ser removido
    while (head != NULL) {
        tmp = head;  // Armazena o ponteiro do combustível atual
        head = head->prox; // Move para o próximo combustível na lista
        free(tmp); // Libera a memória de cada combustível
    }
}

// Função para liberar a memória de todos os projéteis da lista
void liberarProjeteis(Projetil* head) {
    Projetil* tmp;  // Ponteiro temporário para armazenar o projétil a ser removido
    while (head != NULL) {
        tmp = head;  // Armazena o ponteiro do projétil atual
        head = head->prox;  // Move para o próximo projétil na lista
        free(tmp);  // Libera a memória do projétil atual
    }
}

// Função para inicializar o jogo com os parâmetros fornecidos
void inicializarJogo(ObjetoJogo* jogo, int vidas, int pontuacao) {
    // Define a posição inicial do jogador
    jogo->jogador.posicao = (Vector2){(600 - 67) / 2, 600 - 86 - 10}; // Centraliza o jogador horizontalmente    
    // Define a velocidade de movimento do jogador
    jogo->jogador.velocidade = 6.0f;
    // Inicializa a vida do jogador (pode ser usada para outras funcionalidades futuras)
    jogo->jogador.vida = 3;
    // Inicializa o combustível do jogador no máximo 
    jogo->jogador.combustivel = 100.0f;
    // Mantém o número de vidas atual
    jogo->jogador.vidas = vidas;
    // Inicializa a lista de inimigos como vazia
    jogo->inimigos = NULL;
    // Inicializa a lista de combustíveis como vazia
    jogo->combustiveis = NULL;
    // Mantém a pontuação acumulada
    jogo->pontuacao = pontuacao;
    // Define o estado do jogo como não "game over"
    jogo->gameOver = 0;
}

// Função para atualizar a posição do jogador com base nas teclas pressionadas
void atualizarJogador(Jogador* jogador) {
    // Move o jogador para a direita se a tecla direcional direita estiver pressionada e o jogador estiver dentro dos limites
    if (IsKeyDown(KEY_RIGHT) && jogador->posicao.x < 600 - 67) jogador->posicao.x += jogador->velocidade;
    // Move o jogador para a esquerda se a tecla direcional esquerda estiver pressionada e o jogador estiver dentro dos limites
    if (IsKeyDown(KEY_LEFT) && jogador->posicao.x > 0) jogador->posicao.x -= jogador->velocidade;
    // Diminui o combustível do jogador conforme ele se move
    jogador->combustivel -= 0.05f;
}

// Função para verificar colisões entre dois objetos com base em suas posições e tamanhos
int checarColisao(Vector2 pos1, Vector2 pos2, Vector2 tam1, Vector2 tam2) {
    // Retorna verdadeiro (1) se houver interseção entre os objetos, falso (0) caso contrário
    return (pos1.x < pos2.x + tam2.x && pos1.x + tam1.x > pos2.x && pos1.y < pos2.y + tam2.y && pos1.y + tam1.y > pos2.y);
}

void checarColisoes(ObjetoJogo* jogo) {
    Inimigo* inimigoAtual = jogo->inimigos;  // Ponteiro para iterar sobre a lista de inimigos
    Inimigo* inimigoAnterior = NULL;  // Ponteiro para manter o inimigo anterior na lista

    // Itera sobre todos os inimigos na lista
    while (inimigoAtual != NULL) {
        // Verifica se há colisão entre o jogador e o inimigo atual
        if (checarColisao(jogo->jogador.posicao, inimigoAtual->posicao, (Vector2){60, 80}, (Vector2){40, 40})) {
            // Remove o inimigo da lista se colidir com o jogador
            if (inimigoAnterior == NULL) {
                jogo->inimigos = inimigoAtual->prox;  // Atualiza o início da lista se o inimigo removido é o primeiro
            } else {
                inimigoAnterior->prox = inimigoAtual->prox;  // Bypass o inimigo removido
            }
            Inimigo* temp = inimigoAtual;  // Armazena o ponteiro do inimigo atual
            inimigoAtual = inimigoAtual->prox;  // Move para o próximo inimigo na lista
            free(temp);  // Libera a memória do inimigo removido

            jogo->jogador.vidas--;  // Reduz uma vida do jogador
            if (jogo->jogador.vidas <= 0) {
                jogo->gameOver = 1;  // Define game over se o número de vidas do jogador for zero
            } else {
                inicializarJogo(jogo, jogo->jogador.vidas, jogo->pontuacao);  // Reinicia o jogo com o número de vidas e pontuação atual
            }
            return;  // Sai da função após a colisão e remoção do inimigo
        } else {
            inimigoAnterior = inimigoAtual;  // Atualiza o ponteiro para o inimigo anterior
            inimigoAtual = inimigoAtual->prox;  // Move para o próximo inimigo na lista
        }
    }

    Combustivel* combustivelAtual = jogo->combustiveis;  // Ponteiro para iterar sobre a lista de combustíveis
    Combustivel* combustivelAnterior = NULL;  // Ponteiro para manter o combustível anterior na lista

    // Itera sobre todos os combustíveis na lista
    while (combustivelAtual != NULL) {
        // Verifica se há colisão entre o jogador e o combustível atual
        if (checarColisao(jogo->jogador.posicao, combustivelAtual->posicao, (Vector2){60, 80}, (Vector2){40, 40})) {
            // Remove o combustível da lista se colidir com o jogador
            if (combustivelAnterior == NULL) {
                jogo->combustiveis = combustivelAtual->prox;  // Atualiza o início da lista se o combustível removido é o primeiro
            } else {
                combustivelAnterior->prox = combustivelAtual->prox;  // Bypass o combustível removido
            }
            Combustivel* temp = combustivelAtual;  // Armazena o ponteiro do combustível atual
            combustivelAtual = combustivelAtual->prox;  // Move para o próximo combustível na lista
            free(temp);  // Libera a memória do combustível removido

            jogo->jogador.combustivel += 15.0f;  // Adiciona 30 ao combustível do jogador
            jogo->pontuacao += 10;  // Adiciona 10 à pontuação
            if (jogo->jogador.combustivel > 100.0f) {
                jogo->jogador.combustivel = 100.0f;  // Garante que o combustível não ultrapasse o máximo de 100
            }
        } else {
            combustivelAnterior = combustivelAtual;  // Atualiza o ponteiro para o combustível anterior
            combustivelAtual = combustivelAtual->prox;  // Move para o próximo combustível na lista
        }
    }
}

void checarColisoesProjeteis(ObjetoJogo* jogo) {
    // Inicializa ponteiros para iterar e manter o projétil atual e anterior na lista de projéteis
    Projetil* projAtual = jogo->projeteis;
    Projetil* projAnterior = NULL;

    // Itera sobre todos os projéteis na lista
    while (projAtual != NULL) {
        // Inicializa ponteiros para iterar e manter o inimigo atual e anterior na lista de inimigos
        Inimigo* inimigoAtual = jogo->inimigos;
        Inimigo* inimigoAnterior = NULL;

        // Itera sobre todos os inimigos na lista
        while (inimigoAtual != NULL) {
            // Verifica se há colisão entre o projétil atual e o inimigo atual
            if (checarColisao(projAtual->posicao, inimigoAtual->posicao, (Vector2){5, 10}, (Vector2){40, 40})) {
                // Remove o projétil da lista
                if (projAnterior == NULL) {
                    // Atualiza o início da lista se o projétil removido é o primeiro
                    jogo->projeteis = projAtual->prox;
                } else {
                    // Bypass o projétil removido
                    projAnterior->prox = projAtual->prox;
                }
                // Armazena o ponteiro do projétil atual e move para o próximo projétil
                Projetil* tempProj = projAtual;
                projAtual = projAtual->prox;
                free(tempProj); // Libera a memória do projétil removido

                // Remove o inimigo da lista
                if (inimigoAnterior == NULL) {
                    // Atualiza o início da lista se o inimigo removido é o primeiro
                    jogo->inimigos = inimigoAtual->prox;
                } else {
                    // Bypass o inimigo removido
                    inimigoAnterior->prox = inimigoAtual->prox;
                }
                // Armazena o ponteiro do inimigo atual e move para o próximo inimigo
                Inimigo* tempInimigo = inimigoAtual;
                inimigoAtual = inimigoAtual->prox;
                free(tempInimigo); // Libera a memória do inimigo removido
                
                // Adiciona pontos ao jogador
                jogo->pontuacao += 30; // Adiciona 30 à pontuação
                
                // Sai do loop de inimigos para verificar outros projéteis
                break;
            } else {
                // Atualiza o ponteiro para o inimigo anterior e move para o próximo inimigo
                inimigoAnterior = inimigoAtual;
                inimigoAtual = inimigoAtual->prox;
            }
        }

        // Se o projétil atual não foi removido, atualiza o ponteiro para o projétil anterior e move para o próximo projétil
        if (projAtual != NULL) {
            projAnterior = projAtual;
            projAtual = projAtual->prox;
        }
    }
}

void checarColisoesProjeteisCombustiveis(ObjetoJogo* jogo) {
    // Inicializa ponteiros para iterar e manter o projétil atual e anterior na lista de projéteis
    Projetil* projAtual = jogo->projeteis;
    Projetil* projAnterior = NULL;

    // Itera sobre todos os projéteis na lista
    while (projAtual != NULL) {
        // Inicializa ponteiros para iterar e manter o combustível atual e anterior na lista de combustíveis
        Combustivel* combustivelAtual = jogo->combustiveis;
        Combustivel* combustivelAnterior = NULL;

        // Itera sobre todos os combustíveis na lista
        while (combustivelAtual != NULL) {
            // Verifica se há colisão entre o projétil atual e o combustível atual
            if (checarColisao(projAtual->posicao, combustivelAtual->posicao, (Vector2){5, 10}, (Vector2){40, 40})) {
                // Remove o projétil da lista
                if (projAnterior == NULL) {
                    // Atualiza o início da lista se o projétil removido é o primeiro
                    jogo->projeteis = projAtual->prox;
                } else {
                    // Bypass o projétil removido
                    projAnterior->prox = projAtual->prox;
                }
                // Armazena o ponteiro do projétil atual e move para o próximo projétil
                Projetil* tempProj = projAtual;
                projAtual = projAtual->prox;
                free(tempProj); // Libera a memória do projétil removido

                // Remove o combustível da lista
                if (combustivelAnterior == NULL) {
                    // Atualiza o início da lista se o combustível removido é o primeiro
                    jogo->combustiveis = combustivelAtual->prox;
                } else {
                    // Bypass o combustível removido
                    combustivelAnterior->prox = combustivelAtual->prox;
                }
                // Armazena o ponteiro do combustível atual e move para o próximo combustível
                Combustivel* tempCombustivel = combustivelAtual;
                combustivelAtual = combustivelAtual->prox;
                free(tempCombustivel); // Libera a memória do combustível removido

                // Adiciona pontos ao jogador
                jogo->pontuacao += 20; // Adiciona 20 à pontuação

                // Sai do loop de combustíveis para verificar outros projéteis
                break;
            } else {
                // Atualiza o ponteiro para o combustível anterior e move para o próximo combustível
                combustivelAnterior = combustivelAtual;
                combustivelAtual = combustivelAtual->prox;
            }
        }

        // Se o projétil atual não foi removido, atualiza o ponteiro para o projétil anterior e move para o próximo projétil
        if (projAtual != NULL) {
            projAnterior = projAtual;
            projAtual = projAtual->prox;
        }
    }
}

void desenharJogador(Jogador* jogador) {
    DrawTexture(jogadorTexture, jogador->posicao.x, jogador->posicao.y, WHITE);
    
    // Exibe o nível de combustível do jogador na tela, na posição (10, 30), com uma fonte de tamanho 20 e cor cinza escuro
    DrawText(TextFormat("Combustivel: %.0f", jogador->combustivel), 10, 30, 20, DARKGRAY);
    
    // Exibe o número de vidas do jogador na tela, na posição (10, 50), com uma fonte de tamanho 20 e cor cinza escuro
    DrawText(TextFormat("Vidas: %d", jogador->vidas), 10, 50, 20, DARKGRAY);
}

int main() {
    const int larguraTela = 600;  // Define a largura da tela do jogo
    const int alturaTela = 600;   // Define a altura da tela do jogo
    InitWindow(larguraTela, alturaTela, "RiverXGH");  // Inicializa a janela do jogo com o título "RiverXGH"
    
    jogadorTexture = LoadTexture("resources/Fighter_type_A1.png");

    ObjetoJogo jogo;  // Declara uma variável do tipo ObjetoJogo para armazenar o estado do jogo
    inicializarJogo(&jogo, 3, 0); // Inicializa o jogo com 3 vidas e pontuação 0
    jogo.projeteis = NULL; // Inicializa a lista de projéteis como vazia

    int dificuldade = 1;  // Variável para armazenar a dificuldade inicial do jogo
    int frames = 0;       // Variável para contar o número de frames

    SetTargetFPS(60);  // Define o FPS (Frames Por Segundo) alvo para 60

    int contadorInimigos = 0;   // Contador para controlar a geração de inimigos
    int contadorCombustiveis = 0;  // Contador para controlar a geração de combustíveis

    // Loop principal do jogo: continua enquanto a janela não for fechada
    while (!WindowShouldClose()) {
        // Se o jogo não estiver em estado de "game over"
        if (!jogo.gameOver) {
            contadorInimigos++;
            contadorCombustiveis++;
            frames++;

            // Aumenta a dificuldade a cada 900 frames (aproximadamente a cada 15 segundos se a 60 FPS)
            if (frames % 900 == 0) {
                dificuldade++;
            }

            // Verifica se é hora de gerar um novo inimigo
            if (contadorInimigos >= 100 / dificuldade) {
                inserirInimigo(&jogo.inimigos, GetRandomValue(0, larguraTela - 30), 0);
                contadorInimigos = 0;
            }
            // Verifica se é hora de gerar um novo combustível
            if (contadorCombustiveis >= 200 / dificuldade) {
                inserirCombustivel(&jogo.combustiveis, GetRandomValue(0, larguraTela - 30), 0);
                contadorCombustiveis = 0;
            }

            // Verifica se a tecla de espaço foi pressionada para disparar um projétil
            if (IsKeyPressed(KEY_SPACE)) { 
            inserirProjetil(&jogo.projeteis, jogo.jogador.posicao.x, jogo.jogador.posicao.y); // Passa a posição do jogador 
            }

            atualizarJogador(&jogo.jogador);  // Atualiza o estado do jogador
            atualizarInimigos(jogo.inimigos, dificuldade);  // Atualiza o estado dos inimigos com a dificuldade atual
            atualizarCombustiveis(jogo.combustiveis, dificuldade);  // Atualiza o estado dos combustíveis com a dificuldade atual
            atualizarProjeteis(&jogo.projeteis);  // Atualiza o estado dos projéteis
            checarColisoes(&jogo);  // Verifica colisões entre o jogador e outros objetos
            checarColisoesProjeteis(&jogo);  // Verifica colisões entre projéteis e inimigos
            checarColisoesProjeteisCombustiveis(&jogo); // Adiciona a verificação de colisões entre projéteis e combustíveis

            // Verifica se o combustível do jogador acabou
            if (jogo.jogador.combustivel <= 0) {
                jogo.jogador.vidas--;  // Reduz uma vida do jogador
                if (jogo.jogador.vidas <= 0) {
                    jogo.gameOver = 1;  // Define game over se o número de vidas do jogador for zero
                } else {
                    inicializarJogo(&jogo, jogo.jogador.vidas, jogo.pontuacao);  // Reinicia o jogo com o número de vidas e pontuação atual
                }
            }
        }

        BeginDrawing();  // Inicia o desenho na tela
        ClearBackground(RAYWHITE);  // Limpa o fundo da tela com a cor branca

        // Se o jogo está em estado de "game over"
        if (jogo.gameOver) {
            int gameOverWidth = MeasureText("Game Over!", 40);  // Calcula a largura do texto "Game Over!"
            DrawText("Game Over!", larguraTela / 2 - gameOverWidth / 2, alturaTela / 2 - 20, 40, RED); // Desenha o texto "Game Over!" centralizado na tela
            int pontuacaoWidth = MeasureText(TextFormat("Pontuacao Final: %d", jogo.pontuacao), 20);  // Calcula a largura do texto da pontuação final
            DrawText(TextFormat("Pontuacao Final: %d", jogo.pontuacao), larguraTela / 2 - pontuacaoWidth / 2, alturaTela / 2 + 20, 20, DARKGRAY); // Desenha a pontuação final abaixo de "Game Over"
        } else {  // Se o jogo não está em estado de "game over"
            desenharJogador(&jogo.jogador);  // Desenha o jogador na tela
            desenharInimigos(jogo.inimigos);  // Desenha os inimigos na tela
            desenharCombustiveis(jogo.combustiveis);  // Desenha os combustíveis na tela
            desenharProjeteis(jogo.projeteis);  // Desenha os projéteis na tela

            // Desenha a pontuação, combustível e vidas apenas quando o jogo não está em estado de "Game Over"
            DrawText(TextFormat("Pontuacao: %d", jogo.pontuacao), 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Combustivel: %.0f", jogo.jogador.combustivel), 10, 30, 20, DARKGRAY);
            DrawText(TextFormat("Vidas: %d", jogo.jogador.vidas), 10, 50, 20, DARKGRAY);
        }

        EndDrawing();  // Finaliza o desenho na tela
    }

    liberarInimigos(jogo.inimigos);  // Libera a memória dos inimigos
    liberarCombustiveis(jogo.combustiveis);  // Libera a memória dos combustíveis
    liberarProjeteis(jogo.projeteis); // Libera a memória dos projéteis
    UnloadTexture(jogadorTexture);
    CloseWindow();  // Fecha a janela do jogo

    return 0;  // Retorna 0 para indicar que o programa terminou com sucesso
}
