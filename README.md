# Snake Game em C

Um jogo da cobrinha feito em **C** para **Windows**, executado no console, com sistema de menu, tela inicial personalizada, HUD, frutas raras, easter eggs e mapa centralizado visualmente na tela.

---

## Visão geral

Este projeto é uma versão personalizada do clássico **Snake Game**, desenvolvida em linguagem C utilizando recursos do console do Windows.

O jogo inclui:

- movimentação com **setas** ou **WASD**
- sistema de pontuação
- menu principal
- tela de instruções
- tela inicial com nome do jogador
- animação da cobra “comendo” o nome digitado
- tela de game over com opções de reinício e retorno ao menu
- frutas especiais com easter eggs
- HUD com pontuação, jogador e recorde
- centralização visual do mapa no console

---

## Tecnologias utilizadas

- **Linguagem C**
- Bibliotecas padrão:
  - `stdio.h`
  - `stdlib.h`
  - `time.h`
  - `string.h`
- Bibliotecas específicas do Windows:
  - `conio.h`
  - `windows.h`

---

## Compatibilidade

Este projeto foi feito para rodar em:

- **Windows**
- terminal do **VS Code**
- **CMD**
- **PowerShell**

> Como o jogo usa `windows.h`, `conio.h`, `SetConsoleCursorPosition`, `SetConsoleTextAttribute` e outras funções específicas do Windows, ele **não é compatível nativamente com Linux ou macOS** sem adaptação.

---

## Funcionalidades

### 1. Menu principal
O jogo inicia com um menu contendo:

- **Jogar**
- **Instruções**
- **Sair**

A navegação é feita com:
- **Seta para cima**
- **Seta para baixo**
- **Enter**

---

### 2. Tela inicial personalizada
Antes da partida iniciar, o jogador pode digitar seu nome.

Depois disso, uma animação é exibida com a cobra “comendo” as letras digitadas.

---

### 3. Jogabilidade
Durante o jogo:

- a cobra se move pelo mapa
- ao comer frutas, ganha pontos
- ao bater na parede ou no próprio corpo, o jogo termina
- o jogador pode controlar a cobra com:
  - **Setas do teclado**
  - **W, A, S, D**

---

### 4. Sistema de pontuação
O jogo possui três tipos de fruta:

#### Fruta normal
- símbolo: `@`
- recompensa: **10 pontos**

#### Fruta especial
- símbolo: `&`
- recompensa: **20 pontos**
- chance: **1%**
- efeito: mostra um **gato secreto** na tela

#### Fruta lendária
- símbolo: `$`
- recompensa: **50 pontos**
- chance: **0,01%**
- efeito: mostra uma **caveira** na tela

---

### 5. Easter eggs
Ao comer frutas raras, o jogo exibe uma tela especial temporária.

#### Gato secreto
Aparece ao comer a fruta especial `&`.

#### Caveira lendária
Aparece ao comer a fruta lendária `$`.

Depois da exibição, o jogo retorna normalmente ao mapa.

---

### 6. HUD
Durante a partida, o HUD mostra:

- **pontuação atual**
- **nome do jogador**
- **recorde da sessão**

---

### 7. Game Over
Ao perder, o jogador vê uma tela de game over com:

- pontuação final
- melhor pontuação
- nome do jogador

Opções disponíveis:
- `[R]` Reiniciar
- `[M]` Menu principal
- `[ESC]` Sair

---

## Estrutura do código

O código está organizado em blocos funcionais.

### 1. Configurações iniciais
Define valores como:

- largura e altura do mapa
- velocidade do jogo
- quantidade de frutas
- probabilidades de frutas especiais
- aparência visual

