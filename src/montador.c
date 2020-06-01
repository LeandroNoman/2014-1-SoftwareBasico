#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define TAM_LINHA 1024
#define MAX_TAM_INSTRUCTION 6
#define MAX_TAM_LABEL 100
#define QNT_INSTRUCTION 25

//Criacao de nomes para os codigos das instrucoes
enum INSTRUCOES {INVALIDA, LOAD, STORE, READ, WRITE, COPY, XCH, ADD, SUB, AND, OR, XOR, NOT, JMP, JZ, JNZ, JN, JNN, PUSH, POP, CALL, RET, DUMP, HALT, WORD, END};
//Vetor de strings, contendo os nomes de todas as instrucoes
char instruction_table [QNT_INSTRUCTION][MAX_TAM_INSTRUCTION] = {"LOAD", "STORE", "READ",
							"WRITE", "COPY", "XCH", "ADD", "SUB", "AND", "OR", "XOR", "NOT",
							"JMP", "JZ", "JNZ", "JN", "JNN", "PUSH", "POP", "CALL", "RET",
							"DUMP", "HALT", "WORD", "END"};

/**********************
 * Tabela de simbolos *
 **********************/
typedef struct TS {
	char label [MAX_TAM_LABEL];
	int pc;
	struct TS* prox;
} tabela_simbolos;

/*************
 * Instrucao *
 *************/
typedef struct {
	int code;
	int operando;
	char label [MAX_TAM_LABEL];
} instruction;

/*************************************************
 * Inicializacao dos arquivos de entrada e saida *
 *************************************************/
int inicializa_arquivos(int argc, char** argv, FILE** entrada, FILE** saida, int* verbose) {
	int i;
	int flag_o = 0;

	if(argc < 2) {
		printf("Argumentos invalidos.\nIndique o nome do arquivo de entrada.\n");
		return -1;
	}

	*entrada = fopen(argv[1], "r");

	if(*entrada == NULL) {
		printf("Nao foi possivel abrir o arquivo de entrada para leitura.\n");
		fclose(*entrada);
		return -1;
	}

	*verbose = 0;

	for(i = 2; i < argc; i++) {
		if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "-V")) *verbose = 1;
		else if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "-O")) flag_o = i;
	}

	if(!flag_o) {
		*saida = fopen("exec.osa", "w+b");
	}
	else {
		*saida = fopen(argv[flag_o+1], "w+b");
	}

	if(*saida == NULL) {
		printf("Nao foi possivel abrir o arquivo de saida para escrita.\n");
		fclose(*saida);
		fclose(*entrada);
		return -1;
	}

	return 0;
}

/*********************************************************************
 * Decodifica a instrucao, retornando seu codigo numerico            *
 * e seta o parametro label para 1 caso exista um label nessa linha. *
 * Essa funcao e utilizada na primeira passada do montador.          *
 *********************************************************************/
