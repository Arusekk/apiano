#ifdef __WIN32

#include <conio.h>
#include <windows.h>

#else // __WIN32

#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#define STD_OUTPUT_HANDLE stdout
#define GetStdHandle(x) x
#define kbhit _kbhit
#define getch _getch
static struct termios termios_saved_attributes;
static void termios_reset_input_mode() {
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_saved_attributes);
}
static void termios_set_input_mode() {
  struct termios tattr;
  if (!isatty(STDIN_FILENO)) {
    fprintf(stderr, "Not a terminal.\n");
    exit(EXIT_FAILURE);
  }
  tcgetattr(STDIN_FILENO, &termios_saved_attributes);
  atexit(termios_reset_input_mode);
  tcgetattr(STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO);
  tattr.c_iflag &= ~(INLCR|ICRNL|IGNCR);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}
static void Sleep(int ms) {
  fflush(stdout);
  timespec tim, tim2;
  tim.tv_sec = ms/1000;
  tim.tv_nsec = 1000000L*(ms%1000);
  int err;
  if ((err=nanosleep(&tim, &tim2))<0)
    nanosleep(&tim2, NULL);
}
static void _init_hit() {
  static bool isinited=false;
  if (isinited) return;
  isinited=true;
  termios_set_input_mode();
}
static bool _kbhit() {
  static struct pollfd _fds = {
    .fd = STDIN_FILENO,
    .events = POLLIN
  };
  _init_hit();
  poll(&_fds, 1, 0);
  return _fds.revents & POLLIN;
}
static char _getch() {
  char c;
  _init_hit();
  while (read(STDIN_FILENO, &c, 1) != 1 && errno == EINTR);
  return c;
}
struct COORD {
  int X;
  int Y;
};
typedef unsigned char uchar;
static void SetConsoleCursorPosition(FILE*f, COORD p) {
  fprintf(f, "\x1b[%d;%dH", p.Y+1, p.X+1);
  fflush(f);
}
static void SetConsoleTextAttribute(FILE*f, short color) {
  static const uchar COLOR_WIN_TO_LNX[]={0,4,2,6,1,5,3,7};
  int fg=color%16;
  int bg=color/16;
  int xdfg=3+(fg&8)/4*3;
  int xdbg=4+(bg&8)/4*3;
  bg=COLOR_WIN_TO_LNX[bg&7];
  fg=COLOR_WIN_TO_LNX[fg&7];
  if (color & 8192) return; // 0x2000 flag makes no change happen
  if (xdbg != 4 || xdfg != 3 && strcmp(getenv("TERM"), "linux")==0) {
	  xdbg = 4;
	  xdfg = 3;
  }
  fprintf(f, "\x1b[%d%d;%d%dm", xdbg, bg, xdfg, fg);
  fflush(f);
}
static const char* CP852_2_UTF[]={"\xc3\x87",
"\xc3\xbc",
"\xc3\xa9",
"\xc3\xa2",
"\xc3\xa4",
"\xc5\xaf",
"\xc4\x87",
"\xc3\xa7",
"\xc5\x82",
"\xc3\xab",
"\xc5\x90",
"\xc5\x91",
"\xc3\xae",
"\xc5\xb9",
"\xc3\x84",
"\xc4\x86",
"\xc3\x89",
"\xc4\xb9",
"\xc4\xba",
"\xc3\xb4",
"\xc3\xb6",
"\xc4\xbd",
"\xc4\xbe",
"\xc5\x9a",
"\xc5\x9b",
"\xc3\x96",
"\xc3\x9c",
"\xc5\xa4",
"\xc5\xa5",
"\xc5\x81",
"\xc3\x97",
"\xc4\x8d",
"\xc3\xa1",
"\xc3\xad",
"\xc3\xb3",
"\xc3\xba",
"\xc4\x84",
"\xc4\x85",
"\xc5\xbd",
"\xc5\xbe",
"\xc4\x98",
"\xc4\x99",
"\xc2\xac",
"\xc5\xba",
"\xc4\x8c",
"\xc5\x9f",
"\xc2\xab",
"\xc2\xbb",
"\xe2\x96\x91",
"\xe2\x96\x92",
"\xe2\x96\x93",
"\xe2\x94\x82",
"\xe2\x94\xa4",
"\xc3\x81",
"\xc3\x82",
"\xc4\x9a",
"\xc5\x9e",
"\xe2\x95\xa3",
"\xe2\x95\x91",
"\xe2\x95\x97",
"\xe2\x95\x9d",
"\xc5\xbb",
"\xc5\xbc",
"\xe2\x94\x90",
"\xe2\x94\x94",
"\xe2\x94\xb4",
"\xe2\x94\xac",
"\xe2\x94\x9c",
"\xe2\x94\x80",
"\xe2\x94\xbc",
"\xc4\x82",
"\xc4\x83",
"\xe2\x95\x9a",
"\xe2\x95\x94",
"\xe2\x95\xa9",
"\xe2\x95\xa6",
"\xe2\x95\xa0",
"\xe2\x95\x90",
"\xe2\x95\xac",
"\xc2\xa4",
"\xc4\x91",
"\xc4\x90",
"\xc4\x8e",
"\xc3\x8b",
"\xc4\x8f",
"\xc5\x87",
"\xc3\x8d",
"\xc3\x8e",
"\xc4\x9b",
"\xe2\x94\x98",
"\xe2\x94\x8c",
"\xe2\x96\x88",
"\xe2\x96\x84",
"\xc5\xa2",
"\xc5\xae",
"\xe2\x96\x80",
"\xc3\x93",
"\xc3\x9f",
"\xc3\x94",
"\xc5\x83",
"\xc5\x84",
"\xc5\x88",
"\xc5\xa0",
"\xc5\xa1",
"\xc5\x94",
"\xc3\x9a",
"\xc5\x95",
"\xc5\xb0",
"\xc3\xbd",
"\xc3\x9d",
"\xc5\xa3",
"\xc2\xb4",
"\xc2\xad",
"\xcb\x9d",
"\xcb\x9b",
"\xcb\x87",
"\xcb\x98",
"\xc2\xa7",
"\xc3\xb7",
"\xc2\xb8",
"\xc2\xb0",
"\xc2\xa8",
"\xcb\x99",
"\xc5\xb1",
"\xc5\x98",
"\xc5\x99",
"\xe2\x96\xa0",
"\xc2\xa0"};
static const char* con2utf[]={
"\0",
"\xe2\x98\xba",
"\xe2\x98\xbb",
"\xe2\x99\xa5",
"\xe2\x99\xa6",
"\xe2\x99\xa3",
"\xe2\x99\xa0",
"\x07",
"\x08",
"\t",
"\n",
"\xe2\x99\x80",
"\xe2\x99\x82",
"\r",
"\xe2\x99\xac",
"\xe2\x98\xbc",
"\xe2\x96\xb6",
"\xe2\x97\x80",
"\xe2\x86\x95",
"\xe2\x80\xbc",
"\xc2\xb6",
"\xc2\xa7",
"\xe2\x96\x82",
"\xe2\x86\xa8",
"\xe2\x86\x91",
"\xe2\x86\x93",
"\xe2\x86\x92",
"\xe2\x86\x90",
"\xe2\x8c\x99",
"\xe2\x86\x94",
"\xe2\x96\xb2",
"\xe2\x96\xbc"};
static const char* cp2utf(char& x) {
  static char spec178[14];
  uchar us=x;
  if (us<32) return con2utf[us];
  if (us<128) return &x;
  if (us==219) return "\x1b[7m \x1b[27m";
  if (!spec178[0])
    sprintf(spec178,"\x1b[7m%s\x1b[27m",CP852_2_UTF[176-128]);
  if (us==178) return spec178;
  return CP852_2_UTF[us-128];
}
int printf(const char* fmt, ...) {
  char s[1000];
  va_list argz;
  va_start(argz, fmt);
  size_t len=vsprintf(s, fmt, argz);
  va_end(argz);
  size_t i;
  char c[2]={'\0','\0'};
  for (i=0; i<len; i++) {
    c[0]=s[i];
    fputs(cp2utf(c[0]), stdout);
  }
  fflush(stdout);
  return 1;
}
#endif // __WIN32
