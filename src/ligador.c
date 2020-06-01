#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAM_LABEL 100
#define MAX_TAM_STRING 100
#define MAX_ARGS 50

typedef struct TS {
	char label [MAX_TAM_LABEL];
	int pc;
	struct TS* prox;
} tabela_simbolos;

/************************************
 * Guarda os argumentos do programa *
 ************************************/
int nomes_modulos(int argc, char** argv, char modulo_principal[MAX_TAM_STRING], char modulos_objeto [MAX_ARGS][MAX_TAM_STRING], FILE** saida) {
	int i, index_obj;
	int flag_o, flag_m;

	if(argc <= 1) {
		printf("\nArgumentos invalidos.\n\n");
		return -1;
	}

	index_obj = 0;
	flag_o = 0;
	flag_m = 0;
	for(i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-m")) {
			flag_m++;
			i++;
			strcpy(modulo_principal, argv[i]);
		}
		else if(!strcmp(argv[i], "-o")) {
			i++;
			if(flag_o) continue;
			flag_o = 1;
			*saida = fopen(argv[i], "w+b");
		}
		else {
			strcpy(modulos_objeto[index_obj], argv[i]);
			index_obj++;
		}
	}

	if(!flag_m) {
		printf("\nUm modulo principal deve ser especificado.\n\n");
		return -1;
	}

	if(!flag_o) {
		*saida = fopen("programa.sa", "w+b");
	}

	return index_obj;
}

/****************************************************************
 * Preenche as tabelas de simbolos para cada arquivo de entrada *
 ****************************************************************/
void preenche_tabelas(tabela_simbolos* tabelas, int qnt_modulos, char modulo_principal[MAX_TAM_STRING],
                      char modulos_objeto[MAX_ARGS][MAX_TAM_STRING], int modulos_tamanho [MAX_ARGS],
                      int modulos_inicio [MAX_ARGS]) {
	int i, qnt_simbolos, modulo_atual;

	tabela_simbolos* tabela_atual;

	FILE* entrada;

	/*
	 * Cria a tabela do modulo principal
	 */

	entrada = fopen(modulo_principal, "r");
	tabela_atual = &tabelas[0];
	//O primeiro numero indica quantos simbolos tem na tabela
	fscanf(entrada, "%d", &qnt_simbolos);
	for(i = 0; i < qnt_simbolos; i++) {
		//Cada linha do arquivo possui um label e um endereco
		fscanf(entrada, "%s %d", tabela_atual->label, &tabela_atual->pc);
		if(i != qnt_simbolos - 1) {
			tabela_atual->prox = malloc(sizeof(tabela_simbolos));
			tabela_atual = tabela_atual->prox;
		}
	}
	tabela_atual->prox = NULL;
	//Le o tamanho do modulo em memoria
	fscanf(entrada, "%d", &modulos_tamanho[0]);
	//Guarda o inicio do codigo no modulo
	modulos_inicio[0] = ftell(entrada);
	fclose(entrada);

    /*
	 * Cria as tabelas dos outros modulos
	 */

	for(modulo_atual = 0; modulo_atual < qnt_modulos; modulo_atual++) {
		entrada = fopen(modulos_objeto[modulo_atual], "r");
		tabela_atual = &tabelas[modulo_atual + 1];
		qnt_simbolos = 0;
		//O primeiro numero indica quantos simbolos tem na tabela
		fscanf(entrada, "%d", &qnt_simbolos);
		for(i = 0; i < qnt_simbolos; i++) {
			//Cada linha do arquivo possui um label e um endereco
			fscanf(entrada, "%s %d", tabela_atual->label, &tabela_atual->pc);
			if(i != qnt_simbolos - 1) {
				tabela_atual->prox = malloc(sizeof(tabela_simbolos));
				tabela_atual = tabela_atual->prox;
			}
		}
		tabela_atual->prox = NULL;
		//Le o tamanho do modulo em memoria
        fscanf(entrada, "%d", &modulos_tamanho[modulo_atual + 1]);
        //Guarda o inicio do codigo no modulo
        modulos_inicio[modulo_atual + 1] = ftell(entrada);
		fclose(entrada);
	}

}

/*************************************************
 * Processo de realocacao dos modulos na memoria *
 *************************************************/
void realoca_modulos(tabela_simbolos* tabelas, int* modulos_tamanho, int qnt_modulos) {
    int i, acumulador;
    tabela_simbolos* tabela_atual;

    acumulador = modulos_tamanho[0];
    for(i = 1; i <= qnt_modulos; i++) {
        tabela_atual = &tabelas[i];
        while(tabela_atual != NULL) {
            tabela_atual->pc += acumulador;
            tabela_atual = tabela_atual->prox;
        }
        acumulador += modulos_tamanho[i];
    }
}