instruction decodifica_label(char* linha, int* label) {
	int i = 0, j, inicio_sem_espacos;
	char instruction_text [MAX_TAM_INSTRUCTION];
	instruction instrucao;
	*label = 0;

	//Le todos os espacos vazios iniciais
	for(; i < TAM_LINHA; i++) {
		if(linha[i] == ' ' || linha[i] == '\t') continue;
		//Se nao houver nada na linha, um sinal de invalido e enviado
		else if(linha[i] == ';' || linha[i] == '\0' || linha[i] == '\n') {
			instrucao.code = INVALIDA;
			return instrucao;
		}
		break;
	}

	//Marca onde se inicia os caracteres da instrucao
	inicio_sem_espacos = i;
	//Se existe algum label nessa linha, seta a variavel label para 1
	for(; i < TAM_LINHA; i++) {
		if(linha[i] == ':') {
			*label = 1;
			inicio_sem_espacos = i + 1;
			break;
		}
		else if(linha[i] == ' ' || linha[i] == '\t' || linha[i] == '\0' || linha[i] == ';' || linha[i] == '\n') {
			*label = 0;
			break;
		}
	}

	//Marca a posicao inicial da instrucao, caso haja um label nessa linha
	for(i = inicio_sem_espacos; i < TAM_LINHA; i++) {
		if(linha[i] == ' ' || linha[i] == '\t') continue;
		break;
	}
	j = 0;

	//Decodifica a instrucao
	for(; i < TAM_LINHA; i++) {
		//Escreve a instrucao em instruction_text
		if((linha[i] >= 'A' && linha[i] <= 'Z') || (linha[i] >= '0' && linha[i] <= '9')) {
			instruction_text[j] = linha[i];
			j++;
		}
		else break;
	}
	instruction_text[j] = '\0';

	for(i = 0; i < QNT_INSTRUCTION; i++) {
		//Se alguma instrucao foi encontrada, retorna o valor dela
		if(!strcmp(instruction_text, instruction_table[i])) {
			instrucao.code = i + 1;
			return instrucao;
		}
	}

	//Se nenhuma instrucao foi encontrada, retorna instrucao invalida
	instrucao.code = INVALIDA;
	return instrucao;
}

/************************************************
 * Insere um novo simbolo na tabela de simbolos *
 ************************************************/
void insere_simbolo(tabela_simbolos* tabela, char* linha, int pc) {
	int i, j;
	//Marca a posicao inicial do label
	for(i = 0; i < TAM_LINHA; i++) {
		if(linha[i] == ' ') continue;
		break;
	}
	//Coloca o label na tabela
	for(j = 0; i < TAM_LINHA; i++) {
		if(linha[i] == ':') break;
		tabela->label[j] = linha[i];
		j++;
	}
	tabela->label[j] = '\0';
	tabela->pc = pc;
	tabela->prox = malloc(sizeof(tabela_simbolos));
	tabela->prox->prox = NULL;
}

/******************************************
 * Realiza a primeira passada no arquivo, *
 * criando a tabela de simbolos.          *
 ******************************************/
void cria_tabela_de_simbolos(FILE* entrada, tabela_simbolos* tabela, int verbose, int* qnt_simbolos, int* pc_final) {
	tabela_simbolos* label_atual = tabela;
	instruction instrucao;

	char linha [TAM_LINHA];
	int label_flag, opcode, pc = 0;
	*(qnt_simbolos) = 0;
	*(pc_final) = 0;
	//Percorre completamente o arquivo de entrada
	while(!feof(entrada)) {
		//Faz a leitura de cada linha do arquivo de entrada
		fgets(linha, TAM_LINHA, entrada);
		//Verifica se ha um label na linha atual
		instrucao = decodifica_label(linha, &label_flag);
		opcode = instrucao.code;
		if(opcode == END) break;
		if(label_flag == 1) {
			//Se houver, coloca uma nova entrada na tabela de simbolos
			insere_simbolo(label_atual, linha, pc);
			//Se estiver executando em modo verbose, escreve na tela o label e seu endereco correspondente
			if(verbose) printf("%s %d\n", label_atual->label, pc);
			label_atual = label_atual->prox;
			*(qnt_simbolos) += 1;
		}
		//Atualiza o pc de acordo com a instrucao
		//Se a instrucao for de apenas 4 bytes:
		if(opcode == READ || opcode == WRITE || opcode == NOT || opcode == PUSH || opcode == POP || opcode == RET || opcode == DUMP || opcode == HALT || opcode == WORD) {
			pc++;
		}
		//Se for de 8 bytes:
		else if(opcode != INVALIDA) {
			pc += 2;
		}
	}
	*(pc_final) = pc;
}

/*********************************************************************
 * Decodifica a instrucao, transformando ela e o seu operando, se    *
 * houver, em suas representacoes numericas.                         *
 * Essa funcao e utilizada na segunda passada do montador.           *
 *********************************************************************/
