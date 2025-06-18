#include <iostream>
#include <windows.h>
#include <conio.h>
#define TAM 25

using namespace std;

int mapa[TAM][TAM] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}

};
// posição do personagem
int x = 23, y = 23;

// tiro
bool projetilAtivo = false;
int px = -1, py = -1;

bool projetilAtivoInimigo = false;
int tiroinimihoX = -2,tiroinimihoY = -2;


//variaveis de temopo
DWORD ultimoTempo = GetTickCount();
DWORD ultimoTempoInimigo = GetTickCount();
DWORD ultimoTempocronometro = GetTickCount();
DWORD fimdejogotempo = GetTickCount();
DWORD tirodoinimigo = GetTickCount();
DWORD tirodoinimigo1 = GetTickCount();



//inimigo
bool inimigoVivo = true;
int ix = 1, iy = 1;
int velocidadeinimigo = 1000;

COORD coord = {0, 0};

//Nao mexer
void inicializarConsole() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);
}
//Nao mexer

// printador do mapa, personagem e projétil
void desenharMapa() {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (i == x && j == y) {
                cout << char(36); // personagem
            } else if (projetilAtivo == true && i == px && j == py) {
                cout << "^"; // tiro
            } else if(inimigoVivo == true && i == ix && j == iy){
                cout << "W";
            } else if(projetilAtivoInimigo == true && i == tiroinimihoX && j == tiroinimihoY){
                cout << "i";
            }
            else {
                switch (mapa[i][j]) {
                    case 0: cout << " "; break;
                    case 1: cout << char(219); break;
                }
            }
        }
        cout << "\n";
    }
}

// movimento do disparo
void atualizarProjetil() {
    if (projetilAtivo == true && GetTickCount() - ultimoTempo > 100/*a velocidade da bala*/) {
        px--;
        if (px < 0 or mapa[px][py] == 1/*onde a bala vai se desativada*/) {
            projetilAtivo = false;
        }
        ultimoTempo = GetTickCount();
    }
}



//atualizasao do tiro do inimigo
void atualizarProjetilInimiho() {
    if (projetilAtivoInimigo == true && GetTickCount() - tirodoinimigo > 100/*a velocidade da bala*/) {
        tiroinimihoX++;
        if (tiroinimihoX < 0 or mapa[tiroinimihoX][tiroinimihoY] == 1/*onde a bala vai se desativada*/) {
            projetilAtivoInimigo = false;
        }
        tirodoinimigo = GetTickCount();
    }
}
//disparo do inimigo
void dispararProjetilinimigo() {
    if (projetilAtivoInimigo == false) {
        projetilAtivoInimigo  = true;
        tiroinimihoX = ix + 1;
        tiroinimihoY = iy;
        tirodoinimigo = GetTickCount();
    }
}
void tiroinimigopermitido(){
    if(GetTickCount() - tirodoinimigo1 > 1000){
        dispararProjetilinimigo();
        tirodoinimigo1 = GetTickCount();
    }
}





// disparo 1 por vez
void dispararProjetil() {
    if (projetilAtivo == false) {
        projetilAtivo = true;
        px = x - 1;
        py = y;
        ultimoTempo = GetTickCount();
    }
}

// mover e atirar
void tecla() {
    if (_kbhit()) {
        char tecla = getch();
        switch (tecla) {
            case 75: case 'a': if (mapa[x][y - 1] != 1) y--; break;
            case 77: case 'd': if (mapa[x][y + 1] != 1) y++; break;
            case 32: dispararProjetil(); break;
        }
    }
}
//tenta encaixar em outro lugar, nao precisa ser uma funçao sozinha, colocar junto de outra coisa
void inimigovivooumorto(){
    if(px == ix && py == iy){
        inimigoVivo = false;
        projetilAtivo = false;
    }
}
void inimigomovimento(){
    static int direcao = 1; //1 direita e -1 esquerda

    //inimigo tempo de movimento
    if (inimigoVivo == true && GetTickCount() - ultimoTempoInimigo > velocidadeinimigo/*velocidade do inimigo*/){
        //verifica se tem parede
        if (mapa[ix][iy + direcao] == 1){
            //vai pra baixo
            ix++;
            //muda a direcao
            direcao = -direcao;
        //se nao so vai pro lado
        } else {
            iy += direcao;
        }
        if(ix == x && iy == y){
            cout << "MORREU";
        }

        ultimoTempoInimigo = GetTickCount();
    }
}
void cronometro(){
    static int segunda = 0, minutos = 0, horas = 0;
    int disparoinimigoseundo = 0;

    if(segunda <= 60 && GetTickCount() - ultimoTempocronometro > 1000){
        ultimoTempocronometro = GetTickCount();
        segunda++;
    }
    if(segunda == 60){
        minutos++;
        segunda = 0;
    }
    if(minutos == 60){
        horas++;
        minutos = 0;
    }

    //serve pra aumenta a velocidade do inimigo baseado no tempo, ao inves de usar segundos usar a quantidade de inimigos mortos e tirar do cronometro e bota em uma funçao
    if(segunda == 1){
        velocidadeinimigo = 500;
    }
    if(segunda == 3){
        velocidadeinimigo = 200;
    }
    if(segunda == 7){
        velocidadeinimigo = 50;
    }


    //cout do cronometro

    cout << "tempo: ";
    if(horas > 9){
        cout << horas << " ";
    }else{
        cout << "0" << horas << " ";
    }
    if(minutos > 9){
        cout << minutos << " ";
    }else{
        cout << "0" << minutos << " ";
    }
    if(segunda > 9){
        cout << segunda << " " << endl;
    }else{
        cout << "0" << segunda << " " << endl;
    }


    //pra ve a velocidade do inimigo
    cout <<"velocidade do inimigo: " << velocidadeinimigo << "              ";


}


int main() {
    inicializarConsole();

    while (true) {
        atualizarProjetil();
        tiroinimigopermitido();
        atualizarProjetilInimiho();
        desenharMapa();
        tecla();
        inimigomovimento();
        inimigovivooumorto();
        cronometro();

        //verifica se jogador foi atigindo, mudar de lugar pra uma funçao
        if(ix == x && iy == y){
            return 0;
        }
        if(inimigoVivo == false){
            cout << "Vitoria!!!";
            return 0;
        }
        if(tiroinimihoX == x && tiroinimihoY == y){
            cout << "voce morreu";
            return 0;
        }
        //verificador de tempo, cada 1000 e um segundo, assim quando passar 50 segundos acaba o jogo.
         /*if (GetTickCount() - fimdejogotempo > 1000){
             return 0;
         }*/

    }

    return 0;
}
