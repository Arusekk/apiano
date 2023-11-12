#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "player.h"


static char contents[][36]={
  "MThd"              // MIDI track header chunk
  "\x00\x00\x00\x06"  // 6 bytes in MThd
  "\x00\x00"          // ver 0 (single track)
  "\x00\x01"          // 1 track
  "\x00\x60"          // 96 ticks per 1/4

  "MTrk"              // MIDI track chunk
  "\x7f\xff\xff\xff"  // bytes in MTrk (adjusted later)

  "\x00\xff\x58\x04"  // time signature event
  "\x04\x02"          // 4/(1<<2) = 4/4
  "\x18"              // 24 clocks / metronome tick
  "\x08"              // 8 notated 1/32 per MIDI 1/4

  "\x00\xc0"          // select patch (instrument) event
  "\x00",             // acoustic grand by default (adjusted later)

  "\x00\xff\x2f\x00"  // track end
};

static uint32_t *const trk_len = (uint32_t*)(contents[0] + 18);
static uint8_t  *const trk_ins = contents[0] + 32;
static FILE *soundfp = NULL;


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

static pid_t child_pid;

static void openfile(void) {
  int i;
  int fd[2];
  int durax;

  pipe(fd);

  if ((child_pid = fork()) == -1) return;

  if (child_pid == 0) {
    close(fd[WRITE]);
    dup2(fd[READ], 0);
    close(fd[READ]);
    pipe(fd);

    if ((child_pid = fork()) == -1) _exit(1);
    if (child_pid == 0) {
      close(fd[READ]);
      dup2(fd[WRITE], 1);
      close(fd[WRITE]);

      execlp("timidity", "timidity", "-idqq", "-o-", "-Or1sl", "-", NULL);
      _exit(1);
    }
    close(fd[WRITE]);
    dup2(fd[READ], 0);
    close(fd[READ]);

    execlp("pacat", "pacat", "-nAPiano", NULL);
    _exit(1);
  }
  close(fd[READ]);
  soundfp = fdopen(fd[WRITE], "wb");

  //*trk_len = htonl(length + 15);
  *trk_ins = instr;
  fwrite(*contents, 1, 33, soundfp);
}

void closefile(void) {
  if (soundfp) {
    fwrite(contents[1], 1, 4, soundfp);
    fclose(soundfp);
    soundfp = NULL;
  }
}

void playsound(int pitch, int velocity) {
  if (soundfp == NULL)
    openfile();
  addpid(child_pid, pitch);
  fputc('\0', soundfp);
  fprintf(soundfp, "\x90%c%c", pitch + octave*12, velocity);
  fflush(soundfp);
  WriteVarLen(8<<dura, soundfp);
  fprintf(soundfp, "\x80%c%c", pitch + octave*12, velocity);
  closefile();
  openfile();
}

