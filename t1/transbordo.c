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

	int cambios = FALSE;

	while(!vehiculo.listo)
	{
		int dispAca = getDisponible(disponiblesEstaOrilla);
		int dispAlla = getDisponible(disponiblesOrillaOpuesta);

		nPrintf("paso1 p: %d, pp: %d, v: %d, estado: %d\n", dispAca, dispAlla, v, vehiculo.listo);
		if (dispAca > -1)
		{
			setDisponible(disponiblesEstaOrilla, dispAca, FALSE);
			
			// if (EmptyFifoQueue(esperandoAca)){
			// 	// nPrintf("paso2 p: %d, pp: %d, v: %d, estado: %d\n", dispAca, dispAlla, v, vehiculo.listo);
			// }

			Vehiculo* vehicAca= GetObj(esperandoAca);
			// nPrintf("paso2 p: %d, pp: %d, v: %d, estado: %d, sacado: %d\n", dispAca, dispAlla, v, vehiculo.listo, vehicAca->v);

			if (vehicAca == NULL){
				vehicAca = &vehiculo;
			}

			if (cambios){
				cambios = FALSE;
				nNotifyAll(mon);
			}

			nExit(mon);
			haciaAlla(dispAca, vehicAca->v);
			nEnter(mon);
			setDisponible(disponiblesOrillaOpuesta, dispAca, TRUE);
			vehicAca->listo = TRUE;

			nPrintf("paso3 p: %d, v: %d, sacado: %d, estado: %d\n", dispAca, v, vehicAca->v, vehiculo.listo);
			if (vehiculo.listo)
			{
				nNotifyAll(mon);
				nExit(mon);
				return;
			}
			cambios = TRUE;
		}
		else if (dispAlla > -1){
			setDisponible(disponiblesOrillaOpuesta, dispAlla, FALSE);
			Vehiculo* vehicAlla= GetObj(esperandoAlla);

			if (cambios){
				nNotifyAll(mon);
			}

			if (vehicAlla != NULL){
				nPrintf("paso5 p: %d, pp: %d, v: %d, estado: %d, sacado: %d\n", dispAca, dispAlla, v, vehiculo.listo, vehicAlla->v);
			}

			nPrintf("paso5 p: %d, pp: %d, v: %d, estado: %d\n", dispAca, dispAlla, v, vehiculo.listo);
			nExit(mon);
			if (vehicAlla == NULL){
				haciaAca(dispAlla, -1);
			}
			else{
				haciaAca(dispAlla, vehicAlla->v);
			}
			nEnter(mon);
			nPrintf("paso4 p: %d, pp: %d, v: %d, estado: %d\n", dispAca, dispAlla, v, vehiculo.listo);

			setDisponible(disponiblesEstaOrilla, dispAlla, TRUE);
			if (vehicAlla != NULL){
				vehicAlla->listo = TRUE;
				// nPrintf("paso4 p: %d, pp: %d, v: %d, estado: %d, sacado: %d\n", dispAca, dispAlla, v, vehiculo.listo, vehicAlla->v);
			}
			// nPrintf("paso2 p: %d, v: %d\n", dispAlla, v);
			cambios = TRUE;
		}
		else{
			if (cambios){
				cambios = FALSE;
				nNotifyAll(mon);
			}

			nWait(mon);
		}
	}
	if (cambios){
		nPrintf("cayo aca");
	}
	nNotifyAll(mon);
	nExit(mon);
	return;
}

void transbordoAChacao(int v){
	logicaTransbordo(v, disponiblesPargua, disponiblesChacao, esperandoPargua, esperandoChacao, haciaChacao, haciaPargua);
}


void transbordoAPargua(int v){
	logicaTransbordo(v, disponiblesChacao, disponiblesPargua, esperandoChacao, esperandoPargua, haciaPargua, haciaChacao);
}

void finalizar(){
	nFree(disponiblesPargua);
	nFree(disponiblesChacao);
}

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