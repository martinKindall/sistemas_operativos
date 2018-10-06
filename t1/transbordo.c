#include <nSystem.h>
#include "fifoqueues.h"
#include <stdio.h>
#include "transbordo.h"


typedef struct{
	int v;
	int listo;
} Vehiculo;

nMonitor mon;

int transbordadores;
int *disponiblesPargua, *disponiblesChacao;

FifoQueue esperandoPargua;
FifoQueue esperandoChacao;


void inicializar(int p){

	mon= nMakeMonitor();

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
			disp = i;
			break;
		}
	}

	return disp;
}


void setDisponible(int* disponibles, int idx, int val)
{
	disponibles[idx] = val;
}


void logicaTransbordo(int v, int* disponiblesEstaOrilla, int* disponiblesOrillaOpuesta, FifoQueue esperandoAca, FifoQueue esperandoAlla, void (*haciaAlla)(int p1, int v1), void (*haciaAca)(int p2, int v2)){
	Vehiculo vehiculo;
	vehiculo.v = v;
	vehiculo.listo = FALSE;

	nEnter(mon);
	PutObj(esperandoAca, &vehiculo);

	while(!vehiculo.listo)
	{
		int dispAca = getDisponible(disponiblesEstaOrilla);
		int dispAlla = getDisponible(disponiblesOrillaOpuesta);

		if (dispAca > -1)
		{
			setDisponible(disponiblesEstaOrilla, dispAca, FALSE);
			Vehiculo* vehicAca= GetObj(esperandoAca);
			nExit(mon);
			haciaAlla(dispAca, vehicAca->v);
			nEnter(mon);
			setDisponible(disponiblesOrillaOpuesta, dispAca, TRUE);
			vehicAca->listo = TRUE;
			nNotifyAll(mon);

			if (vehicAca->v == v)
			{
				nExit(mon);
				return;
			}

			nWait(mon);
		}
		else if (dispAlla > -1){
			setDisponible(disponiblesOrillaOpuesta, dispAlla, FALSE);
			Vehiculo* vehicAlla= GetObj(esperandoAlla);
			nExit(mon);

			if (vehicAlla == NULL){
				haciaAca(dispAlla, -1);
			}
			else{
				haciaAca(dispAlla, vehicAlla->v);
			}
			nEnter(mon);
			setDisponible(disponiblesEstaOrilla, dispAlla, TRUE);

			if (vehicAlla != NULL){
				vehicAlla->listo = TRUE;
			}

			continue;
		}
		else{
			nWait(mon);
		}
	}

	nExit(mon);
	return;
}

void transbordoAChacao(int v){
	logicaTransbordo(v, disponiblesPargua, disponiblesChacao, esperandoPargua, esperandoChacao, haciaChacao, haciaPargua);
}


void transbordoAPargua(int v){
	logicaTransbordo(v, disponiblesChacao, disponiblesPargua, esperandoChacao, esperandoPargua, haciaPargua, haciaChacao);
}

void finalizar(){}

// int nMain(int argc, char **argv)
// {
// 	inicializar(4);

// 	for (int i = 0; i < transbordadores; ++i)
// 	{
// 		nPrintf("%d\n", disponiblesPargua[i]);
// 		nPrintf("%d\n", disponiblesChacao[i]);
// 		nPrintf("\n");
// 	}
// }