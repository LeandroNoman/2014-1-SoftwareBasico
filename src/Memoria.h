#ifndef _Memoria_h
#define _Memoria_h

typedef struct {
	int acc, pc, pi, sp, psw;
	int registradores[5];
	int memoria[1000];
} Memoria;

void atualizaPSW(Memoria *memoria);

void incrementaPC(Memoria *memoria);

void load(Memoria *memoria);

void store(Memoria *memoria);

void read(Memoria *memoria);

void write(Memoria *memoria);

void copy(Memoria *memoria);

//exchange??
void xch(Memoria *memoria);

#endif
