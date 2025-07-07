#include <iostream>
#include <windows.h>
#include <conio.h>
#include <chrono>
#include <thread>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>



#define ALTURA 25
#define LARGURA 25

#define INIMIGOALTURA 3
#define INIMIGOLARGURA 10
#define MAX_PROJETEIS_INIMIGO 10

#define BARRERA_ALTURA 1
#define BARRERA_LARGURA 4
#define VIDA_BARRERA 10


using namespace std;


struct MarcadorTemporario {
    int x, y;
    chrono::steady_clock::time_point momentoCriacao;
};

vector<MarcadorTemporario> marcadoresX;


bool projetilAtivo = false;
int px = -1, py = -1;
DWORD ultimoTiro = 0;

bool projetilAtivoInimigo = false;
int tiroinimigoX = -2,tiroinimigoY = -2;
DWORD tirodoinimigo = GetTickCount();
DWORD tirodoinimigo1 = GetTickCount();

bool deveDescer = false;

bool projetilAtivoJ2 = false;
int pxJ2 = -1, pyJ2 = -1;
DWORD ultimoTiroJ2 = 0;

static int vivo = 1;



struct Projetil {
    int x, y;
    bool ativo;
    DWORD ultimoMovimento;
};
Projetil projeteisInimigos[MAX_PROJETEIS_INIMIGO];



struct Ponto{
    int x, y;
};

struct Barreira {
    Ponto posicao;
    int vida;
    bool ativa;
};

Barreira barreiras[3] = {
    {{5, ALTURA - 7}, VIDA_BARRERA, true},
    {{LARGURA/2 - 1, ALTURA - 7}, VIDA_BARRERA, true},
    {{LARGURA - 8, ALTURA - 7}, VIDA_BARRERA, true}
};

struct StatusPersonagem{
    Ponto pontoPersonagem;
    int vida, score;
    string nome;
};

bool modoAutomatico = true;
bool modoMultiplayer = true;
StatusPersonagem jogador2 = {{LARGURA - 3, ALTURA - 5}, 3, 0, "Jogador2"};


struct StatusInimigo{
    Ponto pontoInimigo;
    bool vivo;
};


void mudarCor(int cor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, cor);
}

void corInf(HANDLE hConsole, int Cursor, int numeroCursor){
    if(Cursor==numeroCursor){
        mudarCor(3); // Ciano
    }else{
        mudarCor(7); // branco
    }
}

void posicao(int x, int y){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD cordenada;
    cordenada.X = x;
    cordenada.Y = y;
    SetConsoleCursorPosition(hConsole, cordenada);
}

//Função para limpar apenas uma area da tela
void limparTela(int x, int y, int largura, int altura) {
    for (int i = 0; i < altura; i++) {
        posicao(x, y + i);
        for (int j = 0; j < largura; j++) {
            cout << " ";
        }
    }
}

int larguraConsole() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void textoCentralizado(const string& texto, int cor = 7) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    int espacos = (csbi.srWindow.Right - texto.length()) / 2;
    SetConsoleTextAttribute(hConsole, cor);
    cout << string(espacos, ' ') << texto << endl;
    SetConsoleTextAttribute(hConsole, 7); // Reset para branco
}

void textoCentralizado(const string& texto, const string& corHex) {
    int cor;
    try {
        cor = stoi(corHex, nullptr, 16); // Converte hex string para int
    } catch (...) {
        cor = 7; // Valor padrão em caso de erro
    }
    textoCentralizado(texto, cor); // Chama a versão numérica
}

//verifica o mapa
void imprimirmapa(int m[ALTURA][LARGURA], int i, int j){
    switch (m[i][j]){
    case 0: cout<<" "; break; //caminho
    case 1: cout<<char(219); break; //parede
    }
}

// verifica os inimigos
bool dentroDoInimigo (int i, int j, StatusInimigo inimigo){
    return( i >= inimigo.pontoInimigo.y &&
    i < inimigo.pontoInimigo.y + INIMIGOALTURA &&
    j >= inimigo.pontoInimigo.x &&
    j < inimigo.pontoInimigo.x + INIMIGOLARGURA);


}

// movimento do disparo
void atualizarProjetil(int m[ALTURA][LARGURA]) {
    if (projetilAtivo && GetTickCount() - ultimoTiro > 100) {
        py--;
        if (py < 0 or m[py][px] == 1) {
            projetilAtivo = false;
        }
        ultimoTiro = GetTickCount();
    }
}

// disparo 1 por vez
void dispararProjetil(StatusPersonagem personagem) {
    if (projetilAtivo == false) {
        projetilAtivo = true;
        py = personagem.pontoPersonagem.y - 1;
        px = personagem.pontoPersonagem.x;
        ultimoTiro = GetTickCount();
        Beep(1000, 50);
    }
}

//atualizasao do tiro do inimigo
void atualizarProjeteisInimigos(int m[ALTURA][LARGURA]) {
    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
        if (projeteisInimigos[i].ativo) {
            if (GetTickCount() - projeteisInimigos[i].ultimoMovimento > 100) {
                projeteisInimigos[i].y++;
                if (projeteisInimigos[i].y >= ALTURA || m[projeteisInimigos[i].y][projeteisInimigos[i].x] == 1) {
                    projeteisInimigos[i].ativo = false;
                }
                projeteisInimigos[i].ultimoMovimento = GetTickCount();
            }
        }
    }
}


