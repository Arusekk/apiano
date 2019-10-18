#ifndef PLAYER_H
#define PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

void addpid(int pid, int );
void playsound(int pitch, int velocity);
extern int instr, dura;

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_H */
