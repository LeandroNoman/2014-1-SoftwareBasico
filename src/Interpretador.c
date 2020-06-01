#include "Memoria.h"
#include "operacoes.h"
#include <stdio.h>
#include <stdlib.h>

int leitor(char* fileName, Memoria *mem) {
    FILE *entrada;
    char cabecalhoS, cabecalhoA;
	int dado, i;

	entrada = fopen(fileName, "rb");
	if(entrada == NULL) {
		printf("Nao foi possivel abrir o aquivo de entrada!\n");
		return -1;
	}

	//Le os primeiros 2 bytes, 'S' e 'A'
	fread(&cabecalhoS, 1, 1, entrada);
	fread(&cabecalhoA, 1, 1, entrada);
	//printf("SA = %c%c\n", cabecalhoS, cabecalhoA);

	//Se nao for os bytes corretos, termina
	if(cabecalhoS != 'S' || cabecalhoA != 'A') {
		printf("Arquivo invalido. Assinatura incorreta.");
		return -1;
	}

	//Le o PC inicial
	fread(&dado, 4, 1, entrada);
	//printf("%d\n", dado);
	mem->pc = dado;
	//printf("\nPC inicial = %d\n", dado);

	//Le o SP inicial
	fread(&dado, 4, 1, entrada);
	//printf("%d\n", dado);
	mem->sp = dado;
	//printf("SP inicial = %d\n", dado);

	//Le as instrucoes
	for(i = 0; fread(&dado, 4, 1, entrada); i++) {
		//printf("%x\n", dado);
		mem->memoria[i] = dado;
		if(feof(entrada)) break;
	}

	fclose(entrada);
}

int executa(Memoria *mem){
	int instrucao;

	while(1){
		instrucao = mem->memoria[mem->pc];
		++mem->pc;
		printf("%d, %d\n", instrucao, mem->pc - 1);
		switch(instrucao){
		    //Load
            case 1:
                load(mem);
                break;
            //Store
            case 2:
                store(mem);
                break;
            //Read
            case 3:
                read(mem);
                break;
            //Write
            case 4:
                write(mem);
                break;
            //Copy
            case 5:
                copy(mem);
                break;
            //Xch
            case 6:
                xch(mem);
                break;
            //Add
            case 7:
                add(mem);
                break;
            //Sub
            case 8:
                sub(mem);
                break;
            //And
            case 9:
                and(mem);
                break;
            //Or
            case 10:
                or(mem);
                break;
            //Xor
            case 11:
                xor(mem);
                break;
            //Not
            case 12:
                not(mem);
                break;
            //Jmp
            case 13:
                jmp(mem);
                break;
            //Jz
            case 14:
                jz(mem);
                break;
            //Jnz
            case 15:
                jnz(mem);
                break;
            //Jn
            case 16:
                jnegativo(mem);
                break;
            //Jnn
            case 17:
                jnn(mem);
                break;
            //Push
            case 18:
                push(mem);
                break;
            //Pop
            case 19:
                pop(mem);
                break;
            //Call
            case 20:
                call(mem);
                break;
            //Ret
            case 21:
                ret(mem);
                break;
            //Dump
            case 22:
                dump(mem);
                break;
            //Halt
            case 23:
            default:
                return 0;
		}
		mem->pi = instrucao;
	}
}