//disparo do inimigo
void dispararProjetilinimigo(StatusInimigo inimigo, int i, int j) {
    for (int k = 0; k < MAX_PROJETEIS_INIMIGO; k++) {
        if (!projeteisInimigos[k].ativo) {
            projeteisInimigos[k].ativo = true;
            projeteisInimigos[k].x = inimigo.pontoInimigo.x + j;
            projeteisInimigos[k].y = inimigo.pontoInimigo.y + i + 1;
            projeteisInimigos[k].ultimoMovimento = GetTickCount();
            Beep(600, 75);
            break;
        }
    }
}


void tiroinimigopermitido(StatusInimigo inimigo, int i, int j){
    if(GetTickCount() - tirodoinimigo1 > 1000){
        dispararProjetilinimigo(inimigo, i, j);
        tirodoinimigo1 = GetTickCount();
    }
}

void movimentacaopersonagem(char tecla, StatusPersonagem &personagem, int mapa[ALTURA][LARGURA]){

    int newX = personagem.pontoPersonagem.x;
    int newY = personagem.pontoPersonagem.y;

    switch (tecla) {
        case 'a': case 'A': newX--; break; // Esquerda
        case 'd': case 'D': newX++; break; // Direita
        case 32: dispararProjetil(personagem); break;
        default: break;
    }

    if(newX >= 0 && newX < LARGURA ) {
        if(mapa[newY][newX] != 1 && mapa[newY][newX] !=3) {  // Permite movimento em qualquer tile que não seja parede e tenha chão
            personagem.pontoPersonagem.x = newX;
        }
    }
}

//imprime o mapa
bool isProjeteilInimigoAqui(int x, int y) {
    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
        if (projeteisInimigos[i].ativo && projeteisInimigos[i].x == x && projeteisInimigos[i].y == y) {
            return true;
        }
    }
    return false;
}

void imprimirMapaEInimigo (int m[ALTURA][LARGURA], int i, int j, StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]){

      for (int b = 0; b < 3; b++) {
        if (barreiras[b].ativa &&
            i >= barreiras[b].posicao.y && i < barreiras[b].posicao.y + BARRERA_ALTURA &&
            j >= barreiras[b].posicao.x && j < barreiras[b].posicao.x + BARRERA_LARGURA) {
            mudarCor(5); // Cor marrom
            cout << char(178); // Caractere de barreira
            mudarCor(7);
            return;
        }
    }

     if (dentroDoInimigo(i,j,inimigo) && matinimigo[i - inimigo.pontoInimigo.y][j - inimigo.pontoInimigo.x] == 1) {
        mudarCor(12);
        cout << "W";
        mudarCor(7);
     }else if (dentroDoInimigo(i,j,inimigo) && matinimigo[i - inimigo.pontoInimigo.y][j - inimigo.pontoInimigo.x] == 2){
      mudarCor(14);
      cout << "W";
      mudarCor(7);
    }else if (projetilAtivo && i == py && j == px){
        mudarCor(9);
        cout << "i";
        mudarCor(7);
    }else  if (isProjeteilInimigoAqui(j, i)) {
        mudarCor(4);
        cout << "!";
        mudarCor(7);
    }else if (projetilAtivoJ2 && i == pyJ2 && j == pxJ2) {
        mudarCor(13);
        cout << "^";
        mudarCor(7);
    }
    else{imprimirmapa(m, i, j);

    }
}


bool inimigoAtingiuFim(StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]) {
    for (int i = 0; i < INIMIGOALTURA; i++) {
        for (int j = 0; j < INIMIGOLARGURA; j++) {
            if (matinimigo[i][j] >= 1) {
                int y = inimigo.pontoInimigo.y + i;

                if (y >= ALTURA - 2) {
                    return true;
                }
            }
        }
    }
    return false;
}

//verifica atualiza a morte dos inimigos
void colisaoInimigo(StatusInimigo &inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA], int &velocidadeInimigoY, int &velocidadeInimigoX, StatusPersonagem &personagem) {
    if (projetilAtivo == false){return;}

    int altura = INIMIGOALTURA;
    int largura = INIMIGOLARGURA;

    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            if (py == inimigo.pontoInimigo.y + i && px == inimigo.pontoInimigo.x + j && matinimigo[i][j] >= 1) {
                matinimigo[i][j] --;
                marcadoresX.push_back({px, py, chrono::steady_clock::now()});
                if(matinimigo[i][j] == 0){
                velocidadeInimigoY *= 0.97;
                velocidadeInimigoX *= 0.97;
                personagem.score += 10;
                }
                projetilAtivo = false;
                return;
            }
        }
    }
}

void verificarColisaoComInimigo(StatusPersonagem &personagem, StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]) {
    for (int i = 0; i < INIMIGOALTURA; i++) {
        for (int j = 0; j < INIMIGOLARGURA; j++) {
            if (matinimigo[i][j] >= 1) {
                int inimigoY = inimigo.pontoInimigo.y + i;
                int inimigoX = inimigo.pontoInimigo.x + j;

                if (personagem.pontoPersonagem.y == inimigoY &&
                    personagem.pontoPersonagem.x == inimigoX) {
                    personagem.vida--;
                    return;
                }
            }
        }
    }
}

