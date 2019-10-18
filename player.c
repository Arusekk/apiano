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
  "MThd"              // MIDI track header chunk
  "\x00\x00\x00\x06"  // 6 bytes in MThd
  "\x00\x00"          // ver 0 (single track)
  "\x00\x01"          // 1 track
  "\x00\x60"          // 96 ticks per 1/4

  "MTrk"              // MIDI track chunk
  "",                 // 12+4

  "\x00\xff\x58\x04"  // time signature
  "\x04\x02"          // 4/(1<<2) = 4/4
  "\x18"              // 24 clocks / metronome tick
  "\x08"              // 8 notated 1/32 per MIDI 1/4

  "\x00\xc0"          // select patch (instrument)
  "",                 // 8+2

  "\x00\xff\x2f\x00"  // track end
};

static int calc(unsigned long value) {
  int charz=1;
  while (value>>=7) charz++;
  return charz;
}

static void WriteVarLen(unsigned long value, FILE* outfile) {
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

static FILE *soundfp = NULL;

static void openfile(int length, int pitch) {
  int i;
  pid_t child_pid;
  int fd[2];
  int durax;

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
  soundfp = fdopen(fd[WRITE], "w");
  fwrite(*contents, 1, 19, soundfp);
  durax = htonl(length);
  
  fwrite(&durax, 1, 4, soundfp);
  fwrite(contents[1], 1, 10, soundfp);
  fputc(instr, soundfp);
  addpid(child_pid, pitch);
}

static void closefile() {
  fwrite(contents[2], 1, 4, soundfp);
  fclose(soundfp);
  soundfp = NULL;
}

void playsound(int pitch, int velocity) {
  if (soundfp == NULL)
    openfile(7+calc(8<<dura), pitch);
  fputc('\0', soundfp);
  fprintf(soundfp, "\x90%c%c", pitch+48, velocity);
  fflush(soundfp);
  WriteVarLen(8<<dura, soundfp);
  fprintf(soundfp, "\x80%c%c", pitch+48, velocity);
  closefile();
}

