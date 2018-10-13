#include <nSystem.h>
#include "transbordo.h"

int debugging= TRUE;
int verificar, achacao, apargua;

typedef struct {
  int i; /* transbordador */
  int v; /* vehiculo */
  nTask t;
  int haciaChacao;
} Viaje;

/* Guarda el identificador de la tarea nMain que sirve para controlar
   el avance del resto de las tareas */
nTask ctrl;

/* Procedimientos para los tests */

Viaje *esperarTransbordo();
void continuarTransbordo(Viaje *);

int norteno(int v), nortenoConMsg(int v);
int isleno(int v), islenoConMsg(int v);

int nMain( int argc, char **argv ) {
  ctrl= nCurrentTask();
  inicializar(3);
  verificar= TRUE;

  nPrintf("\n\nTest X: se transborda 1 vehiculo a chacao y 1 vehiculo a pargua en paralelo. Los transbordadores osciosos se deben quedar quietos mientras no sean llamados.\n");

  nTask t0= nEmitTask(norteno, 0);
  nTask t1= nEmitTask(isleno, 1);
  Viaje *viajea= esperarTransbordo();
  Viaje *viajeb= esperarTransbordo();

  continuarTransbordo(viajeb);
  
  viajeb= esperarTransbordo();
  continuarTransbordo(viajea);
  nTask t2= nEmitTask(isleno, 2);
  Viaje *viajec= esperarTransbordo();
  nTask t3= nEmitTask(isleno, 3);
  Viaje *viajed= esperarTransbordo();

  if (viajed->i != 2){
    nFatalError("nMain", "Este transbordador tenia que venir a la isla.\n");
  }

  if (viajec->i == 2){
    nFatalError("nMain", "Este transbordador tenia que quedarse quieto.\n");
  }

  continuarTransbordo(viajeb);
  continuarTransbordo(viajec);
  continuarTransbordo(viajed);
  viajed= esperarTransbordo();
  continuarTransbordo(viajed);

  nWaitTask(t0); 
  nWaitTask(t1); 
  nWaitTask(t2); 
  nWaitTask(t3); 
  finalizar();

  nPrintf("\n\nCustom Test aprobado\n");
  return 0;
}

int norteno(int v) {
  transbordoAChacao(v);
  return 0;
}

int isleno(int v) {
  transbordoAPargua(v);
  return 0;
}

void haciaChacao(int i, int v) {
  if (!verificar)
    achacao++;
  else {
    Viaje viaje;
    viaje.i= i;
    viaje.v= v;
    viaje.haciaChacao= TRUE;
    nSend(ctrl, &viaje);
  }
}

void haciaPargua(int i, int v) {
  if (!verificar)
    apargua++;
  else {
    Viaje viaje;
    viaje.i= i;
    viaje.v= v;
    viaje.haciaChacao= FALSE;
    nSend(ctrl, &viaje);
  }
}

Viaje *esperarTransbordo() {
  nTask t;
  Viaje *viaje= nReceive(&t, -1);
  viaje->t= t;
  return viaje;
}

void continuarTransbordo(Viaje *viaje) {
  nReply(viaje->t, 0);
}