//faz a movimentação dos inimigos
void movimentacaoInimigo(int tempoDesdeUltimoMovimentoY,
                         int tempoDesdeUltimoMovimentoX,
                         int velocidadeInimigoY,
                         int velocidadeInimigoX,
                         int &direcaoInimigo,
                         chrono::steady_clock::time_point &ultimoMovimentoInimigoY,
                         chrono::steady_clock::time_point &ultimoMovimentoInimigoX,
                         chrono::steady_clock::time_point &agora,
                         StatusInimigo &inimigo,
                         bool &deveDescer,
                         int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]){

if (deveDescer && tempoDesdeUltimoMovimentoY >= velocidadeInimigoY) {
        inimigo.pontoInimigo.y++;
        deveDescer = false;
        ultimoMovimentoInimigoY = agora;
        return;
    }

    if (tempoDesdeUltimoMovimentoX >= velocidadeInimigoX) {
        bool bateuNaBorda = false;
        int minX = LARGURA;
        int maxX = 0;

        // Verifica apenas inimigos vivos nas bordas
        for (int i = 0; i < INIMIGOALTURA; i++) {
            for (int j = 0; j < INIMIGOLARGURA; j++) {
                if (matinimigo[i][j] >= 1) {
                    int xAtual = inimigo.pontoInimigo.x + j;
                    minX = min(minX, xAtual);
                    maxX = max(maxX, xAtual);
                }
            }
        }

        // Verificação ajustada das bordas
        if ((minX + direcaoInimigo <= 0) ||               // Borda esquerda
            (maxX + direcaoInimigo >= LARGURA - 1)) {     // Borda direita
            deveDescer = true;  // ESSA LINHA É CRUCIAL
            direcaoInimigo *= -1;
        } else {
            inimigo.pontoInimigo.x += direcaoInimigo;
        }
        ultimoMovimentoInimigoX = agora;
    }
}

void disparoInimigo(StatusInimigo inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA]) {

    int chanceDoTiro;

    for (int i = 0; i < INIMIGOALTURA; i++) {
        for (int j = 0; j < INIMIGOLARGURA; j++) {
            if (matinimigo[i][j] >= 1) {
                chanceDoTiro = 5 + (i * 3);
                if (rand() % 100 < chanceDoTiro) {
                    tiroinimigopermitido(inimigo, i, j);
                }
            }
        }
    }
}
void verificacaoDeVidaPersonagem(StatusPersonagem &personagem) {
    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
        if (projeteisInimigos[i].ativo &&
            projeteisInimigos[i].x == personagem.pontoPersonagem.x &&
            projeteisInimigos[i].y == personagem.pontoPersonagem.y && vivo == 1) {
            personagem.vida--;
            projeteisInimigos[i].ativo = false;
        }
    }
}


void ordenarRankings(StatusPersonagem scores[], int inicio, int fim) {
    if (inicio >= fim) return;

    int temp = scores[fim].score;
    int i = inicio - 1;

    for (int j = inicio; j < fim; j++) {
        if (scores[j].score >= temp) {
            i++;
            swap(scores[i], scores[j]);
        }
    }
    swap(scores[i + 1], scores[fim]);

    ordenarRankings(scores, inicio, i);
    ordenarRankings(scores, i + 2, fim);
}

void exibirTop10() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const int MAX = 100;
    StatusPersonagem scores[MAX];
    int contador = 0;

    ifstream arquivo("pontuacao.txt");
    while (arquivo >> scores[contador].nome >> scores[contador].score && contador < MAX) {
        contador++;
    }
    arquivo.close();

    ordenarRankings(scores, 0, contador - 1);

    system("cls");

    textoCentralizado("=== TOP 10 ===", 5);

    cout << endl << endl;

    for (int i = 0; i < min(10, contador); i++) {
        string linha = to_string(i+1) + ". " + scores[i].nome + " - " + to_string(scores[i].score);
        textoCentralizado(linha);
    }

    posicao(0, 29);

    system("pause");
    system("cls");
}

