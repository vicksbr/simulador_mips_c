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
			*sc = 0x44020;	 //Estado 0
		break;

		case 0x44020:		 //Estado 0    
			*sc = 0x30;		 //Estado 1  ... Estado0->Estado1
		break;

		case 0x30:			//Estado 1
			
			/*

			Tabela de despacho(fetch) de acordo com o microcódigo 
			
			decode 		     (2)
			exec   		     (6)
			branch 		     (8)
			jump   		 	 (9)								  
			bne		0x05	 (10)
			bltz	0x01	 (11)
			move	0X3F	 (12)
			lui		0X0F	 (13)
			ori		0X0D	 (14)
			andi	0X0C	 (15)
			slti	0X0A	 (16)
			
			*/
		
			if(opcode == 0){
				*sc = 0x88;	//Estado 6 - Se for Tipo-R    --- Estado1 -> Estado6
			}
			else if(opcode == 0x2B || opcode == 0x23){
				*sc = 0x18;	//Estado 2 - Se for lw/sw  	   ---- Estado1 -> Estado2
			}
			else{
				switch(opcode){
					case 0x4:	
						*sc = 0xD0C;	//Estado 8 beq 	   --- Estado1 -> Estado8
						break;

					case 0x2:	
						*sc = 0x1A00;	//Estado 9 jump(j) --- Estado1 -> Estado9
						break;

					case 0x5:	
						*sc = 0xD08;	//Estado 10 bne    --- Estado1 -> Estado10
						break;

					case 0x1:	
						*sc = 0xCF4;	//Estado 11 bltz   --- Estado1 -> Estado11
						break;

					case 0x3f:	
						*sc = 0x10002;	//Estado 12 move   --- Estado1 -> Estado12
						break;

					case 0x0f:	
						*sc = 0x30002;	//Estado 13 lui    --- Estado1 -> Estado13
						break;

					case 0x0d:	
						*sc = 0x58;	    //Estado 14 ori	   --- Estado1 -> Estado14
						break;

					case 0x0c:	
						*sc = 0x198;	//Estado 15 andi   --- Estado1 -> Estado15
						break;

					case 0x0a:	
						*sc = 0x158;	//Estado 16 slti   ---Estado1 -> Estado16
						break;
				}
				
			}
		break;

		case 0x18:	//Estado2 (lw ou sw)
			/*
			
			Tabela de despacho(fetch) de acordo com o microcódigo 

				lw 	0x2B 	(3)
				sw 	0x23 	(5)
				
			*/	
			if(opcode == 0x23) {
				*sc = 0x6000;	//Estado2 -> Estado3
			}
			else {
				*sc = 0xA000;	//Estado2 -> Estado5
			}
			break;

		case 0x6000:	
			*sc = 0x20002; 	   //Estado3 -> Estado4
			break;

		case 0x20002:	
			*sc = 0x44020;	   //Estado4 -> Estado0
			break;

		case 0xA000:	
			*sc = 0x44020;	   //Estado5 -> Estado0
			break;

		case 0x88:			   //Estado6 -> Estado7
			*sc = 0x3;	
			break;

		case 0x3:	
			*sc = 0x44020;	   //Estado7 -> Estado0
			break;

		case 0xD0C:		
			*sc = 0x44020;    //Estado8 -> Estado0
			break;

		case 0x1A00:	 
			*sc = 0x44020;	  //Estado9 -> Estado0
			break;

		case 0xD08:	
			*sc = 0x44020;	 //Estado10 -> Estado0
			break;

		case 0xCF4:	
			*sc = 0x44020;	////Estado11 -> Estado0
			break;

		case 0x10002:	
			*sc = 0x44020;	//Estado12 -> Estado0
			break;

		case 0x30002:		//Estado13 -> Estado0
			*sc = 0x44020;	
			break;

		case 0x58:		   //Estado14 -> Estado7
			*sc = 0x3;	
			break;

		case 0x198:	      //Estado15 -> Estado7
			*sc = 0x3;	
			break;

		case 0x158:	     //Estado16 -> Estado7
			*sc = 0x3;	
			break;

	}

	//teste
	printf("estado: %d\n", *sc);

}



void Busca_Instrucao(int sc, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew){
	
	char zero, overflow;
	if((sc & 0x40000) !=0) {			 //Se IRWrite == 1
		if((sc & 0x2000) == 0) {		 //Se IorD == 0, a memória é acessada na posição de PC
			*IRnew = memoria[PC/4];
		}
		else{							//Se IorD == 1, a memória é acessada na posição de ALUout
			*IRnew = memoria[ALUOUT/4];
		}
		*MDRnew = *IRnew << 16;			//????
		*MDRnew = *MDRnew >> 16;
		ula(PC, 0x4, 0x2, PCnew, &zero, &overflow);
	}
}



void Decodifica_BuscaRegistrador(int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew){
	
	char zero, overflow;
	
	if(sc==0x30) {	//Se Estado1 (decode)
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

	else if(sc == 0x18) {	//lw ou sw
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



