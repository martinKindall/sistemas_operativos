#include <nSystem.h>
#include "fifoqueues.h"
#include <stdio.h>
#include "transbordo.h"
#include <string.h>


typedef struct{
	int v;
} Vehiculo;

nMonitor mon;

nCondition chacao;
nCondition pargua;

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

	chacao = nMakeCondition(mon);
	pargua = nMakeCondition(mon);
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


void logicaSignal(char* posicion, nCondition cond1, nCondition cond2){
	if (strcmp(posicion, "pargua") == 0){
		nSignalCondition(cond1);
	}else{
		nSignalCondition(cond2);
	}
}


void logicaHaciaDisponible(int* disponibles1, int* disponibles2, int disponibilidad, void (*hacia)(int p, int v), int vehiculo){
	setDisponible(disponibles1, disponibilidad, FALSE);
	nExit(mon);
	hacia(disponibilidad, vehiculo);
	nEnter(mon);
	setDisponible(disponibles2, disponibilidad, TRUE);
}


void logicaTransbordo(int v, int* disponiblesEstaOrilla, int* disponiblesOrillaOpuesta, FifoQueue esperandoAca, FifoQueue esperandoAlla, void (*haciaAlla)(int p1, int v1), void (*haciaAca)(int p2, int v2), char* posicion){
	Vehiculo vehiculo;
	vehiculo.v = v;

	nEnter(mon);
	PutObj(esperandoAca, &vehiculo);

	while(TRUE)
	{
		int dispAca = getDisponible(disponiblesEstaOrilla);
		int dispAlla = getDisponible(disponiblesOrillaOpuesta);

		Vehiculo* vehicAca= GetObj(esperandoAca);

		if (dispAca > -1 && vehicAca->v == v)
		{
			logicaHaciaDisponible(disponiblesEstaOrilla, disponiblesOrillaOpuesta, dispAca, haciaAlla, v);
			logicaSignal(posicion, chacao, pargua);
			nExit(mon);
			return;
		}
		else if (dispAca > -1 && vehicAca->v != v){
			PushObj(esperandoAca, vehicAca);
			logicaSignal(posicion, pargua, chacao);
		}
		else if (dispAlla > -1){
			Vehiculo* vehicAlla= GetObj(esperandoAlla);

			if (vehicAlla != NULL){
				PushObj(esperandoAlla, vehicAlla);
				PushObj(esperandoAca, vehicAca);
				logicaSignal(posicion, chacao, pargua);
			}else{
				logicaHaciaDisponible(disponiblesOrillaOpuesta, disponiblesEstaOrilla, dispAlla, haciaAca, -1);
				if (vehicAca->v == v){
					logicaHaciaDisponible(disponiblesEstaOrilla, disponiblesOrillaOpuesta, dispAlla, haciaAlla, v);
					logicaSignal(posicion, chacao, pargua);
					nExit(mon);
					return;
				}else{
					PushObj(esperandoAca, vehicAca);
					logicaSignal(posicion, pargua, chacao);
				}
			}
		}else{
			PushObj(esperandoAca, vehicAca);
		}

		if (strcmp(posicion, "pargua") == 0){
			nWaitCondition(pargua);
		}
		else{
			nWaitCondition(chacao);
		}
	}
}


void transbordoAChacao(int v){
	logicaTransbordo(v, disponiblesPargua, disponiblesChacao, esperandoPargua, esperandoChacao, haciaChacao, haciaPargua, "pargua");
}


void transbordoAPargua(int v){
	logicaTransbordo(v, disponiblesChacao, disponiblesPargua, esperandoChacao, esperandoPargua, haciaPargua, haciaChacao, "chacao");
}


void finalizar(){
	nFree(disponiblesPargua);
	nFree(disponiblesChacao);

	nDestroyCondition(pargua);
	nDestroyCondition(chacao);
}