void informacoesMenu(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    int cursorAnterior = 0;
    int cursorInfMenuPrincipal=0, cursorInfMenuPrincipalNew;
    char teclaInfMenuPrincipal;
    bool finalizarInfMenuPrincipal = false;

    while(finalizarInfMenuPrincipal==false){
        mudarCor(7); // branco
        posicao (0, 4);
        for(int i=0; i<120; i++){ //imprimir barra inferior
            if(i!=32 && i!=89){
                cout << char(205);
            }else{
                cout << char(202);
            }
        }
        corInf(hConsole, cursorInfMenuPrincipal, 0);
        posicao (13, 2); cout << "Creditos";
        mudarCor(7); // branco

        for(int i=0; i<4; i++){ // imprime barra lateral
            posicao(32, i); cout << char(186);
        }

        corInf(hConsole, cursorInfMenuPrincipal, 1);
        posicao(53, 2); cout << "Como Jogar";
        mudarCor(7); // branco

        for(int i=0; i<4; i++){ // imprime barra lateral
            posicao(89, i); cout << char(186);
        }

        corInf(hConsole, cursorInfMenuPrincipal, 2);
        posicao(97, 2); cout << "Sistema de Pontos";
        mudarCor(7); // branco

        posicao(0, 29); cout << "Pressione Z para voltar";

        // imprime informações quando o cursor estiver em cima de inimigos
       if(cursorInfMenuPrincipal == 0){
            posicao(0, 9); textoCentralizado("Universidade do Vale do Itajai");
            posicao(0, 13); textoCentralizado("Professor: Alex Luciano Roesler Rese");
            posicao(0, 17); textoCentralizado("Alunos: Marlon Ranguet Zucchetti, Gabriel Paranagua, Henrique Gustavo Gonzaga Belli");
        }

        // imprime informações quando o cursor estiver em cima de mapa e itens
        if(cursorInfMenuPrincipal == 1){
            posicao(0, 7);  textoCentralizado("Seu objetivo e destruir todos os inimigos (W)");
            posicao(0, 9);  textoCentralizado("Apertando espaco, voce dispara um projetil");
            posicao(0, 10); textoCentralizado("O jogador so pode disparar um projetil por vez", 8);
            posicao(0, 12); textoCentralizado("Inimigos tambem disparam projeteis");
            posicao(0, 13); textoCentralizado("(cada um causa 1 de dano)", 8);
            posicao(0, 16); textoCentralizado("Perde/Ganha o jogo, se caso o jogador:");
            posicao(0, 17); textoCentralizado("Perder todas as vidas", 8);
            posicao(0, 18); textoCentralizado("/", 8);
            posicao(0, 19); textoCentralizado("Destruir todos os inimigos", 8);
        }

        // imprime informações quando o cursor estiver em cima de score
        if(cursorInfMenuPrincipal == 2){
            posicao(0, 7); textoCentralizado("CADA INIMIGO ELIMINADO, CONCEDE 10 PONTOS");
            posicao(0, 8); textoCentralizado("Nada alem disso diminui ou aumenta os pontos", 8);
        }



        if(_kbhit()){ //Se estiver pressionando uma tecla
        teclaInfMenuPrincipal = getch(); //Recebe o valor da tecla pressionada

        cursorInfMenuPrincipalNew = cursorInfMenuPrincipal;
            switch (teclaInfMenuPrincipal) {
                case 75: case 'a': case 'A': cursorInfMenuPrincipalNew--;break; // Esquerda
                case 77: case 'd': case 'D': cursorInfMenuPrincipalNew++; break; // Direita
                case 'z': case 'Z': // fecha as informações
                    system("cls");
                    finalizarInfMenuPrincipal=true;
                break;
            }

            if(cursorInfMenuPrincipalNew>=0 && cursorInfMenuPrincipalNew<=2){
            cursorInfMenuPrincipal = cursorInfMenuPrincipalNew;
            limparTela(0, 0, 200, 29);
            }

            if(cursorInfMenuPrincipal != cursorAnterior){
                limparTela(37, 7, 55, 21);
                cursorAnterior = cursorInfMenuPrincipal;
            }
        }
    }
}

void creditos(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    system("cls");

    posicao(0, 9); textoCentralizado("Universidade do Vale do Itajai");
    posicao(0, 13); textoCentralizado("Professor: Alex Luciano Roesler Rese");
    posicao(0, 17); textoCentralizado("Alunos: Marlon Ranguet Zucchetti, Gabriel Paranagua, Henrique Gustavo Gonzaga Belli");
    posicao(0, 29);

    system("pause");

}

void selecaoModoDeJogo(bool &desafio){

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int cursorModo =0, cursorModoNew;
    char teclaModo;
    bool finalizarModo = false;

    while(!finalizarModo){
        mudarCor(7); // branco

        posicao(58-3, 11); cout << "Normal";
        posicao(58-3, 13); cout << "Desafio";

        mudarCor(4); //vermelho

        //imprime a seta onde estiver o cursor
        if(cursorModo==0){
            posicao(54-3, 11); cout << "==>";
        }else{
            posicao(54-3, 11); cout << "   ";
        }

        if(cursorModo==1){
            posicao(54-3, 13); cout << "==>";
        }else{
            posicao(54-3, 13);  cout << "   ";
        }

        if (_kbhit()) { //Se estiver pressionando uma tecla
            teclaModo = getch(); //Recebe o valor da tecla pressionada

            cursorModoNew = cursorModo;

            switch (teclaModo) {
                case 72: case 'w': case 'W': cursorModoNew--; break; // Cima
                case 80: case 's': case 'S': cursorModoNew++; break; // Baixo
                case 13: case ' ': //espaço ou enter para confirmar
                if(cursorModo==0){ // limpa a tela e finaliza a função
                    system("cls");
                    desafio = false;
                    finalizarModo = true;
                }else if(cursorModo==1){ // limpa a tela e abre informações
                    system("cls");
                    desafio = true;
                    finalizarModo = true;
                }
                break;
            }

            if(cursorModoNew>=0 && cursorModoNew<=1){
                cursorModo = cursorModoNew;
            }
        }
    }
}


