#include <stdio.h>
#include <stdlib.h>
#include "Interpretador.h"


int main(int argc, char** argv) {
	Memoria mem;
	leitor(argv[1], &mem);
	executa(&mem);
}