int pesquisa_simbolo(tabela_simbolos* tabelas, char label [MAX_TAM_LABEL], int qnt_modulos) {
	int i;

	tabela_simbolos* tabela_atual;

	for(i = 0; i <= qnt_modulos; i++) {
		tabela_atual = &tabelas[i];
		while(tabela_atual != NULL) {
			if(!strcmp(label, tabela_atual->label)) return tabela_atual->pc;
			tabela_atual = tabela_atual->prox;
		}
	}

	return -1;
}

void cria_programa_binario(FILE* saida, tabela_simbolos* tabelas, int qnt_modulos,
						char modulo_principal [MAX_TAM_STRING],
						char modulos_objeto [MAX_ARGS][MAX_TAM_STRING],
						int modulos_inicio [MAX_ARGS]) {
	int buffer, i, pc;
	char label [MAX_TAM_LABEL];

	FILE* entrada;

	//Escreve a assinatura "SA"
	fprintf(saida, "SA");
	//Escreve o PC e o SP, respectivamente 0 e 1000
	buffer = 0;
	fwrite(&buffer, 4, 1, saida);
	buffer = 1000;
	fwrite(&buffer, 4, 1, saida);

	pc = 0;
	//Le os arquivos de entrada e os escreve na saida,
	//trocando os labels por seus respectivos enderecos
	entrada = fopen(modulo_principal, "r");
	fseek(entrada, modulos_inicio[0], SEEK_SET);
	while(!feof(entrada)) {
		if(fscanf(entrada, "%d", &buffer) == 0) break;
		//Se for um WORD
		if(buffer == 24) {
			fscanf(entrada, "%d", &buffer);
			fwrite(&buffer, 4, 1, saida);
			pc++;
			continue;
		}
		fwrite(&buffer, 4, 1, saida);
		pc++;
		//Se for uma instrucao que referencia um registrador, le o registrador
		if(buffer >= 5 && buffer <= 11) {
			fscanf(entrada, "%d", &buffer);
			fwrite(&buffer, 4, 1, saida);
			pc++;
		}
		//Se for uma instrucao que referencia a memoria, verifica o endereco na tabela de simbolos
		else if((buffer >= 1 && buffer <= 2) || (buffer >= 13 && buffer <= 17) || buffer == 20) {
			fscanf(entrada, "%s", label);
			pc++;
			buffer = pesquisa_simbolo(tabelas, label, qnt_modulos);
			buffer -= pc;
			fwrite(&buffer, 4, 1, saida);
		}
	}
	fclose(entrada);

	for(i = 1; i <= qnt_modulos; i++) {
		entrada = fopen(modulos_objeto[i - 1], "r");
		fseek(entrada, modulos_inicio[i], SEEK_SET);
		while(!feof(entrada)) {
			if(fscanf(entrada, "%d", &buffer) == 0) break;
			//Se for um WORD
			if(buffer == 24) {
				fscanf(entrada, "%d", &buffer);
				fwrite(&buffer, 4, 1, saida);
				pc++;
				continue;
			}
			fwrite(&buffer, 4, 1, saida);
			pc++;
			//Se for uma instrucao que referencia um registrador, le o registrador
			if(buffer >= 5 && buffer <= 11) {
				fscanf(entrada, "%d", &buffer);
				fwrite(&buffer, 4, 1, saida);
				pc++;
			}
			//Se for uma instrucao que referencia a memoria, verifica o endereco na tabela de simbolos
			else if((buffer >= 1 && buffer <= 2) || (buffer >= 13 && buffer <= 17) || buffer == 20) {
				fscanf(entrada, "%s", label);
				pc++;
				buffer = pesquisa_simbolo(tabelas, label, qnt_modulos);
				buffer -= pc;
				fwrite(&buffer, 4, 1, saida);
			}
		}
		fclose(entrada);
	}

}

int main(int argc, char** argv) {
	char modulo_principal [MAX_TAM_STRING];
	char modulos_objeto [MAX_ARGS][MAX_TAM_STRING];
	int modulos_tamanho [MAX_ARGS];
	int modulos_inicio [MAX_ARGS];

	int qnt_modulos, i;

	tabela_simbolos* tabelas;

	FILE* saida;

	qnt_modulos = nomes_modulos(argc, argv, modulo_principal, modulos_objeto, &saida);
	if(qnt_modulos < 0) return 0;

	//Cria uma tabela de simbolos para cada arquivo objeto de entrada mais o arquivo principal
	tabelas = malloc((qnt_modulos + 1)*sizeof(tabela_simbolos));

	preenche_tabelas(tabelas, qnt_modulos, modulo_principal, modulos_objeto, modulos_tamanho, modulos_inicio);

	realoca_modulos(tabelas, modulos_tamanho, qnt_modulos);

	cria_programa_binario(saida, tabelas, qnt_modulos, modulo_principal, modulos_objeto, modulos_inicio);

	fclose(saida);

	return 0;
}
