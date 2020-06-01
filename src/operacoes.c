#include "Memoria.h"
#include <stdio.h>

/*
PSW:
bit mais significativo indica se o ultimo resultado foi zero;
bit menos significativo se foi negativo.

converte isso pra inteiro e GGWP
*/

// ALU
void add(Memoria *memoria){
    int R = memoria->memoria[memoria->pc];
	memoria->acc += memoria->registradores[R];

	atualizaPSW(memoria);
	incrementaPC(memoria);
}

void sub(Memoria *memoria){
	int R = memoria->memoria[memoria->pc];
	memoria->acc -= memoria->registradores[R];

	atualizaPSW(memoria);
	incrementaPC(memoria);
}


void and(Memoria *memoria){
	int R = memoria->memoria[memoria->pc];
	memoria->acc &= memoria->registradores[R];

	atualizaPSW(memoria);
	incrementaPC(memoria);
}

void xor(Memoria *memoria){
	int R = memoria->memoria[memoria->pc];
	memoria->acc ^= memoria->registradores[R];

	atualizaPSW(memoria);
	incrementaPC(memoria);
}

void or(Memoria *memoria){
	int R = memoria->memoria[memoria->pc];
	memoria->acc |= memoria->registradores[R];

	atualizaPSW(memoria);
	incrementaPC(memoria);
}

void not(Memoria *memoria){
	memoria->acc = ~memoria->acc;
	atualizaPSW(memoria);
}


// BRANCHES
// incondicional
void jmp(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);

	memoria->pc += M;
}

//se zero
void jz(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
	incrementaPC(memoria);

	if(memoria->psw == 3 || memoria->psw == 2) memoria->pc += M;
}

//se nao zero
void jnz(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);

	if(memoria->psw == 0 || memoria->psw == 1) memoria->pc += M;
}

//se negativo; nao pus jn porque nao pode
void jnegativo(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);

	if(memoria->psw == 1 || memoria->psw == 3) memoria->pc += M;
}

//se nao negativo
void jnn(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);

	if(memoria->psw == 0 || memoria->psw == 2) memoria->pc += M;
}

// PILHA

void push(Memoria *memoria){
	// MEM[--SP] = ACC;
	memoria->memoria[--(memoria->sp)] = memoria->acc;
}

void pop(Memoria *memoria){
	//ACC = MEM[SP]
	memoria->acc = memoria->memoria[memoria->sp];
	//++SP
	++memoria->sp;
}

// SUB ROTINAS

void call(Memoria *memoria){
    int M = memoria->memoria[memoria->pc];
    incrementaPC(memoria);
	//MEM[--SP] = PC
	memoria->memoria[--memoria->sp] = memoria->pc;
	//PC = PC + offset
	memoria->pc += M;
}

void ret(Memoria *memoria){
	//PC = MEM[SP]
	memoria->pc = memoria->memoria[memoria->sp];
	//SP++;
	memoria->sp++;
}

////////////////

void dump(Memoria *memoria){
	int i;
	printf("\n%d %d %d %d ",memoria->acc,memoria->pc,memoria->pi,memoria->sp);

	//zero
	if(memoria->psw == 0 || memoria->psw == 1) printf("0 ");
	else printf("1 ");
	//negativo
	if(memoria->psw == 1 || memoria->psw == 3) printf("1 ");
	else printf("0 ");

	for(i = 0; i < 5; i++) printf("%d ",memoria->registradores[i]);

}