void selecaoDeDificuldade(int &dificuldade, bool &desafio){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int cursorDificuldade=0, cursorDificuldadeNew;
    char teclaDificuldade;
    bool finalizarDificuldade = false;

    while(!finalizarDificuldade){
        mudarCor(7); // branco

        posicao(58-3, 11); cout << "Normal";
        posicao(58-3, 13); cout << "Dificil";
        posicao(58-4, 15); cout << "MuitoDificil";

        mudarCor(4); //vermelho

        //imprime a seta onde estiver o cursor
        if(cursorDificuldade==0){
            posicao(54-3, 11); cout << "==>";
        }else{
            posicao(54-3, 11); cout << "   ";
        }

        if(cursorDificuldade==1){
            posicao(54-3, 13); cout << "==>";
        }else{
            posicao(54-3, 13);  cout << "   ";
        }

        if(cursorDificuldade==2){
            posicao(54-4, 15); cout << "==>";
        }else{
            posicao(54-4, 15); cout << "   ";
        }

        if (_kbhit()) { //Se estiver pressionando uma tecla
            teclaDificuldade = getch(); //Recebe o valor da tecla pressionada

            cursorDificuldadeNew = cursorDificuldade;

            switch (teclaDificuldade) {
                case 72: case 'w': case 'W': cursorDificuldadeNew--; break; // Cima
                case 80: case 's': case 'S': cursorDificuldadeNew++; break; // Baixo
                case 13: case ' ': //espaço ou enter para confirmar
                if(cursorDificuldade==0){ // limpa a tela e finaliza a função
                    system("cls");
                    dificuldade = 1;
                    finalizarDificuldade = true;
                    selecaoModoDeJogo(desafio);
                }else if(cursorDificuldade==1){ // limpa a tela e abre informações
                    system("cls");
                    dificuldade = 2;
                    finalizarDificuldade = true;
                    selecaoModoDeJogo(desafio);
                }else if(cursorDificuldade==2){// limpa a tela e abre as 10 melhores pontuacoes
                    system("cls");
                    dificuldade = 3;
                    finalizarDificuldade = true;
                    selecaoModoDeJogo(desafio);
                }
                break;
            }

            if(cursorDificuldadeNew>=0 && cursorDificuldadeNew<=2){
                cursorDificuldade = cursorDificuldadeNew;
            }
        }
    }
}
void menuConfiguracoes() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int cursor = 0, novoCursor;
    char tecla;
    bool sair = false;

    while (!sair) {
        system("cls");
        mudarCor(7);

        textoCentralizado("CONFIGURACOES", 11);
        cout << "\n\n";

        for (int i = 0; i < 3; i++) {
            if (cursor == i) mudarCor(3);
            else mudarCor(7);

            posicao(50, 6 + i * 2);
            if (i == 0) cout << "[Modo Automatico]  " << (modoAutomatico ? "Ligado" : "Desligado");
            if (i == 1) cout << "[Modo Multiplayer] " << (modoMultiplayer ? "Ligado" : "Desligado");
            if (i == 2) cout << "Voltar";
        }

        tecla = getch();

        novoCursor = cursor;
        switch (tecla) {
            case 72: case 'w': case 'W': novoCursor--; break; // Cima
            case 80: case 's': case 'S': novoCursor++; break; // Baixo
            case 13: case ' ': // Enter ou espaço
                if (cursor == 0) modoAutomatico = !modoAutomatico;
                else if (cursor == 1) modoMultiplayer = !modoMultiplayer;
                else if (cursor == 2) sair = true;
                break;
        }

        if (novoCursor >= 0 && novoCursor <= 2) {
            cursor = novoCursor;
        }
    }

    system("cls");
}

void printarMenuInicial(int &dificuldade, bool &desafio){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int cursorMenuPrincipal=0, cursorMenuPrincipalNew;
    char teclaMenuPrincipal;
    bool finalizarMenuPrincipal = false;

    while(!finalizarMenuPrincipal){
        mudarCor(7); // branco

        posicao(58, 11); cout << "Jogar";
        posicao(58-3, 13); cout << "Informacoes";
        posicao(58-4, 15); cout << "Ranking top 10";
        posicao(58-4, 17); cout << "Configuracoes";
        posicao(58, 19); cout << "Sair";

        mudarCor(4); //vermelho

        //imprime a seta onde estiver o cursor
        if(cursorMenuPrincipal==0){
            posicao(54, 11); cout << "==>";
        }else{
            posicao(54, 11); cout << "   ";
        }

        if(cursorMenuPrincipal==1){
            posicao(54-3, 13); cout << "==>";
        }else{
            posicao(54-3, 13);  cout << "   ";
        }

        if(cursorMenuPrincipal==2){
            posicao(54-4, 15); cout << "==>";
        }else{
            posicao(54-4, 15); cout << "   ";
        }
        if(cursorMenuPrincipal==4){
            posicao(54, 19); cout << "==>";
        }else{
            posicao(54, 19); cout << "   ";
        }
        if(cursorMenuPrincipal==3){
            posicao(54-5, 17); cout << "==>";
        } else {
            posicao(54-5, 17); cout << "   ";
        }



        if (_kbhit()) { //Se estiver pressionando uma tecla
            teclaMenuPrincipal = getch(); //Recebe o valor da tecla pressionada

            cursorMenuPrincipalNew = cursorMenuPrincipal;

            switch (teclaMenuPrincipal) {
                case 72: case 'w': case 'W': cursorMenuPrincipalNew--; break; // Cima
                case 80: case 's': case 'S': cursorMenuPrincipalNew++; break; // Baixo
                case 13: case ' ': //espaço ou enter para confirmar
                if(cursorMenuPrincipal==0){ // limpa a tela e finaliza a função
                    system("cls");
                    finalizarMenuPrincipal = true;
                    selecaoDeDificuldade(dificuldade, desafio);
                }else if(cursorMenuPrincipal==1){ // limpa a tela e abre informações
                    system("cls");
                    informacoesMenu();
                }else if(cursorMenuPrincipal==2){// limpa a tela e abre as 10 melhores pontuacoes
                    exibirTop10();
                    system("cls");
                }else if(cursorMenuPrincipal==4){ // finaliza o programa
                    exit(0);
                }else if(cursorMenuPrincipal==3){
                    system("cls");
                    menuConfiguracoes();
                }

                break;
            }

            if(cursorMenuPrincipalNew>=0 && cursorMenuPrincipalNew<=4){
                cursorMenuPrincipal = cursorMenuPrincipalNew;
            }
        }
    }
}

