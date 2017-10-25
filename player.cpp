#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <cerrno>
#include <list>
#include <algorithm>

extern void Sleep(int);
extern int instr, dura;

const char* contents[]={
  "MThd\x00\x00\x00\x06\x00\x00\x00\x01\x00\x60MTrk\x00\x00\x00", // 12+7
  "\x00\xff\x58\x04\x04\x02\x18\x08\x00\xc0" // 8+2
};

int calc(register unsigned long value) {
  register int charz=1;
  while (value>>=7) charz++;
  return charz;
}

void WriteVarLen(FILE* outfile, register unsigned long value) {
  register unsigned long buffer;
  buffer = value & 0x7F;

  while (value>>=7) {
    buffer<<=8;
    buffer|=((value&0x7F) | 0x80);
  }

  while (true) {
    putc(buffer, outfile);
    if (buffer & 0x80)
      buffer >>= 8;
    else
      break;
  }
}

#define READ  0
#define WRITE 1
std::list<std::pair<int,int> > pids;

void playsound(int pitch) {
  int i;
  FILE* fp;
  pid_t child_pid;
  int fd[2];
  pipe(fd);

  if((child_pid = fork()) == -1) return;

  if (child_pid == 0) {
      close(fd[WRITE]);
      dup2(fd[READ], 0);
      fd[WRITE] = open("/dev/null", O_RDONLY);
      dup2(fd[WRITE], 2);
      dup2(2, 1);
      close(fd[WRITE]);

      execl("/usr/bin/timidity", "/usr/bin/timidity", "-", NULL);
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
  fprintf(fp, "\x90%c\x60", pitch+48);
  WriteVarLen(fp, 8<<dura);
  fprintf(fp, "\x80%c\x60", pitch+48);
  fwrite("\0\xff\x2f\0", 1, 4, fp);
  fclose(fp);
  pids.push_back(std::make_pair(child_pid, pitch));
}
