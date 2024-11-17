#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int noMenuInicial = 1;  // Inicialmente, o jogo está no menu inicial

Texture2D jogadorTexture;
Sound tiroSound;
Sound aviaoSound;
Sound explosaoSound;
Sound quedaSound;
Music musicaFundo;

Rectangle botaoReiniciar = { 200, 400, 200, 50 };
Rectangle botaoVoltarMenu = { 250, 500, 200, 50 };

// Estrutura para armazenar as informações do jogador
typedef struct Jogador {
    Vector2 posicao;   // Posição do jogador
    float velocidade;  // Velocidade de movimento do jogador
    int vida;          // Vida do jogador
    float combustivel; // Combustível do jogador
    int vidas;         // Número de vidas do jogador
} Jogador;

// Enumeração de diferentes tipos de inimigo
typedef enum {
    INIMIGO_BASICO,
    INIMIGO_RAPIDO,
    INIMIGO_ZIGZAG,
    // Adicione novos tipos aqui
} TipoInimigo;


typedef struct Inimigo {
    Vector2 posicao;   // Posição do inimigo
    TipoInimigo tipo;  // Tipo do inimigo
    float velocidade;  // Velocidade de movimento
    float amplitude;   // Para movimento em zigzag
    float fase;        // Para movimento em zigzag
    struct Inimigo* prox;
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

typedef struct PadraoSpawn {
    int numInimigos;
    Vector2 inimigos[5];
} PadraoSpawn;

PadraoSpawn padroes[] = {
    { 3, { {150, 0}, {300, 0}, {450, 0} } },
    { 2, { {200, 0}, {400, 0} } },
    { 4, { {100, 0}, {200, 0}, {400, 0}, {500, 0} } },
};

int numPadroes = sizeof(padroes) / sizeof(PadraoSpawn);

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
Inimigo* criarInimigo(float x, float y, TipoInimigo tipo) {
    Inimigo* novo = (Inimigo*)malloc(sizeof(Inimigo));
    novo->posicao = (Vector2){ x, y };
    novo->tipo = tipo;
    novo->prox = NULL;
    
    // Configurações específicas para cada tipo
    switch (tipo) {
        case INIMIGO_BASICO:
            novo->velocidade = 2.0f;
            break;
        case INIMIGO_RAPIDO:
            novo->velocidade = 4.0f;
            break;
        case INIMIGO_ZIGZAG:
            novo->velocidade = 2.5f;
            novo->amplitude = 100.0f;
            novo->fase = 0.0f;
            break;
    }
    
    return novo;
}

// Função para inserir um inimigo na lista de inimigos
void inserirInimigo(Inimigo** head, float x, float y, TipoInimigo tipo) {
    Inimigo* novo = criarInimigo(x, y, tipo);
    novo->prox = *head;
    *head = novo;
}

// Função para criar um novo projétil
Projetil* criarProjetil(float x, float y) {
    Projetil* novo = (Projetil*)malloc(sizeof(Projetil)); // Aloca memória para um novo projétil
    novo->posicao = (Vector2){ x, y }; // Define a posição do projétil
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
    novo->posicao = (Vector2){ x, y }; // Define a posição do combustível
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
void atualizarInimigos(Inimigo* head) {
    Inimigo* atual = head;
    while (atual != NULL) {
        switch (atual->tipo) {
            case INIMIGO_BASICO:
                atual->posicao.y += atual->velocidade;
                break;
                
            case INIMIGO_RAPIDO:
                atual->posicao.y += atual->velocidade;
                break;
                
            case INIMIGO_ZIGZAG:
                atual->posicao.y += atual->velocidade;
                atual->fase += 0.05f;
                atual->posicao.x = atual->posicao.x + sinf(atual->fase) * 2.0f;
                break;
        }
        atual = atual->prox;
    }
}

// Função para atualizar a posição dos combustíveis
void atualizarCombustiveis(Combustivel* head) {
    Combustivel* atual = head;
    while (atual != NULL) {
        // Atualiza a posição do combustível na direção Y com uma velocidade constante
        atual->posicao.y += 2.0f;
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
            }
            else {
                anterior->prox = atual->prox;  // Faz o projétil anterior apontar para o próximo, pulando o projétil atual
            }
            Projetil* temp = atual;  // Armazena o ponteiro do projétil a ser removido
            atual = atual->prox;  // Move para o próximo projétil na lista
            free(temp);  // Libera a memória do projétil removido
        }
        else {
            anterior = atual;  // Atualiza o ponteiro do projétil anterior
            atual = atual->prox;  // Move para o próximo projétil na lista
        }
    }
}

