#include <stdio.h>
#include <stdlib.h>

#include "Memoria.h"

void atualizaPSW(Memoria *memoria){
	int tempPSW = 0;
	if(memoria->acc == 0) tempPSW += 2;
	if(memoria->acc < 0) tempPSW += 1;
	memoria->psw = tempPSW;
}

void incrementaPC(Memoria *memoria) {
    ++memoria->pc;
}

void load(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);
    memoria->acc = memoria->memoria[memoria->pc + M];
}

void store(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);
    memoria->memoria[memoria->pc + M] = memoria->acc;
}

void read(Memoria *memoria) {
    scanf("%d", &(memoria->acc));
    printf("\n");
}

void write(Memoria *memoria) {
    printf("%d\n", memoria->acc);
}

void copy(Memoria *memoria){
    int R = memoria->memoria[memoria->pc];
    memoria->registradores[R] = memoria->acc;
    incrementaPC(memoria);
}

void xch(Memoria *memoria){
    int R = memoria->memoria[memoria->pc];
    int temp = memoria->acc;
    memoria->acc = memoria->registradores[R];
    memoria->registradores[R] = temp;

    atualizaPSW(memoria);
    incrementaPC(memoria);
}