void cronometro(auto &inicio, int &minuto = 0) {
    posicao(0, 27);
    // Espera por um segundo antes de mostrar o tempo
    auto agora = chrono::steady_clock::now();
    auto duracao = chrono::duration_cast<chrono::seconds>(agora - inicio);

    if(duracao.count() == 60) {
        inicio = chrono::steady_clock::now();
        minuto++;
    }
    cout << "Tempo: " << minuto << ":" ;

    if(duracao.count() < 10) {
        cout << "0";
    }
    cout << duracao.count() << "                                                                 " << endl;
}

void salvarPontuacao(StatusPersonagem personagem) {
    ofstream arquivo("pontuacao.txt", ios::app);
    if (arquivo.is_open()) {
        arquivo << personagem.nome << ": " << personagem.score << endl;
        arquivo.close();
    }
}

void fimDeJogo(StatusPersonagem& personagem, bool venceu) {
    system("cls");
    cout << "Fim de jogo! " << (venceu ? "Voce venceu!" : "Voce morreu!") << endl;
    cout << "Sua pontuacao final foi de: " << personagem.score << " pts." << endl;

    cout << "Digite seu nome: ";
    cin >> personagem.nome;

    if (personagem.nome == "apagarArquivo") {
        ofstream arquivo("pontuacao.txt", ios::trunc);
        arquivo.close();
        cout << "Pontuacoes resetadas!" << endl;
    } else if (personagem.nome == "rodadaTeste"){
        cout << "Sua pontuacao nao foi salva!" << endl;
    }else {
        salvarPontuacao(personagem);
        cout << "Pontuacao salva!" << endl;
    }
}

void colisaoProjetilVsProjetil() {
    if (!projetilAtivo) return;

    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
        if (projeteisInimigos[i].ativo && projeteisInimigos[i].x == px && projeteisInimigos[i].y == py) {

            // Desativa ambos os projéteis
            marcadoresX.push_back({px, py, chrono::steady_clock::now()});
            projeteisInimigos[i].ativo = false;
            projetilAtivo = false;
            Beep(300, 100);
            return;
        }
    }
}

void setInimigo(int matinimigo[INIMIGOALTURA][INIMIGOLARGURA], int dificuldade){

    if(dificuldade == 1){
        for(int i = 0; i<INIMIGOALTURA; i++){
            for(int j = 0; j<INIMIGOLARGURA; j++){
                matinimigo[i][j] = 1;
            }
        }
    }

    if(dificuldade == 2){
        for(int i = 0; i<INIMIGOALTURA; i++){
            for(int j = 0; j<INIMIGOLARGURA; j++){
                int valorAleatorio = rand() % 100;
                if(valorAleatorio <= 50){
                matinimigo[i][j] = 2;
                }else {matinimigo[i][j] = 1;}
            }
        }
    }

    if(dificuldade == 3){
        for(int i = 0; i<INIMIGOALTURA; i++){
            for(int j = 0; j<INIMIGOLARGURA; j++){
                int valorAleatorio = rand() % 100;
                if(valorAleatorio <= 70){
                matinimigo[i][j] = 2;
                }else {matinimigo[i][j] = 1;}
            }
        }
    }
}

void resetDesafio (StatusInimigo &inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA],int &velocidadeInimigoY,
                                                                                         int &velocidadeInimigoX,
                                                                                         int dificuldade){

    inimigo.pontoInimigo.y = 2;
    inimigo.pontoInimigo.x = 2;

    setInimigo(matinimigo, dificuldade);

        velocidadeInimigoY *= 1.7;
        velocidadeInimigoX *= 1.7;

        if (velocidadeInimigoY < 200) {velocidadeInimigoY = 200;}
        if (velocidadeInimigoX < 100) {velocidadeInimigoX = 100;}

}
void dispararProjetilJogador2(StatusPersonagem personagem) {
    if (!projetilAtivoJ2) {
        projetilAtivoJ2 = true;
        pyJ2 = personagem.pontoPersonagem.y - 1;
        pxJ2 = personagem.pontoPersonagem.x;
        ultimoTiroJ2 = GetTickCount();
        Beep(1200, 50);
    }
}
void movimentacaoJogador2(char tecla, StatusPersonagem &jogador2, int mapa[ALTURA][LARGURA]) {
    int newX = jogador2.pontoPersonagem.x;

    switch (tecla) {
        case 75: newX--; break; // ←
        case 77: newX++; break; // →
        case 72: dispararProjetilJogador2(jogador2); break;
        default: break;
    }

    if(newX >= 0 && newX < LARGURA && mapa[jogador2.pontoPersonagem.y][newX] != 1) {
        jogador2.pontoPersonagem.x = newX;
    }
}

void atualizarProjetilJogador2(int m[ALTURA][LARGURA]) {
    if (projetilAtivoJ2 && GetTickCount() - ultimoTiroJ2 > 100) {
        pyJ2--;
        if (pyJ2 < 0 || m[pyJ2][pxJ2] == 1) {
            projetilAtivoJ2 = false;
        }
        ultimoTiroJ2 = GetTickCount();
    }
}
void colisaoInimigoJogador2(StatusInimigo &inimigo, int matinimigo[INIMIGOALTURA][INIMIGOLARGURA], int &velocidadeInimigoY, int &velocidadeInimigoX, StatusPersonagem &jogador2) {
    if (projetilAtivoJ2 == false) return;

    int altura = INIMIGOALTURA;
    int largura = INIMIGOLARGURA;

    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            if (pyJ2 == inimigo.pontoInimigo.y + i && pxJ2 == inimigo.pontoInimigo.x + j && matinimigo[i][j] >= 1) {
                matinimigo[i][j]--;
                if (matinimigo[i][j] == 0) {
                    velocidadeInimigoY *= 0.97;
                    velocidadeInimigoX *= 0.97;
                    jogador2.score += 10;
                }
                projetilAtivoJ2 = false;
                return;
            }
        }
    }
}
void verificacaoDeVidaJogador2(StatusPersonagem &jogador2) {
    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
        if (projeteisInimigos[i].ativo &&
            projeteisInimigos[i].x == jogador2.pontoPersonagem.x &&
            projeteisInimigos[i].y == jogador2.pontoPersonagem.y) {
            jogador2.vida--;
            projeteisInimigos[i].ativo = false;
        }
    }
}

