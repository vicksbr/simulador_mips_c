/* Arquivo cpu_mips_multiciclo_2014.c
   Autor: Paulo Sergio Lopes de Souza

Observacoes:
(1)  Trabalho 2 - SSC0112 - Organizacao de Computadores Digitais I
(2)  Este codigo fonte esta sendo disponibilizado aos alunos pelo professor
     e deve ser utilizado por todos os grupos. Nao o altere. Ele sera usado
	 para a correcao do trabalho.
(3)  Para realizar o seu trabalho, edite um arquivo texto chamado
     cpu_multi_code.c  Insira nele todas as funcionalidades necessarias ao
	 seu trabalho.
(4) Escreva, obrigatoriamente, como comentario nas primeiras linhas do arquivo
    cpu_multi_code.c, os nomes dos alunos integrantes do grupo que efetivamente
	contribuiram para o desenvolvimento deste trabalho.
(5) Antes de submeter o seu trabalho, compacte os arquivos no  padr�o zip,
       gerando o arquivo  TB2-TurmaX-GrupoYY.zip (X indica o nr da turma (1, 2 ou 3)
       e YY indica o nr do grupo na turma).
(6) Submeta viz Moodle do STOA/USP o arquivo TB2-TurmaX-GrupoYY.zip
*/

#include <stdio.h>
#include <stdlib.h>

#define MAX 1024
#define NUMREG 32

/***********************
prototipacao inicial
 ***********************/

int main (int, char **);

/* ULA segue a especifica��o dada em sala de aula.
void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow)
args de entrada:		int a, int b, char ula_op
args de saida: 		int *result_ula, char *zero, char *overflow */
void ula(int, int, char, int *, char *, char *);

/* UC principal
void UnidadeControle(int IR, int *sc);
args de entrada:		 int IR
args de saida: 		int *sc     */
void UnidadeControle(int, int *);

/* Busca da Instrucao
void Busca_Instrucao(int sc, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew);
args de entrada:		int sc, int PC, int ALUOUT, int IR, int A, int B
args de saida: 		int *PCnew, int *IRnew, int *MDRnew     */
void Busca_Instrucao(int, int, int, int, int, int, int*, int*, int*);

/* Decodifica Instrucao, Busca Registradores e Calcula Endereco para beq
void Decodifica_BuscaRegistrador(int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew);
args de entrada:		int sc, int IR, int PC, int A, int B,
args de saida: 		int *Anew, int *Bnew, int *ALUOUTnew     */
void Decodifica_BuscaRegistrador(int, int, int, int, int, int *, int *, int *);

/*Executa TipoR, Calcula endereco para lw/sw e efetiva desvio condicional e incondicional
void Execucao_CalcEnd_Desvio(int sc, int A, int B, int IR, int PC, int ALUOUT, int *ALUOUTnew, int *PCnew);
args de entrada:		int sc, int A, int B, int IR, int PC, int ALUOUT
args de saida: 		 int *ALUOUTnew, int *PCnew */
void Execucao_CalcEnd_Desvio(int, int, int, int, int, int, int *, int *);

/* Escreve no Bco de Regs resultado TiporR, Le memoria em lw e escreve na memoria em sw
void EscreveTipoR_AcessaMemoria(int sc, int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew);
args de entrada:	 int sc, int B, int IR, int ALUOUT, int PC
args de saida: 	 int *MDRnew, int *IRnew */
void EscreveTipoR_AcessaMemoria(int, int, int, int, int, int *, int *);

/* Escreve no Bco de Regs o resultado da leitura da memoria feita por lw
void EscreveRefMem(sort int sc, int IR, int MDR, int ALUOUT);
args de entrada:		int sc, int IR, int MDR, int ALUOUT
args de saida: 		nao ha */
void EscreveRefMem(int, int, int, int);


/****************************
definicao de variaveis globais
 ****************************/
int memoria[MAX];		// Memoria RAM
int reg[NUMREG];		// Banco de Registradores

char loop = 1; 		// variavel auxiliar que determina a parada da execucao deste programa

void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow)
{
		*overflow = 0;
		switch (ula_op)
		{
      case 0:    // and
				*result_ula = a & b;
				break;

      case 1:   // or
				*result_ula = a | b;
				break;

			case 2:  // add
				*result_ula = a + b;
				if ( (a >= 0 && b >= 0 && *result_ula < 0) || (a < 0 && b < 0 && *result_ula >= 0) )
					*overflow  = 1;
				break;

      case 6:   // sub
				*result_ula = a - b;
				if ( (a >= 0 && b < 0 && *result_ula < 0) || (a < 0 && b >= 0 && *result_ula >= 0) )
					*overflow  = 1;
				break;

      case 7:   // slt
				if(a < b)
					*result_ula = 1;
				else
					*result_ula = 0;
				break;

      case 12:   // nor
				*result_ula = ~(a | b);
				break;
		}
		if (*result_ula == 0)
			*zero = 1;
		else
			*zero = 0;

		return;
} // fim da ula

// contem todas as funcoes desenvolvidas por voce para o processador multiciclo.
//Inclua aqui as suas funcoes/procedimentos
#include "cpu_multi_code.c"

