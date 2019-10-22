#include <cstdio>
#include <cstdlib>
#include <deque>
#include <algorithm>
#include <signal.h>
#include <sys/wait.h>
#include <vector>
#include "conio.h"
#include "instrums.h"
#include "player.h"

#ifdef USECHARZ
#define caseprintf printf
#else
static void caseprintf(const char* x) {
  printf("%*s", strlen(x), "");
}
#endif

static void CPos(int i, int j) {
    COORD cPos;
    cPos.X = j;
    cPos.Y = i;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cPos);
}
static void setcolor(unsigned short int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}


static bool exitflag, menupending;

static const char* keyconf="\tq2w3er5t6y7ui9o0p[=]azsxcfvgbnjmk,l./'";
int instr, dura=4, menupoz, velocity=0x7f;

static std::deque<std::pair<int,int> > pids;

void addpid(int child_pid, int pitch) {
  pids.push_back(std::make_pair(child_pid, pitch));
}

static std::vector<bool> colmap({1,1,0,1,0,1,1,0,1,0,1,0});

static void redraw() {
#ifdef __WIN32
  system("cls");
#else
  fprintf(stderr, "\x1b[H\x1b[J");
#endif
  int i;
  // keyboard:
  //  _ _ _ _ _ _ _ _ _ _ _ _
  // | |2| |4| | |7| |9|1|1|1|
  // |1|_|3|_|5|6|_|8|_|0|1|2|
  // |  |   |  |  |   |   |  |
  // |__|___|__|__|___|___|__|
  for (i=1; i<37; i++) {
    int col;
    setcolor(15);
    col = colmap[i%12];
    CPos(1, i*2);
    caseprintf("_");
    CPos(2, i*2-1);
    caseprintf("|");
    setcolor(9 | (col*240));
    for (std::deque<std::pair<int,int> >::iterator it=pids.begin(); it!=pids.end(); it++)
      if (it->second == i-1) {
        setcolor(144 | (col*15));
        break;
      }
    printf("%c", keyconf[i]);
    setcolor(15);
    caseprintf("|");
    CPos(3, i*2-1);
    caseprintf("|");
    if (col) {
      setcolor(240);
      printf(" ");
      setcolor(15);
    }
    else
      caseprintf("_");
    caseprintf("|");
    CPos(4, i*2-1);
    setcolor(240);
    switch (i%12) {
      case 1:
      case 6:
        caseprintf("|  ");
        CPos(5, i*2-1);
        caseprintf("|__");
        setcolor(15);
        CPos(4, i*2-1);
        caseprintf("|");
        CPos(5, i*2-1);
        caseprintf("|");
        break;
      case 0:
      case 5:
        caseprintf("  |");
        CPos(5, i*2-1);
        caseprintf("__|");
        setcolor(15);
        CPos(4, i*2+1);
        caseprintf("|");
        CPos(5, i*2+1);
        caseprintf("|");
        break;
      case 3:
      case 8:
      case 10:
        printf("   ");
        CPos(5, i*2-1);
        caseprintf("___");
        break;
      default:
        caseprintf(" | ");
        CPos(5, i*2-1);
        caseprintf("_|_");
        setcolor(15);
        CPos(4, i*2);
        caseprintf("|");
        CPos(5, i*2);
        caseprintf("|");
        break;
    }
  }

  const int top=5, bot=10, lef=5, rig=45;
  if (menupending) {
    CPos(top, lef+1);
    for (i=lef+1; i<rig; i++)
      printf("-");
    CPos(bot, lef+1);
    for (i=lef+1; i<rig; i++)
      printf("-");
    for (i=top+1; i<bot; i++) {
      CPos(i, lef);
      printf("|");
      CPos(i, rig);
      printf("|");
    }
    CPos(top, lef);
    printf("+");
    CPos(top, rig);
    printf("+");
    CPos(bot, lef);
    printf("+");
    CPos(bot, rig);
    printf("+");

    CPos(top+1, lef+2);
    printf("Instrument: %s", instrums[instr]);
    CPos(top+2, lef+2);
    printf("Duration: %d", dura);
    CPos(top+3, lef+2);
    printf("Velocity: %d", velocity);
    CPos(top+1+menupoz, lef+1);
    printf("%c", 15);
  }
  CPos(11,1);
  printf("Press [TAB] to exit, [RET] to toggle menu");
  if (menupending)
    printf(", %c to toggle, %c to select", 29, 18); //, 17, 16, 30, 31);
  CPos(7,1);
}

static void handler(int sig) {
  while (!pids.empty() && (waitpid(pids.front().first, NULL, WNOHANG) > 0 || errno == ECHILD)) {
    int i;
    i = pids.front().second+1;
    pids.pop_front();
    CPos(2, i*2);
    setcolor(9 | (colmap[i%12]*240));
    printf("%c", keyconf[i]);
    setcolor(15);
    CPos(7,1);
  }
}

static void interprt(char c) {
  static int okcharz=0;
  if (c=='\n' || c=='\r') {
    menupending=!menupending;
    redraw();
    return;
  }
#ifdef __WIN32
  if (c=='\xe0' && menupending) // -32
    okcharz=2;
  if (c=='H')
    c='A';
  else if (c=='P')
    c='B';
  else if (c=='M')
    c='C';
  else if (c=='K')
    c='D';
  else if (c=='A')
    c='H';
  else if (c=='B')
    c='P';
  else if (c=='C')
    c='M';
  else if (c=='D')
    c='K';
#else
  if (c=='\x1b' && menupending) {
    okcharz=1;
    return;
  }
  else if (c=='[' && okcharz==1) {
    okcharz=2;
    return;
  }
#endif
  if (okcharz==2) {
    okcharz=0;
    switch (c) {
      case 'A':
        menupoz--;
        break;
      case 'B':
        menupoz++;
        break;
      case 'C':
        (menupoz==1 ? dura : menupoz==2 ? velocity : instr)++;
        break;
      case 'D':
        (menupoz==1 ? dura : menupoz==2 ? velocity : instr)--;
        break;
      default:
        okcharz=69;
        break;
    }
    instr&=127;
	velocity&=127;
    dura&=7;
    menupoz += 3;
	menupoz %= 3;
    if (!okcharz) {
      redraw();
      return;
    }
  }
  okcharz=0;
  for (int i=0; keyconf[i]; i++)
    if (keyconf[i]==c) {
      if (i==0) {
        exitflag=true;
        return;
      }
      CPos(2, i*2);
      setcolor(144 | (colmap[i%12]*15));
      printf("%c", c);
      setcolor(15);
      CPos(7,1);
      playsound(i-1, velocity);
    }
}

int main() {
  struct sigaction act;
  //act.sa_flags = SA_NOCLDSTOP;
  act.sa_handler = handler;
  sigaction(SIGCHLD,&act,NULL);

#ifndef __WIN32
  fprintf(stderr, "\x1b[2J");
#endif
  redraw();
  while (!exitflag)
    interprt(getch());
#ifndef __WIN32
  fprintf(stderr, "\x1b[H\x1b[J");
#endif
  return 0;
}