Exemplo:
```c
#define SPEED 140
#define FRUIT_COUNT 5
#define LEGENDARY_CHANCE 1
#define SPECIAL_CHANCE 100
#define RANDOM_RANGE 10000

2. Hash table de ocupação

O vetor occupancyHash é usado para marcar as posições ocupadas pela cobra no mapa.

Isso ajuda a detectar colisões com o próprio corpo de forma mais rápida.

int occupancyHash[TABLE_SIZE] = {0};

A função abaixo converte coordenadas em um índice linear:

int get_hash_index(int x, int y) {
    return (y * WIDTH) + x;
}
3. Estruturas principais
Node

Representa cada parte do corpo da cobra.

typedef struct Node {
    int x;
    int y;
    struct Node *next;
    struct Node *prev;
} Node;
Fruit

Representa uma fruta no mapa.

typedef struct {
    int x;
    int y;
    int type;
} Fruit;

Tipos:

0 = normal

1 = especial

2 = lendária

4. Controle visual

O projeto usa funções do console para:

mudar cor do texto

mover o cursor

esconder o cursor

Exemplo:

void setColor(int color);
void gotoxy(int x, int y);
void gotoxy_map(int x, int y);
void hideCursor();
5. Sistema de centralização do mapa

O código calcula o tamanho da tela e cria deslocamentos visuais para centralizar o mapa.

Variáveis importantes:

int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int MAP_OFFSET_X;
int MAP_OFFSET_Y;

A função:

void update_console_size();

lê o tamanho atual da janela do console e centraliza o mapa.

6. Sistema da cobra

A cobra é implementada com lista duplamente encadeada.

Funções principais:

insert_head() → adiciona nova cabeça

remove_tail() → remove a cauda

update() → atualiza a lógica do movimento

7. Sistema de colisão

O jogo verifica colisões com:

parede

corpo da cobra

Funções:

collision_wall()

collision_body()

8. Sistema de frutas

As frutas são geradas em posições válidas do mapa, sem sobrepor:

o corpo da cobra

outras frutas

Função:

void generate_fruit(int index);

Ela também decide aleatoriamente o tipo da fruta.

9. Telas do sistema

O jogo possui várias telas auxiliares:

main_menu()

instructions_screen()

start_screen()

game_over_screen()

10. Easter eggs

As funções abaixo exibem telas especiais:

show_cat()

show_skull()

E depois redesenham o jogo com:

redraw_game()

Controles
Durante o jogo

W ou Seta para cima → mover para cima

S ou Seta para baixo → mover para baixo

A ou Seta para esquerda → mover para a esquerda

D ou Seta para direita → mover para a direita

No menu

Seta para cima

Seta para baixo

Enter

Na tela de Game Over

R → reiniciar

M → voltar ao menu

ESC → sair

Como compilar
Usando GCC / MinGW

No terminal:

gcc snake.c -o snake

Se o nome do arquivo for diferente, troque snake.c pelo nome correto.

Exemplo:

gcc main.c -o snake
Como executar

Depois de compilar:

.\snake.exe

ou

.\snake

dependendo da configuração do terminal.

Requisitos

Para compilar no Windows, é necessário ter um compilador C instalado, como:

MinGW

GCC

TDM-GCC

Se estiver usando VS Code, também é recomendado configurar:

terminal integrado

extensão C/C++

compilador no PATH do sistema

Possíveis problemas
1. a.exe does not exist

Isso acontece quando o programa ainda não foi compilado.

Solução:

gcc snake.c -o snake
2. undefined reference

Significa que existe chamada para uma função sem implementação ou com nome diferente do protótipo.

3. implicit declaration of function

A função foi usada antes de ser declarada.

Solução:
adicionar o protótipo da função no topo do arquivo.

4. Flickering

Como o jogo roda no console, algumas transições podem piscar dependendo do terminal e das chamadas para system("cls").

O projeto já foi ajustado para reduzir esse efeito, principalmente no menu.

Pontos fortes do projeto

estrutura organizada em funções

uso de lista encadeada para a cobra

HUD funcional

easter eggs personalizados

sistema de frutas raras

centralização visual do mapa

menu e game over interativos

personalização do nome do jogador

Melhorias futuras

Algumas ideias para evoluir o projeto:

salvar recorde em arquivo

aumentar velocidade com o tempo

adicionar pause

criar obstáculos

implementar fases

adicionar sons

criar skins diferentes para a cobra

fazer versão gráfica com raylib ou SDL2

deixar o jogo mais parecido com o Snake do Google

Exemplo de probabilidades das frutas

No código:

#define LEGENDARY_CHANCE 1
#define SPECIAL_CHANCE 100
#define RANDOM_RANGE 10000

Interpretação:

lendária = 1 / 10000 = 0,01%

especial = 100 / 10000 = 1%

normal = restante

Organização sugerida do projeto
Snakegame/
│
├── snake.c
├── README.md
└── .vscode/
Autor

Projeto desenvolvido e personalizado por Montagnini, com foco em estudo de linguagem C, manipulação de console e lógica de jogos.

Licença

Este projeto pode ser usado para fins de estudo, aprendizado e personalização.


---

Se quiser, eu também posso transformar isso em uma versão **mais profissional de GitHub**, com:
- badges
- seção de screenshots
- instalação formatada
- roadmap
- tópicos mais bonito