// Função para desenhar todos os inimigos na tela
void desenharInimigos(Inimigo* head) {
    Inimigo* atual = head;
    while (atual != NULL) {
        Color cor;
        Vector2 tamanho;
        
        switch (atual->tipo) {
            case INIMIGO_BASICO:
                cor = RED;
                tamanho = (Vector2){ 40, 40 };
                break;
                
            case INIMIGO_RAPIDO:
                cor = ORANGE;
                tamanho = (Vector2){ 30, 45 };
                break;
                
            case INIMIGO_ZIGZAG:
                cor = PURPLE;
                tamanho = (Vector2){ 35, 35 };
                break;
        }
        
        DrawRectangleV(atual->posicao, tamanho, cor);
        atual = atual->prox;
    }
}

// Função para desenhar todos os combustíveis na tela
void desenharCombustiveis(Combustivel* head) {
    Combustivel* atual = head;  // Ponteiro para o combustível atual
    while (atual != NULL) {
        // Desenha cada combustível como um retângulo verde na posição atual
        DrawRectangleV(atual->posicao, (Vector2) { 40, 40 }, GREEN);
        atual = atual->prox;  // Move para o próximo combustível na lista
    }
}

// Função para desenhar todos os projéteis na tela
void desenharProjeteis(Projetil* head) {
    Projetil* atual = head;  // Ponteiro para o projétil atual
    while (atual != NULL) {
        // Desenha cada projétil como um retângulo preto na posição atual
        DrawRectangleV(atual->posicao, (Vector2) { 5, 10 }, BLACK);
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

// Função para carregar a maior pontuação do arquivo
int carregarHighScore() {
    FILE* arquivo = fopen("highscore.txt", "r");
    if (arquivo == NULL) {
        return 0;  // Retorna 0 se o arquivo não existir
    }

    int highScore;
    fscanf(arquivo, "%d", &highScore);
    fclose(arquivo);
    return highScore;
}

// Função para salvar a maior pontuação no arquivo
void salvarHighScore(int pontuacao) {
    int highScoreAtual = carregarHighScore();
    if (pontuacao > highScoreAtual) {
        FILE* arquivo = fopen("highscore.txt", "w");
        if (arquivo != NULL) {
            fprintf(arquivo, "%d", pontuacao);
            fclose(arquivo);
        }
    }
}

// Função para inicializar o jogo com os parâmetros fornecidos
void inicializarJogo(ObjetoJogo* jogo, int vidas, int pontuacao) {
    // Inicia a música de fundo
    PlayMusicStream(musicaFundo);
    // Define a posição inicial do jogador
    jogo->jogador.posicao = (Vector2){ (700 - 67) / 2, 700 - 86 - 10 }; // Centraliza o jogador horizontalmente    
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
    if (IsKeyDown(KEY_RIGHT) && jogador->posicao.x < 700 - 67) jogador->posicao.x += jogador->velocidade;
    if (IsKeyDown(KEY_D) && jogador->posicao.x < 700 - 67) jogador->posicao.x += jogador->velocidade;
    // Move o jogador para a esquerda se a tecla direcional esquerda estiver pressionada e o jogador estiver dentro dos limites
    if (IsKeyDown(KEY_LEFT) && jogador->posicao.x > 0) jogador->posicao.x -= jogador->velocidade;
    if (IsKeyDown(KEY_A) && jogador->posicao.x > 0) jogador->posicao.x -= jogador->velocidade;
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
        if (checarColisao(jogo->jogador.posicao, inimigoAtual->posicao, (Vector2) { 60, 80 }, (Vector2) { 40, 40 })) {

            PlaySound(explosaoSound);
            // Remove o inimigo da lista se colidir com o jogador
            if (inimigoAnterior == NULL) {
                jogo->inimigos = inimigoAtual->prox;  // Atualiza o início da lista se o inimigo removido é o primeiro
            }
            else {
                inimigoAnterior->prox = inimigoAtual->prox;  // Bypass o inimigo removido
            }
            Inimigo* temp = inimigoAtual;  // Armazena o ponteiro do inimigo atual
            inimigoAtual = inimigoAtual->prox;  // Move para o próximo inimigo na lista
            free(temp);  // Libera a memória do inimigo removido

            jogo->jogador.vidas--;  // Reduz uma vida do jogador
            if (jogo->jogador.vidas <= 0) {
                jogo->gameOver = 1;  // Define game over se o número de vidas do jogador for zero
            }
            else {
                inicializarJogo(jogo, jogo->jogador.vidas, jogo->pontuacao);  // Reinicia o jogo com o número de vidas e pontuação atual
            }
            return;  // Sai da função após a colisão e remoção do inimigo
        }
        else {
            inimigoAnterior = inimigoAtual;  // Atualiza o ponteiro para o inimigo anterior
            inimigoAtual = inimigoAtual->prox;  // Move para o próximo inimigo na lista
        }
    }

    Combustivel* combustivelAtual = jogo->combustiveis;  // Ponteiro para iterar sobre a lista de combustíveis
    Combustivel* combustivelAnterior = NULL;  // Ponteiro para manter o combustível anterior na lista

    // Itera sobre todos os combustíveis na lista
    while (combustivelAtual != NULL) {
        // Verifica se há colisão entre o jogador e o combustível atual
        if (checarColisao(jogo->jogador.posicao, combustivelAtual->posicao, (Vector2) { 60, 80 }, (Vector2) { 40, 40 })) {
            // Remove o combustível da lista se colidir com o jogador
            if (combustivelAnterior == NULL) {
                jogo->combustiveis = combustivelAtual->prox;  // Atualiza o início da lista se o combustível removido é o primeiro
            }
            else {
                combustivelAnterior->prox = combustivelAtual->prox;  // Bypass o combustível removido
            }
            Combustivel* temp = combustivelAtual;  // Armazena o ponteiro do combustível atual
            combustivelAtual = combustivelAtual->prox;  // Move para o próximo combustível na lista
            free(temp);  // Libera a memória do combustível removido

            jogo->jogador.combustivel += 30.0f;  // Adiciona 30 ao combustível do jogador
            jogo->pontuacao += 10;  // Adiciona 10 à pontuação
            if (jogo->jogador.combustivel > 100.0f) {
                jogo->jogador.combustivel = 100.0f;  // Garante que o combustível não ultrapasse o máximo de 100
            }
        }
        else {
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
            if (checarColisao(projAtual->posicao, inimigoAtual->posicao, (Vector2) { 5, 10 }, (Vector2) { 40, 40 })) {
                // Remove o projétil da lista
                if (projAnterior == NULL) {
                    // Atualiza o início da lista se o projétil removido é o primeiro
                    jogo->projeteis = projAtual->prox;
                }
                else {
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
                }
                else {
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
            }
            else {
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
            if (checarColisao(projAtual->posicao, combustivelAtual->posicao, (Vector2) { 5, 10 }, (Vector2) { 40, 40 })) {
                // Remove o projétil da lista
                if (projAnterior == NULL) {
                    // Atualiza o início da lista se o projétil removido é o primeiro
                    jogo->projeteis = projAtual->prox;
                }
                else {
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
                }
                else {
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
            }
            else {
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
}

void resetarJogo(ObjetoJogo* jogo) {
    inicializarJogo(jogo, 3, 0);
    jogo->projeteis = NULL;
}

int main() {
    const int larguraTela = 700;  // Define a largura da tela do jogo
    const int alturaTela = 700;   // Define a altura da tela do jogo
    InitWindow(larguraTela, alturaTela, "RiverXGH");  // Inicializa a janela do jogo com o título "RiverXGH"
    InitAudioDevice();

    jogadorTexture = LoadTexture("resources/Fighter_type_A1.png");
    tiroSound = LoadSound("resources/tiro.mp3");
    aviaoSound = LoadSound("resources/aviao.mp3");
    explosaoSound = LoadSound("resources/explosao.mp3");
    quedaSound = LoadSound("resources/queda.mp3");
    musicaFundo = LoadMusicStream("resources/musicafundo.mp3");
    SetSoundVolume(aviaoSound, 0.1f);
    SetMusicVolume(musicaFundo, 0.2f);

    ObjetoJogo jogo;  // Declara uma variável do tipo ObjetoJogo para armazenar o estado do jogo
    inicializarJogo(&jogo, 3, 0); // Inicializa o jogo com 3 vidas e pontuação 0
    jogo.projeteis = NULL; // Inicializa a lista de projéteis como vazia

    int frames = 0;       // Variável para contar o número de frames
    int noMenuInicial = 1;  // Inicialmente, o jogo está no menu inicial

    SetTargetFPS(60);  // Define o FPS (Frames Por Segundo) alvo para 60

    int contadorInimigos = 0;   // Contador para controlar a geração de inimigos
    int contadorCombustiveis = 0;  // Contador para controlar a geração de combustíveis

    while (!WindowShouldClose()) {
        UpdateMusicStream(musicaFundo);

        BeginDrawing();
        ClearBackground(BLUE);

        if (noMenuInicial) {
            // Exibir o menu inicial
            DrawText("RiverXGH", larguraTela / 2 - MeasureText("RiverXGH", 40) / 2, alturaTela / 2 - 50, 40, RAYWHITE);
            DrawText("Aperte Enter para iniciar", larguraTela / 2 - MeasureText("Aperte Enter para iniciar", 20) / 2, alturaTela / 2 + 10, 20, RAYWHITE);

            // Verifica se a tecla Enter foi pressionada
            if (IsKeyPressed(KEY_ENTER)) {
                noMenuInicial = 0;  // Muda o estado para começar o jogo
                PlaySound(aviaoSound);  // Inicia o som de avião de fundo
                resetarJogo(&jogo); // Reinicializa o jogo
            }
        }
        else {
            // Lógica do jogo
            if (!jogo.gameOver) {
                contadorInimigos++;
                contadorCombustiveis++;
                frames++;

                if (contadorInimigos >= 200) {
                    int padraoIndex = GetRandomValue(0, numPadroes - 1);
                    PadraoSpawn padraoEscolhido = padroes[padraoIndex];

                    for (int i = 0; i < padraoEscolhido.numInimigos; i++) {
                        TipoInimigo tipoAleatorio = GetRandomValue(0, 2); // Escolhe um tipo aleatório
                        inserirInimigo(&jogo.inimigos, 
                                    padraoEscolhido.inimigos[i].x, 
                                    padraoEscolhido.inimigos[i].y, 
                                    tipoAleatorio);
                    }

                    contadorInimigos = 0;
                }

                if (contadorCombustiveis >= 300) {
                    inserirCombustivel(&jogo.combustiveis, GetRandomValue(0, larguraTela - 30), 0);
                    contadorCombustiveis = 0;
                }

                if (IsKeyPressed(KEY_SPACE)) {
                    PlaySound(tiroSound);
                    inserirProjetil(&jogo.projeteis, jogo.jogador.posicao.x, jogo.jogador.posicao.y);
                }

                atualizarJogador(&jogo.jogador);
                atualizarInimigos(jogo.inimigos);
                atualizarCombustiveis(jogo.combustiveis);
                atualizarProjeteis(&jogo.projeteis);
                checarColisoes(&jogo);
                checarColisoesProjeteis(&jogo);
                checarColisoesProjeteisCombustiveis(&jogo);

                if (jogo.jogador.combustivel <= 0) {
                    PlaySound(quedaSound);
                    jogo.jogador.vidas--;
                    if (jogo.jogador.vidas <= 0) {
                        jogo.gameOver = 1;
                    }
                    else {
                        inicializarJogo(&jogo, jogo.jogador.vidas, jogo.pontuacao);
                    }
                }
            }

            if (jogo.gameOver) {
                // Desenhar o texto de "Game Over" e pontuação final
                salvarHighScore(jogo.pontuacao);
                int highScore = carregarHighScore();
                int gameOverWidth = MeasureText("Game Over!", 40);
                DrawText("Game Over!", larguraTela / 2 - gameOverWidth / 2, alturaTela / 2 - 60, 40, RED);
                StopSound(aviaoSound);
                StopMusicStream(musicaFundo);

                int pontuacaoWidth = MeasureText(TextFormat("Pontuacao Final: %d", jogo.pontuacao), 20);
                DrawText(TextFormat("Pontuacao Final: %d", jogo.pontuacao), larguraTela / 2 - pontuacaoWidth / 2, alturaTela / 2, 20, RAYWHITE);

                int highScoreWidth = MeasureText(TextFormat("High Score: %d", highScore), 20);
                DrawText(TextFormat("High Score: %d", highScore), larguraTela / 2 - highScoreWidth / 2, alturaTela / 2 + 30, 20, GOLD);

                // Definir a largura e altura do botão "Jogar Novamente"
                botaoReiniciar.width = 200;
                botaoReiniciar.height = 50;
                botaoReiniciar.x = larguraTela / 2 - botaoReiniciar.width / 2;
                botaoReiniciar.y = alturaTela / 2 + 60;

                DrawRectangleRec(botaoReiniciar, RAYWHITE);
                DrawText("Jogar Novamente", botaoReiniciar.x + (botaoReiniciar.width / 2) - (MeasureText("Jogar Novamente", 20) / 2), botaoReiniciar.y + 15, 20, BLACK);

                // Desenhar o botão "Voltar ao Menu"
                Rectangle botaoVoltarMenu = { larguraTela / 2 - 100, alturaTela / 2 + 130, 200, 50 };  // Ajuste a posição conforme necessário
                DrawRectangleRec(botaoVoltarMenu, RAYWHITE);
                DrawText("Voltar ao Menu", botaoVoltarMenu.x + (botaoVoltarMenu.width / 2) - (MeasureText("Voltar ao Menu", 20) / 2), botaoVoltarMenu.y + 15, 20, BLACK);

                // Detectar cliques nos botões
                Vector2 mousePos = GetMousePosition();
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, botaoReiniciar)) {
                        // Reiniciar o jogo
                        inicializarJogo(&jogo, 3, 0);
                        jogo.projeteis = NULL;
                        frames = 0;
                        contadorInimigos = 0;
                        contadorCombustiveis = 0;
                    }
                    else if (CheckCollisionPointRec(mousePos, botaoVoltarMenu)) {
                        noMenuInicial = 1;  // Retornar ao menu inicial
                        PlayMusicStream(musicaFundo);  // Reiniciar a música de fundo
                    }
                }
            }
            else {  // Se o jogo não está em estado de "game over"
                desenharJogador(&jogo.jogador);
                desenharInimigos(jogo.inimigos);
                desenharCombustiveis(jogo.combustiveis);
                desenharProjeteis(jogo.projeteis);

                DrawText(TextFormat("Pontuacao: %d", jogo.pontuacao), 10, 10, 20, RAYWHITE);
                DrawText(TextFormat("Combustivel: %.0f", jogo.jogador.combustivel), 10, 30, 20, RAYWHITE);
                DrawText(TextFormat("Vidas: %d", jogo.jogador.vidas), 10, 50, 20, RAYWHITE);
            }
        }

        EndDrawing();
    }

    liberarInimigos(jogo.inimigos);  // Libera a memória dos inimigos
    liberarCombustiveis(jogo.combustiveis);  // Libera a memória dos combustíveis
    liberarProjeteis(jogo.projeteis); // Libera a memória dos projéteis
    UnloadTexture(jogadorTexture);
    UnloadSound(tiroSound);
    UnloadSound(aviaoSound);
    UnloadSound(explosaoSound);
    UnloadSound(quedaSound);
    UnloadMusicStream(musicaFundo);
    CloseAudioDevice();
    CloseWindow();  // Fecha a janela do jogo

    return 0;  // Retorna 0 para indicar que o programa terminou com sucesso
}
