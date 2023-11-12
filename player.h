#ifndef PLAYER_H
#define PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

void addpid(int pid, int pitch);
void playsound(int pitch, int velocity);
void closefile(void);
extern int instr, dura, octave;

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_H */
