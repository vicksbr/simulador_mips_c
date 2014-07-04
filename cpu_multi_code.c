#include <stdio.h>
#include <stdlib.h>

extern int memoria[];
extern int reg[];
extern char loop;
extern void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow);

char returnSignal = 0;

//variavel pra testes
char pausa;

void UnidadeControle(int IR, int *sc){

	int opcode = IR >> 26; 
	opcode = opcode&0x3f; //filtrando para tratar negativo

	switch(*sc) {
		
		case 0:				 //Inicializador
			*sc = 0x44020;	 //0
		break;

		case 0x44020:		 //0
			*sc = 0x30;		 //1
		break;

		case 0x30:			//1
			
			//Tabela de despacho
			/*
				2, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16
			*/
			if(opcode == 0){
				*sc = 0x88;	//6
			}
			else if(opcode == 0x2B || opcode == 0x23){
				*sc = 0x18;	//2
			}
			else{
				switch(opcode){
					case 0x4:	//BEQ
						*sc = 0xD0C;	//8
					break;

					case 0x2:	//j
						*sc = 0x1A00;	//9
					break;

					case 0x5:	//BNE
						*sc = 0xD08;	//10
					break;

					case 0x1:	//BLTZ
						*sc = 0xCF4;	//11
					break;

					case 0x3f:	//move
						*sc = 0x10002;	//12
					break;

					case 0x0f:	//LUI
						*sc = 0x30002;	//13
					break;

					case 0x0d:	//ORI
						*sc = 0x58;	//14
					break;

					case 0x0c:	//ANDI
						*sc = 0x198;	//15
					break;

					case 0x0a:	//SLTI
						*sc = 0x158;	//16
					break;

				}
				
			}
		break;

		case 0x18:	//2
			//Tabela de despacho
			/*
				opcode 0x2B -> 3
				opcode 0x23 -> 5
			*/
			if(opcode == 0x23){
				*sc = 0x6000;	//3
			}
			else{
				*sc = 0xA000;	//5
			}

		break;

		case 0x6000:	//3
			*sc = 0x20002;	//4
		break;

		case 0x20002:	//4
			*sc = 0x44020;	//0
		break;

		case 0xA000:	//5
			*sc = 0x44020;	//0
		break;

		case 0x88:	//6
			*sc = 0x3;	//7
		break;

		case 0x3:	//7
			*sc = 0x44020;	//0
		break;

		case 0xD0C:	//8
			*sc = 0x44020;	//0
		break;

		case 0x1A00:	//9
			*sc = 0x44020;	//0
		break;

		case 0xD08:	//10
			*sc = 0x44020;	//0
		break;

		case 0xCF4:	//11
			*sc = 0x44020;	//0
		break;

		case 0x10002:	//12
			*sc = 0x44020;	//0
		break;

		case 0x30002:	//13
			*sc = 0x44020;	//0
		break;

		case 0x58:	//14
			*sc = 0x3;	//7
		break;

		case 0x198:	//15
			*sc = 0x3;	//7
		break;

		case 0x158:	//16
			*sc = 0x3;	//7
		break;

	}

	printf("estado: %d\n", *sc);

}



void Busca_Instrucao(int sc, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew){
	
	char zero, overflow;
	if((sc & 0x40000) !=0) {			 //Se IRWrite == 1
		if((sc & 0x2000) == 0) {		 //Se IorD == 0, a memória é acessada na posição de PC
			*IRnew = memoria[PC/4];
		}
		else{						//Se IorD == 1, a memória é acessada na posição de ALUout
			*IRnew = memoria[ALUOUT/4];
		}
		*MDRnew = *IRnew << 16;
		*MDRnew = *MDRnew >> 16;
		ula(PC, 0x4, 0x2, PCnew, &zero, &overflow);
	}
}