instruction decodifica_instrucao(char* linha, tabela_simbolos* tabela, int pc) {
	int i = 0, j, inicio_sem_espacos, flag_operando;
	char instruction_text [MAX_TAM_INSTRUCTION];
	char instruction_operand [MAX_TAM_LABEL];
	instruction instrucao;

	//Le todos os espacos vazios iniciais
	for(i = 0; i < TAM_LINHA; i++) {
		if(linha[i] == ' ' || linha[i] == '\t') continue;
		//Se nao houver nada na linha, um sinal de invalido e enviado
		else if(linha[i] == ';' || linha[i] == '\0' || linha[i] == '\n') {
			instrucao.code = INVALIDA;
			return instrucao;
		}
		break;
	}

	//Marca onde se inicia os caracteres da instrucao
	inicio_sem_espacos = i;
	//Verifica se existe algum label nessa linha
	for(; i < TAM_LINHA; i++) {
		if(linha[i] == ':') {
			inicio_sem_espacos = i + 1;
			break;
		}
		else if(linha[i] == ' ' || linha[i] == '\t' || linha[i] == '\0' || linha[i] == ';' || linha[i] == '\n') {
			break;
		}
	}

	//Marca a posicao inicial da instrucao, caso haja um label nessa linha
	for(i = inicio_sem_espacos; i < TAM_LINHA; i++) {
		if(linha[i] == ' ' || linha[i] == '\t') continue;
		break;
	}

	j = 0;
	//Decodifica a instrucao
	for(; i < TAM_LINHA; i++) {
		//Escreve a instrucao em instruction_text
		if(linha[i] >= 'A' && linha[i] <= 'Z')	{
			instruction_text[j] = linha[i];
			j++;
		}
		else break;
	}
	instruction_text[j] = '\0';

	//Se nenhuma instrucao foi encontrada, retorna instrucao invalida
    instrucao.code = INVALIDA;

	for(j = 0; j < QNT_INSTRUCTION; j++) {
		//Se alguma instrucao foi encontrada, retorna o valor dela
		if(!strcmp(instruction_text, instruction_table[j])) {
			instrucao.code = j + 1;
			break;
		}
	}

	flag_operando = 1;
	//Marca o inicio do operando da instrucao, se houver algum
	for(; i < TAM_LINHA; i++) {
		if(linha[i] == ' ' || linha[i] == '\t') continue;
		//Se nao houver operando, seta o flag para 0
		else if(linha[i] == '\n' || linha[i] == '\0' || linha[i] == ';') {
			flag_operando = 0;
		}
		break;
	}

	//Se ha algum operando, copia seu texto
	if(flag_operando) {
		j = 0;
		for(; i < TAM_LINHA; i++) {
			//Escreve o operando em instruction_operand
			if((linha[i] < 'A' || linha[i] > 'Z') && (linha[i] < '!' || linha[i] > '9') && (linha[i] < 'a' || linha[i] > 'z') && linha[i] != '_') break;
			instruction_operand[j] = linha[i];
			j++;
		}
		instruction_operand[j] = '\0';

		//Transforma o operando em seu valor numerico
		flag_operando = 0;
		//Primeiramente, verifica se e algum registrador
		if(!strcmp(instruction_operand, "R0")) {
			flag_operando = 1;
			instrucao.operando = 0;
		}
		else if(!strcmp(instruction_operand, "R1")) {
			flag_operando = 1;
			instrucao.operando = 1;
		}
		else if(!strcmp(instruction_operand, "R2")) {
			flag_operando = 1;
			instrucao.operando = 2;
		}
		else if(!strcmp(instruction_operand, "R3")) {
			flag_operando = 1;
			instrucao.operando = 3;
		}
		else if(!strcmp(instruction_operand, "R4")) {
			flag_operando = 1;
			instrucao.operando = 4;
		}
		//Se a instrucao for um WORD, transforma o operando em numero
		if(instrucao.code == WORD) {
		    flag_operando = 1;
            instrucao.operando = atoi(instruction_operand);
		}
		//Se nao for nenhum registrador, eh algum label
		if(!flag_operando) {
			strcpy(instrucao.label, instruction_operand);	
		}
	}

	return instrucao;
}
/*****************************************
 * Realiza a segunda passada do montador *
 * traduzindo as instrucoes e os labels  *
 * e escrevendo o arquivo binario.       *
 *****************************************/
