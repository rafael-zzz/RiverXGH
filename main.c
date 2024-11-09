#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Jogador {
    Vector2 posicao;
    float velocidade;
    int vida;
    float combustivel;
} Jogador;

typedef struct Inimigo {
    Vector2 posicao;
    struct Inimigo* prox;
} Inimigo;

typedef struct Combustivel {
    Vector2 posicao;
    struct Combustivel* prox;
} Combustivel;

typedef struct Projetil {
    Vector2 posicao;
    struct Projetil* prox;
} Projetil;

typedef struct ObjetoJogo {
    Jogador jogador;
    Inimigo* inimigos;
    Combustivel* combustiveis;
    Projetil* projeteis;
    int pontuacao;
    int gameOver;
} ObjetoJogo;

Inimigo* criarInimigo(float x, float y) {
    Inimigo* novo = (Inimigo*)malloc(sizeof(Inimigo));
    novo->posicao = (Vector2){x, y};
    novo->prox = NULL;
    return novo;
}

void inserirInimigo(Inimigo** head, float x, float y) {
    Inimigo* novo = criarInimigo(x, y);
    novo->prox = *head;
    *head = novo;
}

Projetil* criarProjetil(float x, float y) {
    Projetil* novo = (Projetil*)malloc(sizeof(Projetil));
    novo->posicao = (Vector2){x, y};
    novo->prox = NULL;
    return novo;
}

void inserirProjetil(Projetil** head, float x, float y) {
    Projetil* novo = criarProjetil(x, y);
    novo->prox = *head;
    *head = novo;
}

Combustivel* criarCombustivel(float x, float y) {
    Combustivel* novo = (Combustivel*)malloc(sizeof(Combustivel));
    novo->posicao = (Vector2){x, y};
    novo->prox = NULL;
    return novo;
}

void inserirCombustivel(Combustivel** head, float x, float y) {
    Combustivel* novo = criarCombustivel(x, y);
    novo->prox = *head;
    *head = novo;
}

void atualizarInimigos(Inimigo* head) {
    Inimigo* atual = head;
    while (atual != NULL) {
        atual->posicao.y += 2.0f;
        atual = atual->prox;
    }
}

void atualizarCombustiveis(Combustivel* head) {
    Combustivel* atual = head;
    while (atual != NULL) {
        atual->posicao.y += 2.0f;
        atual = atual->prox;
    }
}

void atualizarProjeteis(Projetil** head) {
    Projetil* atual = *head;
    Projetil* anterior = NULL;

    while (atual != NULL) {
        atual->posicao.y -= 5.0f; // Velocidade do projétil

        if (atual->posicao.y < 0) {
            // Remove o projétil da lista se sair da tela
            if (anterior == NULL) {
                *head = atual->prox;
            } else {
                anterior->prox = atual->prox;
            }
            Projetil* temp = atual;
            atual = atual->prox;
            free(temp);
        } else {
            anterior = atual;
            atual = atual->prox;
        }
    }
}

void desenharInimigos(Inimigo* head) {
    Inimigo* atual = head;
    while (atual != NULL) {
        DrawRectangleV(atual->posicao, (Vector2){20, 20}, RED);
        atual = atual->prox;
    }
}

void desenharCombustiveis(Combustivel* head) {
    Combustivel* atual = head;
    while (atual != NULL) {
        DrawRectangleV(atual->posicao, (Vector2){20, 20}, GREEN);
        atual = atual->prox;
    }
}

void desenharProjeteis(Projetil* head) {
    Projetil* atual = head;
    while (atual != NULL) {
        DrawRectangleV(atual->posicao, (Vector2){5, 10}, BLACK);
        atual = atual->prox;
    }
}

void liberarInimigos(Inimigo* head) {
    Inimigo* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->prox;
        free(tmp);
    }
}

void liberarCombustiveis(Combustivel* head) {
    Combustivel* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->prox;
        free(tmp);
    }
}

void liberarProjeteis(Projetil* head) {
    Projetil* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->prox;
        free(tmp);
    }
}

void inicializarJogo(ObjetoJogo* jogo) {
    jogo->jogador.posicao = (Vector2){400, 550};
    jogo->jogador.velocidade = 5.0f;
    jogo->jogador.vida = 3;
    jogo->jogador.combustivel = 100.0f;
    jogo->inimigos = NULL;
    jogo->combustiveis = NULL;
    jogo->pontuacao = 0;
    jogo->gameOver = 0;
}

void atualizarJogador(Jogador* jogador) {
    if (IsKeyDown(KEY_RIGHT) && jogador->posicao.x < 780) jogador->posicao.x += jogador->velocidade;
    if (IsKeyDown(KEY_LEFT) && jogador->posicao.x > 0) jogador->posicao.x -= jogador->velocidade;
    jogador->combustivel -= 0.1f;
}

int checarColisao(Vector2 pos1, Vector2 pos2, float tamanho) {
    return (pos1.x < pos2.x + tamanho && pos1.x + tamanho > pos2.x && pos1.y < pos2.y + tamanho && pos1.y + tamanho > pos2.y);
}