void Decodifica_BuscaRegistrador(int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew){
	
	char zero, overflow;
	
	if(sc==0x30) {	//Realiza a 
		if(IR == 0) {
			loop = 0;
			printf("HALT!\n");
			returnSignal = 1;
			return;
		}
		else {
			*Anew = IR >> 21;		//*Anew recebe os 5 bits referentes ao registrador RS
			*Anew = *Anew & 0x1f;

			*Bnew = IR >> 16;		//*Anew recebe os 5 bits referentes ao registrador RT
			*Bnew = *Bnew & 0x1f;

			IR = IR << 16;
			IR = IR >> 14;
			ula(PC, IR, 0x2, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT
		}
	}
}



void Execucao_CalcEnd_Desvio(int sc, int A, int B, int IR, int PC, int ALUOUT, int *ALUOUTnew, int *PCnew){

	char zero, overflow;
	int op;
	
	int VI = IR << 16;
	VI = VI >> 16;
	 
	if(returnSignal == 1) {
		return;
	}

	if(sc == 0x88){		//Se for tipo-r
						
		op = (IR & 0x3f);
		switch(op) {
			case 0x20:	//add
				ula(reg[A], reg[B], 0x2, ALUOUTnew, &zero, &overflow);
			break;

			case 0x22:	//sub
				ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
			break;

			case 0x24:	//and
				ula(reg[A], reg[B], 0x0, ALUOUTnew, &zero, &overflow);
			break;

			case 0x25:	//or
				ula(reg[A], reg[B], 0x1, ALUOUTnew, &zero, &overflow);
			break;

			case 0x2a:	//slt
				ula(reg[A], reg[B], 0x7, ALUOUTnew, &zero, &overflow);
			break;
		}
	}
	else if(sc == 0x18){	//lw ou sw
		
		ula(reg[A], VI, 0x2, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT
	}
	else if(sc == 0xCF4) {	//BLTZ
		ula(reg[A], 0, 0x7, ALUOUTnew, &zero, &overflow);
		
		if(*ALUOUTnew == 1) {
			*PCnew = ALUOUT;
		}
	}
	else if(sc == 0xD0C) {	//BEQ
		ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
		
		if(zero == 1) {
			*PCnew = ALUOUT;
		}
	}
	else if(sc == 0xD08) {	//BNE
		
		ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
		
		if(zero == 0) {
			*PCnew = ALUOUT;
		}
	}
	else if(sc == 0x1A00) {	//j
		
		VI = IR&0x3ffffff;	//realiza a extensão de sinal
		VI = VI << 2;
		*PCnew = VI;
	}
	else if(sc == 88) {		//ori
		ula(reg[A], VI, 0x1, ALUOUTnew, &zero, &overflow);
	}
	else if(sc == 0x198) {	//andi
		ula(reg[A], VI, 0x0, ALUOUTnew, &zero, &overflow);
	}
	else if(sc == 0x158) {	//slti
		ula(reg[A], VI, 0x7, ALUOUTnew, &zero, &overflow);
	}
	else if(sc == 0x10002) {	//move
		reg[A] = reg[B];
	}
	else if(sc == 0x30002) {	//lui
		reg[B] = IR << 16;
	}
}



void EscreveTipoR_AcessaMemoria(int sc, int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew){
	int RD;
	if(returnSignal==1) {
		return;
	}
	if(sc == 3) {	//7
		if(IR >> 27 == 0) {	//Tipo-R
			RD = IR >> 11;
			RD = RD&0x1f;
			reg[RD] = ALUOUT;
		}
		else{	//Tipo-I
			reg[B] = ALUOUT;
		}
		printf("\t\tresultado da ula: %d\n", ALUOUT);
	}
	else if(sc == 0x6000) {	//3
		(*MDRnew) = (memoria[ALUOUT/4]) << 16;	//LW
		(*MDRnew) = (*MDRnew) >> 16;	//LW
		printf("----------ALUOUT: %d\n", ALUOUT);
		printf("MDRnew: %d\n", *MDRnew);
	}
	else if(sc == 0xA000) {	//5
		memoria[ALUOUT/4] = reg[B];	//SW
	}
}



void EscreveRefMem(int sc, int IR, int MDR, int ALUOUT){
	if(returnSignal == 1) {
		return;
	}
	int B = IR >> 16;		//*Anew recebe os 5 bits referentes ao registrador RT
	B = B & 0x1f;
	if(sc == 0x20002) {
		reg[B] = MDR;
	}
}