void verificarColisaoBarreiras() {
    // Verifica colisão com projétil do jogador 1
    if (projetilAtivo) {
        for (int b = 0; b < 3; b++) {
            if (barreiras[b].ativa &&
                py >= barreiras[b].posicao.y && py < barreiras[b].posicao.y + BARRERA_ALTURA &&
                px >= barreiras[b].posicao.x && px < barreiras[b].posicao.x + BARRERA_LARGURA) {
                barreiras[b].vida--;
                projetilAtivo = false;
                if (barreiras[b].vida <= 0) {
                    barreiras[b].ativa = false;
                }
                return;
            }
        }
    }

    // Verifica colisão com projétil do jogador 2 (se multiplayer)
    if (modoMultiplayer && projetilAtivoJ2) {
        for (int b = 0; b < 3; b++) {
            if (barreiras[b].ativa &&
                pyJ2 >= barreiras[b].posicao.y && pyJ2 < barreiras[b].posicao.y + BARRERA_ALTURA &&
                pxJ2 >= barreiras[b].posicao.x && pxJ2 < barreiras[b].posicao.x + BARRERA_LARGURA) {
                barreiras[b].vida--;
                projetilAtivoJ2 = false;
                if (barreiras[b].vida <= 0) {
                    barreiras[b].ativa = false;
                }
                return;
            }
        }
    }

    // Verifica colisão com projéteis inimigos
    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
        if (projeteisInimigos[i].ativo) {
            for (int b = 0; b < 3; b++) {
                if (barreiras[b].ativa &&
                    projeteisInimigos[i].y >= barreiras[b].posicao.y &&
                    projeteisInimigos[i].y < barreiras[b].posicao.y + BARRERA_ALTURA &&
                    projeteisInimigos[i].x >= barreiras[b].posicao.x &&
                    projeteisInimigos[i].x < barreiras[b].posicao.x + BARRERA_LARGURA) {
                    barreiras[b].vida--;
                    projeteisInimigos[i].ativo = false;
                    if (barreiras[b].vida <= 0) {
                        barreiras[b].ativa = false;
                    }
                    break;
                }
            }
        }
    }
}






