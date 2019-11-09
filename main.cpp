#include <iostream>
#include <vector>
#include <curses.h>
#include <unistd.h>
#include <sstream>


// 0 <= x0 < nTerminalWidth;
// 0 <= y0 < nTerminalHeight;
int nTerminalHeight, nTerminalWidth;

uint64_t framecount = 0;
float score = 0.0;
bool isGameOver = false;

int nPlayerX = 32;
float nPlayerY = 16;

int MAX_PIPE_HEIGHT;
int PADDING = 2;
int WINDOW_SIZE = 8;
int MAX_N_PIPES = 20;
int PIPE_SEPARATION = 12;

wchar_t *screenBuffer;

struct Pipe{
    int nPosX;
    int nTop;
    int nBottom;
};

std::vector<Pipe> vPipe;


void update();
void updatePlayer();
void updatePipes();
void updateScreenBuffer();
void updateHUD();
Pipe* getNewPipe();
int getMaxPipeX();
void checkGameOver();


int main() {

    setlocale(LC_ALL, "");

    initscr();
    noecho();                           // no echo to stdout
    cbreak();                           // no buffering of stdin
    nodelay(stdscr, TRUE);              // non blocking getch

    getmaxyx(stdscr, nTerminalHeight, nTerminalWidth);
    screenBuffer = new wchar_t[nTerminalHeight * nTerminalWidth];
    MAX_PIPE_HEIGHT = nTerminalHeight / 3;


    while (true) {
        char ch;
        if ((ch = getch()) == ERR) {
            // check if game over
            checkGameOver();

            // update scene
            update();

            // render
            for (int y=0; y<nTerminalHeight; y++) {
                for (int x=0; x<nTerminalWidth; x++) {
                    move(y, x);
                    addch(screenBuffer[y + nTerminalHeight * x]);
                }
            }

            refresh();
            framecount++;
            usleep(100 * 1000);
        }
        else {
            // player pressed spacebar
            if (ch == ' ') {
                nPlayerY -= 2;
                if (nPlayerY < 0)
                    nPlayerY = 0;
            }
        }
    }

    return 0;
}

void update() {
    if (!isGameOver) {
        // update player
        updatePlayer();

        // update pipes
        updatePipes();

        // update screen buffer
        updateScreenBuffer();

        // update HUD
        updateHUD();
    }
    else {
        std::string text;
        if (framecount % 2 == 0) {
            text = "Game Over";
        }
        else {
            text = "         ";
        }


        for(int i=0; i<text.size(); i++) {
            screenBuffer[int(nTerminalHeight/2 + nTerminalHeight * (i + (nTerminalWidth - text.size())/2))] = text[i];
        }
    }
}

void checkGameOver() {
    for (auto p: vPipe) {
        if (nPlayerX == p.nPosX) {
            if (int(nPlayerY) < p.nTop || int(nPlayerY) > nTerminalHeight - p.nBottom) {
                isGameOver = true;
                break;
            }
        }
    }
}

void updatePlayer() {
    nPlayerY += 0.4;
    if (nPlayerY > nTerminalHeight) {
        nPlayerY = nTerminalHeight - 0.3;
    }
}

void updatePipes() {
    // update vPipe
    while (vPipe.size() < MAX_N_PIPES) {
        Pipe *p = getNewPipe();
        vPipe.push_back(*p);
    }

    // remove exhausted pipes
    std::vector<Pipe> tvPipe;
    for (auto p=vPipe.begin(); p != vPipe.end(); p++) {
        p->nPosX -= 1;
        if (p->nPosX > 0) {
           tvPipe.push_back(*p);
        }
    }
    vPipe = tvPipe;
}

void updateScreenBuffer() {
    for (int y=0; y<nTerminalHeight; y++) {
        for (int x=0; x<nTerminalWidth; x++) {
            if (y == int(nPlayerY) && x == nPlayerX) {
                screenBuffer[y + nTerminalHeight * x] = 'P';
            }
            else {
                bool isPipe = false;

                for (auto p: vPipe) {
                    if (x == p.nPosX && (y < p.nTop || (y > nTerminalHeight - p.nBottom))) {
                        isPipe = true;
                        screenBuffer[y + nTerminalHeight * x] = 'H';
                        break;
                    }
                }

                if (!isPipe)
                    screenBuffer[y + nTerminalHeight * x] = ' ';
            }
        }
    }
}

void updateHUD() {
    score += 0.1;

    std::ostringstream stringStream;
    stringStream << "Score: " << int(score);
    std::string sScore = stringStream.str();

    for(int i=0; i<sScore.size(); i++) {
        screenBuffer[nTerminalHeight * (1+i)] = sScore[i];
    }
}

int getMaxPipeX() {
    int maxPosX = 0;
    for(auto p: vPipe) {
        maxPosX = std::max(maxPosX, p.nPosX);

    }

    return maxPosX;
}

Pipe* getNewPipe() {
    Pipe *p = new Pipe;
    p->nTop = std::rand() % MAX_PIPE_HEIGHT + PADDING;
    p->nBottom = nTerminalHeight - (p->nTop + WINDOW_SIZE);
    p->nPosX = getMaxPipeX() + PIPE_SEPARATION;

    if (p->nPosX < nTerminalWidth) {
        p->nPosX = nTerminalWidth + std::rand() % nTerminalWidth;
    }

    // fprintf(stderr,
    //         "getNewPipe::{nTop: %d, nBottom: %d, nPosX: %d}\n",
    //         p->nTop,
    //         p->nBottom,
    //         p->nPosX);

    return p;
}