void checarColisoes(ObjetoJogo* jogo) {
    Inimigo* inimigoAtual = jogo->inimigos;
    Inimigo* inimigoAnterior = NULL;

    while (inimigoAtual != NULL) {
        if (checarColisao(jogo->jogador.posicao, inimigoAtual->posicao, 20)) {
            // Remove o inimigo da lista se colidir com o jogador
            if (inimigoAnterior == NULL) {
                jogo->inimigos = inimigoAtual->prox;
            } else {
                inimigoAnterior->prox = inimigoAtual->prox;
            }
            Inimigo* temp = inimigoAtual;
            inimigoAtual = inimigoAtual->prox;
            free(temp);
            jogo->jogador.vida--;

            if (jogo->jogador.vida <= 0) {
                jogo->gameOver = 1;
            }
        } else {
            inimigoAnterior = inimigoAtual;
            inimigoAtual = inimigoAtual->prox;
        }
    }

    Combustivel* combustivelAtual = jogo->combustiveis;
    Combustivel* combustivelAnterior = NULL;

    while (combustivelAtual != NULL) {
        if (checarColisao(jogo->jogador.posicao, combustivelAtual->posicao, 20)) {
            // Remove o combustível da lista se colidir com o jogador
            if (combustivelAnterior == NULL) {
                jogo->combustiveis = combustivelAtual->prox;
            } else {
                combustivelAnterior->prox = combustivelAtual->prox;
            }
            Combustivel* temp = combustivelAtual;
            combustivelAtual = combustivelAtual->prox;
            free(temp);
            jogo->jogador.combustivel = 100.0f;
        } else {
            combustivelAnterior = combustivelAtual;
            combustivelAtual = combustivelAtual->prox;
        }
    }
}

void checarColisoesProjeteis(ObjetoJogo* jogo) {
    Projetil* projAtual = jogo->projeteis;
    Projetil* projAnterior = NULL;

    while (projAtual != NULL) {
        Inimigo* inimigoAtual = jogo->inimigos;
        Inimigo* inimigoAnterior = NULL;

        while (inimigoAtual != NULL) {
            if (checarColisao(projAtual->posicao, inimigoAtual->posicao, 20)) {
                // Remove o projétil e o inimigo
                if (projAnterior == NULL) {
                    jogo->projeteis = projAtual->prox;
                } else {
                    projAnterior->prox = projAtual->prox;
                }
                Projetil* tempProj = projAtual;
                projAtual = projAtual->prox;
                free(tempProj);

                if (inimigoAnterior == NULL) {
                    jogo->inimigos = inimigoAtual->prox;
                } else {
                    inimigoAnterior->prox = inimigoAtual->prox;
                }
                Inimigo* tempInimigo = inimigoAtual;
                inimigoAtual = inimigoAtual->prox;
                free(tempInimigo);
                jogo->pontuacao += 10;
                break; // Para verificar outros projéteis
            } else {
                inimigoAnterior = inimigoAtual;
                inimigoAtual = inimigoAtual->prox;
            }
        }

        if (projAtual != NULL) {
            projAnterior = projAtual;
            projAtual = projAtual->prox;
        }
    }
}

void desenharJogador(Jogador* jogador) {
    DrawRectangleV(jogador->posicao, (Vector2){20, 20}, BLUE);
    DrawText(TextFormat("Combustivel: %.2f", jogador->combustivel), 10, 30, 20, DARKGRAY);
}

int main() {
    const int larguraTela = 800;
    const int alturaTela = 600;
    InitWindow(larguraTela, alturaTela, "River Raid com Raylib");

    ObjetoJogo jogo;
    inicializarJogo(&jogo);
    jogo.projeteis = NULL; // Inicializa a lista de projéteis

    SetTargetFPS(60);

    int contadorInimigos = 0;
    int contadorCombustiveis = 0;

    while (!WindowShouldClose()) {
        if (!jogo.gameOver) {
            contadorInimigos++;
            contadorCombustiveis++;

            if (contadorInimigos >= 60) {
                inserirInimigo(&jogo.inimigos, GetRandomValue(0, larguraTela - 20), 0);
                contadorInimigos = 0;
            }
            if (contadorCombustiveis >= 300) {
                inserirCombustivel(&jogo.combustiveis, GetRandomValue(0, larguraTela - 20), 0);
                contadorCombustiveis = 0;
            }

            if (IsKeyPressed(KEY_SPACE)) {
                inserirProjetil(&jogo.projeteis, jogo.jogador.posicao.x + 7.5f, jogo.jogador.posicao.y); // Ajuste a posição conforme necessário
            }

            atualizarJogador(&jogo.jogador);
            atualizarInimigos(jogo.inimigos);
            atualizarCombustiveis(jogo.combustiveis);
            atualizarProjeteis(&jogo.projeteis);
            checarColisoes(&jogo);
            checarColisoesProjeteis(&jogo);

            if (jogo.jogador.combustivel <= 0) {
                jogo.gameOver = 1;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (jogo.gameOver) {
            DrawText("Game Over!", larguraTela / 2 - 80, alturaTela / 2, 40, RED);
        } else {
            desenharJogador(&jogo.jogador);
            desenharInimigos(jogo.inimigos);
            desenharCombustiveis(jogo.combustiveis);
            desenharProjeteis(jogo.projeteis);
        }

        DrawText(TextFormat("Pontuacao: %d", jogo.pontuacao), 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    liberarInimigos(jogo.inimigos); // Libera memória dos inimigos
    liberarCombustiveis(jogo.combustiveis); // Libera memória dos combustíveis
    liberarProjeteis(jogo.projeteis); // Libera memória dos projéteis
    CloseWindow();

    return 0;
}