void montador(FILE* entrada, FILE* saida, tabela_simbolos* tabela, int qnt_simbolos, int pc_final) {
	char linha [TAM_LINHA];
	int i, pc;

	tabela_simbolos* ts_atual = tabela;
	instruction instrucao;

	//Escreve quantos simbolos existem na tabela de simbolos
	fprintf(saida, "%d\n", qnt_simbolos);
	/*Escreve a tabela de simbolos na forma:
	  Simbolo endereco'\n'
	 */
	for(i = 0; i < qnt_simbolos; i++) {
		fprintf(saida, "%s %d\n", ts_atual->label, ts_atual->pc);
		ts_atual = ts_atual->prox;
	}
	//Escreve quantos espacos de memoria sao utilizados
	fprintf(saida, "%d", pc_final);

	//"Rebobina" o arquivo para comecar a ler do inicio novamente
	fseek(entrada, 0, SEEK_SET);
	pc = 0;
	//Percorre completamente o arquivo de entrada
	while(!feof(entrada)) {
		//Faz a leitura de cada linha do arquivo de entrada
		fgets(linha, TAM_LINHA, entrada);
		instrucao = decodifica_instrucao(linha, tabela, pc);
		//Se nao existe a instrucao, passa para a proxima linha
		if(instrucao.code == INVALIDA) continue;
		//Se a instrucao indica que o codigo acabou, sai do loop
		if(instrucao.code == END) break;
		//Atualiza o pc e a escreve no arquivo de saida de acordo com a instrucao
		//Se a instrucao for de apenas 4 bytes:
		if(instrucao.code == READ || instrucao.code == WRITE || instrucao.code == NOT || instrucao.code == PUSH || instrucao.code == POP || instrucao.code == RET || instrucao.code == DUMP || instrucao.code == HALT) {
			fprintf(saida, "\n%d", instrucao.code);
			pc++;
		}
		//Se for a instrucao WORD
		else if(instrucao.code == WORD) {
			fprintf(saida, "\n24\n%d", instrucao.operando);
			pc++;
		}
		//Se for de 8 bytes:
		else if(instrucao.code != INVALIDA) {
			//Se o operando for um registrador
			if(instrucao.code == COPY || instrucao.code == XCH || instrucao.code == ADD || instrucao.code == SUB || instrucao.code == AND || instrucao.code == OR || instrucao.code == XOR ) {
				fprintf(saida, "\n%d\n%d", instrucao.code, instrucao.operando);
			}
			//Se o operando for um label
			else if(instrucao.code == LOAD || instrucao.code == STORE || instrucao.code == JMP || instrucao.code == JZ || instrucao.code == JNZ || instrucao.code == JN || instrucao.code == JNN || instrucao.code == CALL) {
				fprintf(saida, "\n%d\n%s", instrucao.code, instrucao.label);
			}
			pc += 2;
		}
	}
}

int main(int argc, char** argv) {
	FILE* entrada;
	FILE* saida;
	tabela_simbolos tabela;
	int verbose = 0;
	int qnt_simbolos = 0;
	int pc_final = 0;

	tabela.prox = NULL;
	if(inicializa_arquivos(argc, argv, &entrada, &saida, &verbose)) return 0;
	cria_tabela_de_simbolos(entrada, &tabela, verbose, &qnt_simbolos, &pc_final);

	montador(entrada, saida, &tabela, qnt_simbolos, pc_final);

	fclose(entrada);
	fclose(saida);
	return 0;
}