int main (int argc, char *argv[])
{
	int PCnew = 0, IRnew, MDRnew, Anew, Bnew, ALUOUTnew;
					// Registradores auxiliares usados para a escrita dentro de um ciclo.
					// Guardam temporariamente o resultado durante um ciclo. Os resultados aqui armazenados estarao
					// disponiveis para leitura no final do ciclo atual (para que o mesmo esteja disponivel apenas no
					// incio do ciclo seguinte).
					// Em um ciclo sempre e lido o conteudo de um registrador atualizado no ciclo anterior.

	int PC = 0, IR=-1, MDR, A, B, ALUOUT;
					// Registradores especiais usados para a leitura em um ciclo.
					// Guardam os resultados que poderao ser utilizados ja neste ciclo, pois foram atualizados no final
					// do ciclo anterior.
					// Ex.: em um ciclo, o resultado da ULA e inserido inicialmente em ALUOUTnew.  Apenas no final
					// do ciclo esse conteudo podera ser atribuido para ALUOUT, para que o mesmo possa ser
					// usado no ciclo seguinte.
					// Em um ciclo sempre e lido o conteudo de um registrador atualizado no ciclo anterior.

	int sc = 0;	   			// Sinais de Controle
					//                    cada bit determina um dos sinais de controle que saem da UC.
					// A posicao de cada sinal dentro do int esta especificada no enunciado
	char ula_op = 0;   			// Sinais de Controle de entrada para a ULA
					//                    sao usados apenas os 4 bits menos significativos dos 8 disponiveis.

	int nr_ciclos = 0; 			// contador do numero de ciclos executados

/*
As variaveis zero e overflow nao precisam definidas na main.
Serao argumentos de retorno da ula e devem ser definidas localmente nas
rotinas adequadas.
char  zero, overflow;  			// Sinais de Controle de saida da UL: bit zero e bit para indicar overflow
*/

	memoria[0] = 0x8c480000;  // 1000 1100 01007 1000 0000 0000 0000 0000  lw $t0, 0($v0)								5
	memoria[1] = 0x010c182a;  // 0000 0001 0000 1100 0001 1000 0010 1010   slt $v1, $t0, $t4								4
	memoria[2] = 0x106d0004;  // 0001 0000 0110 1101 0000 0000 0000 0100  beq $v1, $t5, fim(4 palavras abaixo de PC+4)	3
	memoria[3] = 0x01084020;  // 0000 0001 0000 1000 0100 0000 0010 0000  add $t0, $t0, $t0								4
	memoria[4] = 0xac480000;  // 1010 1100 0100 1000 0000 0000 0000 0000  sw $t0, 0($v0)								4
	memoria[5] = 0x004b1020;  // 0000 0000 0100 1011 0001 0000 0010 0000  add $v0, $t3, $v0								4
	memoria[6] = 0x08000000;  // 0000 1000 0000 0000 0000 0000 0000 0000  j inicio (palavra 0)								3
	memoria[7] = 0;           // fim (criterio de parada do programa)    (27*6)+(5+4+3)+1
	memoria[8] = 0;
	memoria[9] = 0;

	// Dados
	memoria[20] = 10;
	memoria[21] = 12;
	memoria[22] = 14;
	memoria[23] = 16;
	memoria[24] = 18;
	memoria[25] = 20;
	memoria[26] = -1;

	reg[2]  = 80;  // $v0

	reg[11] = 4;  // $t3
	reg[12] = 0;  // $t4
	reg[13] = 1;  // $t5

	while(loop){
		// aqui comeca um novo ciclo

		// abaixo estao as unidades funcionais que executarao em todos os ciclos
		// os sinais de controle em sc impedirao/permitirao que a execucao seja, de fato, efetivada

		UnidadeControle(IR, &sc);
		Busca_Instrucao(sc, PC, ALUOUT, IR, A, B, &PCnew, &IRnew, &MDRnew);
		Decodifica_BuscaRegistrador(sc, IR, PC, A, B, &Anew, &Bnew, &ALUOUTnew);
		Execucao_CalcEnd_Desvio(sc, A, B, IR, PC, ALUOUT, &ALUOUTnew, &PCnew);
		EscreveTipoR_AcessaMemoria(sc, B, IR, ALUOUT, PC, &MDRnew, &IRnew);
		EscreveRefMem(sc, IR, MDR, ALUOUT);

		// contador que determina quantos ciclos foram executados
		nr_ciclos++;

		// atualizando os registradores temporarios necessarios ao proximo ciclo.
		PC	 	= PCnew;
		IR		= IRnew;
		MDR		= MDRnew;
		A		= Anew;
		B		= Bnew;
		ALUOUT 	= ALUOUTnew;

		// aqui termina um ciclo
	} // fim do while(loop)

	// impressao da memoria para verificar se a implementacao esta correta
	{
		int ii;
		for (ii = 20; ii < 27; ii++) {
			printf("memoria[%d]=%d \n", ii, memoria[ii]);
		}
		printf("Nr de ciclos executados =%d \n", nr_ciclos);
	}
	exit(0);
} 

// fim de main
// ***********************************************************************************************
