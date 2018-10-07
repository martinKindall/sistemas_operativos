#include <nSystem.h>
#include "fifoqueues.h"
#include <stdio.h>
#include "transbordo.h"


typedef struct{
	int v;
	int listo;
	int enViaje;
	nCondition cond;
} Vehiculo;

nMonitor mon;

int esperando;

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

	esperando = 0;
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
	vehiculo.cond = nMakeCondition(mon);

	nEnter(mon);
	PutObj(esperandoAca, &vehiculo);

	while(!vehiculo.listo)
	{
		int dispAca = getDisponible(disponiblesEstaOrilla);
		int dispAlla = getDisponible(disponiblesOrillaOpuesta);

		Vehiculo* vehicAca= GetObj(esperandoAca);

		if (dispAca > -1 && vehicAca->v == v)
		{
			setDisponible(disponiblesEstaOrilla, dispAca, FALSE);
			nExit(mon);
			haciaAlla(dispAca, v);
			nEnter(mon);
			setDisponible(disponiblesOrillaOpuesta, dispAca, TRUE);
			nNotifyAll(mon);
			nExit(mon);
			return;
		}
		else if (dispAca > -1 && vehicAca->v != v){
			nSignalCondition(vehicAca->cond);
		}
		
		if (dispAlla > -1){
			Vehiculo* vehicAlla= GetObj(esperandoAlla);

			if (vehicAlla != NULL){
				nSignalCondition(vehicAlla->cond);
				PushObj(esperandoAlla, vehicAlla);
			}else{
				setDisponible(disponiblesOrillaOpuesta, dispAlla, FALSE);
				nExit(mon);
				haciaAca(dispAlla, -1);
				nEnter(mon);
				setDisponible(disponiblesEstaOrilla, dispAlla, TRUE);
				if (vehicAca->v == v){
					PushObj(esperandoAca, vehicAca);
					continue;
				}else{
					nSignalCondition(vehicAca->cond);
					PushObj(esperandoAca, vehicAca);
				}
			}
		}

		nWaitCondition(vehiculo.cond);
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

void finalizar(){
	nFree(disponiblesPargua);
	nFree(disponiblesChacao);
}