#include <nSystem.h>
#include "transbordo.h"

int transbordadores;


void inicializar(int p){
	transbordadores = p;
}


void transbordoAChacao(int v){
	nEnter(m);
	if (!disponiblePargua()){
		if (disponibleChacao()){
			int disponible = getDisponibleChacao();
			int autoOpuesto = getFifo(Chacao);
			if (autoOpuesto == null){
				autoOpuesto = -1;
			}
			pushFifo(v);
			nExit(m);
			haciaPargua(disponible, autoOpuesto);
			nEnter(m);
			int currAuto = getFifo(Pargua);

			if (currAuto == NULL)
			{
				currAuto = v;
			}

			nExit(m);
			haciaChacao(disponible, currAuto);
		}
		else
		{
			pushFifo(v);
		}
	}
	else if (disponiblePargua()){
		int disponible = getDisponiblePargua();
		int currAuto = getFifo(Pargua);

		if (currAuto == NULL)
		{
			currAuto = v;
		}

		nExit(m);
		haciaChacao(disponible, currAuto);
		nEnter(m);
		setDisponible(disponible);
		nNotifyAll(m);
		nExit(m);
		if (currAuto == v){
			return v;
		}
		else{
			nWait(m);
		}
	}
	else{
		nWait(m);
	}
}