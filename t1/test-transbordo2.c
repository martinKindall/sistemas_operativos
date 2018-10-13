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

int testUnTransbordo(int (*tipo)(), int v);
int testUnTransbordoVacio(int (*tipo)(), int v, int haciaChacao);

int norteno(int v), nortenoConMsg(int v);
int isleno(int v), islenoConMsg(int v);

int automovilista(int v, int n);

int nMain( int argc, char **argv ) {
  ctrl= nCurrentTask();
  inicializar(3);
  verificar= TRUE;

  nPrintf("\n\nTest X: se transborda 1 vehiculo a chacao y 1 vehiculo a pargua en paralelo. Los transbordadores osciosos se deben quedar quietos.\n");

  nTask t0= nEmitTask(norteno, 0);
  nTask t1= nEmitTask(isleno, 1);
  Viaje *viajea= esperarTransbordo();
  Viaje *viajeb= esperarTransbordo();
  nPrintf("\ndatos del viajeA. trans: %d, auto: %d, haciaChacao: %d\n", viajea->i, viajea->v, viajea->haciaChacao);
  nPrintf("\ndatos del viajeB. trans: %d, auto: %d, haciaChacao: %d\n", viajeb->i, viajeb->v, viajeb->haciaChacao);
  continuarTransbordo(viajeb);
  // viajea= esperarTransbordo();
  // nPrintf("\ndatos del viajeA. trans: %d, auto: %d, haciaChacao: %d\n", viajea->i, viajea->v, viajea->haciaChacao);
  
  // continuarTransbordo(viajeb);
  nPrintf("\ndatos del viajeB. trans: %d, auto: %d, haciaChacao: %d\n", viajeb->i, viajeb->v, viajeb->haciaChacao);
  // viajeb= esperarTransbordo();
  // if (viajeb->v==-1){
  //   nPrintf("entro if\n");
  //   // continuarTransbordo(viajeb);
  //   viajeb= esperarTransbordo();
  // }
  // else
  //   nFatalError("nMain", "Tenía que venir un transbordador vacío\n");
  // continuarTransbordo(viajeb);
  viajeb= esperarTransbordo();
  continuarTransbordo(viajea);
  continuarTransbordo(viajeb);
  // nTask t2= nEmitTask(isleno, 2);

  // if (viajea->i==viajeb->i)
  //     nFatalError("nMain", "Los transbordadores debieron ser distintos\n");
  


  nWaitTask(t0); 
  nWaitTask(t1); 
  finalizar();

  nPrintf("\n\nCustom Test aprobado\n");
  return 0;
}

int testUnTransbordo(int (*tipo)(), int v) {
  nTask vehiculoTask= nEmitTask(tipo, v); /* vehiculo v */
  Viaje *viaje= esperarTransbordo();
  int i= viaje->i; /* el transbordador usado */
  if (viaje->v!=v)
    nFatalError("testUnTransbordo", "Se transborda el vehiculo incorrecto\n");
  if ( !(0<=i && i<3) )
    nFatalError("testUnTransbordo", "El trabordador debe estar entre 0 y 2\n");
  continuarTransbordo(viaje);
  nWaitTask(vehiculoTask);
  return i;
}

int testUnTransbordoVacio(int (*tipo)(), int v, int haciaChacao) {
  nTask t;
  nTask vehiculoTask= nEmitTask(tipo, v); /* vehiculo v */
  Viaje *viaje= esperarTransbordo(); /* Este viaje no lleva auto */
  int i= viaje->i, old=i; /* el transbordador usado */
  if (viaje->v>=0)
    nFatalError("testUnTransbordoVacio",
                "No se debio transportar ningun vehiculo\n");
  if (viaje->haciaChacao==haciaChacao)
    nFatalError("testUnTransbordoVacio",
                "Este viaje es en la direccion incorrecta\n");
  continuarTransbordo(viaje);
  viaje= esperarTransbordo(); /* Este viaje si que lleva a v */
  if (i!=old)
    nFatalError("testUnTransbordo", "Se debio usar el mismo transbordador\n");
  if (viaje->v!=v)
    nFatalError("testUnTransbordo", "Se transborda el vehiculo incorrecto\n");
  if ( !(0<=i && i<3) )
    nFatalError("testUnTransbordo", "El trabordador debe estar entre 0 y 2\n");
  if (viaje->haciaChacao!=haciaChacao)
    nFatalError("testUnTransbordoVacio",
                "Este viaje es en la direccion incorrecta\n");
  if (nReceive(NULL, 1)!=NULL)
    nFatalError("testUnTransbordoVacio",
                "Este mensaje no debio haber llegado\n");
  continuarTransbordo(viaje);
  /* Ahora deberia llegar el mensaje falso */
  viaje= nReceive(&t, -1);
  if (viaje->v!= 1000)
    nFatalError("testUnTransbordoVacio",
                "Debio haber llegado un mensaje falso\n");
  nReply(t, 0);
  nWaitTask(vehiculoTask);
  return i;
}

int norteno(int v) {
  transbordoAChacao(v);
  return 0;
}

int nortenoConMsg(int v) {
  Viaje falso;
  falso.v= 1000;
  transbordoAChacao(v);
  /* Si transbordoAChacao retorna antes de invocar haciaChacao, este
     mensaje va hacer fallar los tests */
  return nSend(ctrl, &falso);
}

int isleno(int v) {
  transbordoAPargua(v);
  return 0;
}

int islenoConMsg(int v) {
  Viaje falso;
  falso.v= 1000;
  transbordoAPargua(v);
  /* Si transbordoAPargua retorna antes de invocar haciaPargua, este
     mensaje va hacer fallar los tests */
  return nSend(ctrl, &falso);
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

int automovilista(int v, int n) {
  int k;
  for (k=0; k<n; k++) {
    transbordoAChacao(v);
    transbordoAPargua(v);
  }
  return 0;
}
