#include <nSystem.h>
#include <stdio.h>
#include "transbordo.h"


typedef struct{
	int v;
	int listo;
} Vehiculo;


int transbordadores;
int *disponiblesPargua, *disponiblesChacao;

FifoQueue* esperandoPargua;
FifoQueue* esperandoChacao;


void inicializar(int p){

	transbordadores = p;
	disponiblesPargua = nMalloc(sizeof(int)*p);
	disponiblesChacao = nMalloc(sizeof(int)*p);

	for (int i = 0; i < p; ++i){
		disponiblesPargua[i] = 1;
		disponiblesChacao[i] = 0;
	}

	esperandoPargua = MakeFifoQueue();
	esperandoChacao = MakeFifoQueue();
}


int getDisponible(int* disponibles)
{
	int disp = -1;

	for (int i = 0; i < transbordadores; ++i){
		if (disponibles[i] == 1){
			disp = disponibles[i];
			break;
		}
	}

	return disp;
}

void setDisponible(int* disponibles, int idx, int val)
{
	disponibles[idx] = val;
}


void transbordoAChacao(int v){
	Vehiculo vehiculo;
	vehiculo.v = v;
	vehiculo.listo = FALSE;

	nEnter(m);
	PutObj(esperandoPargua, *vehiculo);

	while(!vehiculo->listo)
	{
		int dispPargua = getDisponible(disponiblesPargua);
		int dispChacao = getDisponible(disponiblesChacao);

		if (dispPargua > -1)
		{
			setDisponible(disponiblesPargua, dispPargua, FALSE);
			Vehiculo* vehicPargua= GetObj(esperandoPargua);
			nExit(m);
			haciaChacao(dispPargua, vehicPargua->v);
			nEnter(m);
			setDisponible(disponiblesChacao, dispPargua, TRUE);
			vehicPargua->listo = TRUE;
			nNotifyAll(m);

			if (vehicPargua->v == v)
			{
				nExit(m);
				return v;
			}

			nWait(m);
		}
		else if (dispChacao > -1){
			setDisponible(disponiblesChacao, dispChacao, FALSE);
			Vehiculo* vehicChacao= GetObj(esperandoChacao);
			nExit(m);

			if (vehicChacao == NULL){
				haciaPargua(dispChacao, -1);
			}
			else{
				haciaPargua(dispChacao, vehicChacao->v);
			}
			nEnter(m);
			setDisponible(disponiblesPargua, dispChacao, TRUE);

			if (vehicChacao != NULL){
				vehicChacao->listo = TRUE;
			}

			nNotifyAll(m);
			nWait(m);
		}
		else{
			nWait(m);
		}
	}

	nExit(m);
	return v;
}


void transbordoAPargua(int v){


}


void logicaTransbordo(int v, int* disponiblesEstaOrilla, int* disponiblesOrillaOpuesta){
	Vehiculo vehiculo;
	vehiculo.v = v;
	vehiculo.listo = FALSE;

	nEnter(m);
	PutObj(esperandoPargua, *vehiculo);

	while(!vehiculo->listo)
	{
		int dispPargua = getDisponible(disponiblesPargua);
		int dispChacao = getDisponible(disponiblesChacao);

		if (dispPargua > -1)
		{
			setDisponible(disponiblesPargua, dispPargua, FALSE);
			Vehiculo* vehicPargua= GetObj(esperandoPargua);
			nExit(m);
			haciaChacao(dispPargua, vehicPargua->v);
			nEnter(m);
			setDisponible(disponiblesChacao, dispPargua, TRUE);
			vehicPargua->listo = TRUE;
			nNotifyAll(m);

			if (vehicPargua->v == v)
			{
				nExit(m);
				return v;
			}

			nWait(m);
		}
		else if (dispChacao > -1){
			setDisponible(disponiblesChacao, dispChacao, FALSE);
			Vehiculo* vehicChacao= GetObj(esperandoChacao);
			nExit(m);

			if (vehicChacao == NULL){
				haciaPargua(dispChacao, -1);
			}
			else{
				haciaPargua(dispChacao, vehicChacao->v);
			}
			nEnter(m);
			setDisponible(disponiblesPargua, dispChacao, TRUE);

			if (vehicChacao != NULL){
				vehicChacao->listo = TRUE;
			}

			nNotifyAll(m);
			nWait(m);
		}
		else{
			nWait(m);
		}
	}

	nExit(m);
	return v;
}


int nMain(int argc, char **argv)
{
	inicializar(4);

	for (int i = 0; i < transbordadores; ++i)
	{
		printf("%d\n", disponiblesPargua[i]);
		printf("%d\n", disponiblesChacao[i]);
		printf("\n");
	}
}