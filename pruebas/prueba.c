#include "nSystem.h"


int escritor(int num, int espera);


int nMain(int argc, char **argv)
{
	nTask tareas[3];
	int i;
	for (i = 0; i < 3; ++i)
		tareas[i] = nEmitTask(escritor, i, i * 200);
	for (i = 0; i < 3; ++i)
		nWaitTask(tareas[i]);
	nPrintf("Fin ejemplo\n");
}


int escritor(int num, int espera) 
{
	int i = 5;
	while (i > 0) 
	{
		nPrintf("thread %d: %d\n", num, i);
		nSleep(espera);
		i--;
	}
}