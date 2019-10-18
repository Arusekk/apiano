#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "player.h"

extern void Sleep(int);
extern int instr, dura;

static const char* contents[]={
  "MThd\x00\x00\x00\x06\x00\x00\x00\x01\x00\x60MTrk\x00\x00\x00", // 12+7
  "\x00\xff\x58\x04\x04\x02\x18\x08\x00\xc0" // 8+2
};

static int calc(unsigned long value) {
  int charz=1;
  while (value>>=7) charz++;
  return charz;
}

static void WriteVarLen(FILE* outfile, unsigned long value) {
  unsigned long buffer;
  buffer = value & 0x7F;

  while (value>>=7) {
    buffer<<=8;
    buffer|=((value&0x7F) | 0x80);
  }

  do {
    putc(buffer, outfile);
    if (buffer & 0x80)
      buffer >>= 8;
    else
      break;
  } while(1);
}

#define READ  0
#define WRITE 1

void playsound(int pitch, int velocity) {
  int i;
  FILE* fp;
  pid_t child_pid;
  int fd[2];
  pipe(fd);

  if ((child_pid = fork()) == -1) return;

  if (child_pid == 0) {
    close(fd[WRITE]);
    dup2(fd[READ], 0);
    close(fd[READ]);
    pipe(fd);

    if ((child_pid = fork()) == -1) exit(1);
    if (child_pid == 0) {
      close(fd[READ]);
      dup2(fd[WRITE], 1);
      close(fd[WRITE]);

      execl("/usr/bin/timidity", "timidity", "-idqq", "-o-", "-Or1sl", "-", NULL);
      exit(0);
    }
    close(fd[WRITE]);
    dup2(fd[READ], 0);
    close(fd[READ]);

    execl("/usr/bin/pacat", "pacat", "-nAPiano", NULL);
    exit(0);
  }
  close(fd[READ]);
  fp = fdopen(fd[WRITE], "w");
  fwrite(*contents, 1, 19, fp);
//   for (i=0; i<19; i++)
//     fprintf(fp, "%c", contents[0][i]);
  fprintf(fp, "%c", calc(8<<dura)+22);
  fwrite(contents[1], 1, 10, fp);
//   for (i=0; i<10; i++)
//     fprintf(fp, "%c", contents[1][i]);
  fputc(instr, fp);
  fputc('\0', fp);
  fprintf(fp, "\x90%c%c", pitch+48, velocity);
  WriteVarLen(fp, 8<<dura);
  fprintf(fp, "\x80%c%c", pitch+48, velocity);
  fwrite("\0\xff\x2f\0", 1, 4, fp);
  fclose(fp);
  addpid(child_pid, pitch);
}