int main()
{
    ///ALERTA: NAO MODIFICAR O TRECHO DE CODIGO, A SEGUIR.
        //INICIO: COMANDOS PARA QUE O CURSOR NAO FIQUE PISCANDO NA TELA
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO     cursorInfo;
        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = false; // set the cursor visibility
        SetConsoleCursorInfo(out, &cursorInfo);
        //FIM: COMANDOS PARA QUE O CURSOR NAO FIQUE PISCANDO NA TELA
        //INICIO: COMANDOS PARA REPOSICIONAR O CURSOR NO INICIO DA TELA
        short int CX=0, CY=0;
        COORD coord;
        coord.X = CX;
        coord.Y = CY;
        //FIM: COMANDOS PARA REPOSICIONAR O CURSOR NO INICIO DA TELA
    ///ALERTA: NAO MODIFICAR O TRECHO DE CODIGO, ACIMA.

    auto ultimoMovimentoInimigoY = chrono::steady_clock::now();
    auto ultimoMovimentoInimigoX = chrono::steady_clock::now();
    srand(time(NULL));

    int m[ALTURA][LARGURA] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

    StatusPersonagem personagem {{2, 20}, 3}; // Variavel do personagem
    if (modoMultiplayer) {
        jogador2.vida = 3;
        jogador2.score = 0;
        jogador2.pontoPersonagem.x = LARGURA - 3;
        jogador2.pontoPersonagem.y = ALTURA - 4;
    }

    StatusInimigo inimigo {2, 2};
    char tecla;

    int matinimigo [INIMIGOALTURA][INIMIGOLARGURA];

    int velocidadeInimigoY = 3000;
    int velocidadeInimigoX = 1750;
    int direcao = -1;

    int ultimaVidaDesenhada = -1;
    int ultimaVidaDesenhada2 = -1;
    int ultimoScoreDesenhado = -1;

    int minuto = 0;
    int dificuldade = 0;
    bool desafio = false;
    printarMenuInicial(dificuldade, desafio);

    auto inicio = chrono::steady_clock::now();

    for (int i = 0; i < MAX_PROJETEIS_INIMIGO; i++) {
    projeteisInimigos[i].ativo = false;
    }
     setInimigo(matinimigo, dificuldade);

    while(true){

        cronometro(inicio, minuto);

        auto agora = chrono::steady_clock::now();
        auto tempopassado = chrono::duration_cast<chrono::milliseconds>(agora - inicio).count();
        auto tempoDesdeUltimoMovimentoY = chrono::duration_cast<chrono::milliseconds>(agora - ultimoMovimentoInimigoY).count();
        auto tempoDesdeUltimoMovimentoX = chrono::duration_cast<chrono::milliseconds>(agora - ultimoMovimentoInimigoX).count();

        movimentacaoInimigo(tempoDesdeUltimoMovimentoY,tempoDesdeUltimoMovimentoX,velocidadeInimigoY,velocidadeInimigoX,direcao,ultimoMovimentoInimigoY,ultimoMovimentoInimigoX, agora, inimigo, deveDescer, matinimigo);

        ///Posiciona a escrita no iicio do console
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        atualizarProjetil(m);
        colisaoInimigo(inimigo, matinimigo, velocidadeInimigoY, velocidadeInimigoX, personagem);
        colisaoProjetilVsProjetil();
        verificarColisaoComInimigo(personagem, inimigo, matinimigo);
        disparoInimigo(inimigo, matinimigo);
        atualizarProjeteisInimigos(m);
        verificacaoDeVidaPersonagem(personagem);
        colisaoInimigoJogador2(inimigo, matinimigo, velocidadeInimigoY, velocidadeInimigoX, jogador2);
        verificacaoDeVidaJogador2(jogador2);
        atualizarProjetilJogador2(m);
        verificarColisaoBarreiras();



        if(jogador2.vida == 0){
            modoMultiplayer = false;
        }
        if(personagem.vida == 0){
            vivo = 0;
        }

        marcadoresX.erase(remove_if(marcadoresX.begin(), marcadoresX.end(), [](MarcadorTemporario& m) {
            return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - m.momentoCriacao).count() > 1000;
        }), marcadoresX.end());


        for(int i=0;i<ALTURA;i++){
            for(int j=0;j<LARGURA;j++){
                bool xDesenhado = false;
for (const auto& marcador : marcadoresX) {
    if (i == marcador.y && j == marcador.x) {
        mudarCor(7); // vermelho
        cout << "X";
        mudarCor(7);
        xDesenhado = true;
        break;
    }
}

if (xDesenhado) {
    // já desenhou o X, então não desenha mais nada
}
else if(i == personagem.pontoPersonagem.y && j == personagem.pontoPersonagem.x && vivo == 1){
    mudarCor(2);
    cout<< "A";
    mudarCor(7);
}
else if (modoMultiplayer && i == jogador2.pontoPersonagem.y && j == jogador2.pontoPersonagem.x){
    mudarCor(5);
    cout << "B";
    mudarCor(7);
}
else {
    imprimirMapaEInimigo(m,i,j,inimigo,matinimigo);
}

            }
            cout<<"\n";
        } //fim for mapa

        ///executa os movimentos
         if (_kbhit()) {
    tecla = getch();
    movimentacaopersonagem(tecla, personagem, m);
    if (modoMultiplayer) {
        movimentacaoJogador2(tecla, jogador2, m);
    }
} else if (modoAutomatico) {
    // Encontrar o inimigo vivo mais à esquerda
    int alvoX = -1;
    for (int j = 0; j < INIMIGOLARGURA; j++) {
        for (int i = 0; i < INIMIGOALTURA; i++) {
            if (matinimigo[i][j] >= 1) {
                alvoX = inimigo.pontoInimigo.x + j;
                break;
            }
        }
        if (alvoX != -1) break;
    }

    // Movimento automático
    if (alvoX != -1) {
        if (personagem.pontoPersonagem.x < alvoX) {
            movimentacaopersonagem('d', personagem, m);
        } else if (personagem.pontoPersonagem.x > alvoX) {
            movimentacaopersonagem('a', personagem, m);
        }

        // Atirar automaticamente se não tiver projétil
        if (!projetilAtivo) {
            movimentacaopersonagem(32, personagem, m);
        }
    }
}



        this_thread::sleep_until(inicio + chrono::milliseconds(20));

        // Vida do jogador 1
    if(personagem.vida != ultimaVidaDesenhada){
        posicao(0, 25);
        cout << "               ";
        posicao(0, 25);
        cout << "vida1:";
        mudarCor(12);
        for(int i = 0; i < personagem.vida; i++){
            cout << char(219);
        }
        mudarCor(7);
        ultimaVidaDesenhada = personagem.vida;
    }

    // Vida do jogador 2
    if(jogador2.vida != ultimaVidaDesenhada2){
    posicao(0, 26);
    cout << "               ";
    posicao(0, 26);
    cout << "vida2:";
    mudarCor(5);
    for (int i = 0; i < jogador2.vida; i++) {
        cout << char(219);
    }
    mudarCor(7);
    ultimaVidaDesenhada2 = jogador2.vida;
    }


        if(personagem.score != ultimoScoreDesenhado){
            posicao(20, 25);
            cout << "                ";
            posicao(20, 25);
            cout << "score: ";
            mudarCor(14);
            cout << personagem.score;
            mudarCor(7);

            ultimoScoreDesenhado = personagem.score;
        }


        bool venceu = true;
        for (int i = 0; i < INIMIGOALTURA; i++) {
            for (int j = 0; j < INIMIGOLARGURA; j++) {
                if (matinimigo[i][j] >= 1) {
                    venceu = false;
                    break;
                }
            }
            if (!venceu) break;
        }

        if (venceu && desafio){
            resetDesafio(inimigo, matinimigo, velocidadeInimigoY, velocidadeInimigoX, dificuldade);
        }

        if(inimigoAtingiuFim(inimigo, matinimigo) || venceu && !desafio){
            fimDeJogo(personagem, true);
            return 0;
        } else if(personagem.vida <= 0 && jogador2.vida <= 0){
            fimDeJogo(personagem, false);
            return 0;
        }

    } //fim do laco do jogo

    return 0;
} //fim main
