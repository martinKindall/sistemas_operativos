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
  int k;
  ctrl= nCurrentTask();
  inicializar(3);
  verificar= TRUE;
  nPrintf("Test 1: se transbordan 3 vehiculos a chacao secuencialmente\n");
  for (k=0; k<10; k++)
  { /* Se transbordan 3 vehiculos a Chacao secuencialmente */
    int i0= testUnTransbordo(norteno, 0); /* entrega el transbordador usado */
    int i1= testUnTransbordo(norteno, 1);
    int i2= testUnTransbordo(norteno, 2);
    if (i0==i1 || i1==i2 || i0==i2)
      nFatalError("nMain", "Los transbordadores debieron ser distintos\n");

    /* Ahora todos los transbordadores estan en Chacao */
    testUnTransbordoVacio(nortenoConMsg, 3, TRUE);
    testUnTransbordo(isleno, 4);
    testUnTransbordo(norteno, 5);

    i0= testUnTransbordo(isleno, 0); /* entrega el transbordador usado */
    i1= testUnTransbordo(isleno, 1);
    i2= testUnTransbordo(isleno, 2);
    if (i0==i1 || i1==i2 || i0==i2)
      nFatalError("nMain", "Los transbordadores debieron ser distintos\n");

    /* Ahora todos los transbordadores estan en Pargua */
    testUnTransbordoVacio(islenoConMsg, 3, FALSE);
    testUnTransbordo(norteno, 4);
    testUnTransbordo(isleno, 5);
  }
  { /* Se transbordan 4 vehiculos a Chacao en paralelo */
    nPrintf("Test 2: se transbordan 4 vehiculos a Chacao en paralelo\n");
    nTask t0= nEmitTask(norteno, 0);
    nTask t1= nEmitTask(norteno, 1);
    nTask t2= nEmitTask(norteno, 2);
    nTask t3, t4, t5, t6, t7;
    Viaje *viajea= esperarTransbordo();
    Viaje *viajeb= esperarTransbordo();
    Viaje *viajec= esperarTransbordo();
    if (viajea->i==viajeb->i || viajeb->i==viajec->i || viajea->i==viajec->i)
      nFatalError("nMain", "Los transbordadores debieron ser distintos\n");
    t3= nEmitTask(isleno, 3); /* No hay transbordadores, debe esperar */
    if ((Viaje*)nReceive(NULL, 1)!=NULL)
      nFatalError("nMain", "De donde salio un transbordador adicional?\n");
    continuarTransbordo(viajeb); /* Se libera un transbordador */
    viajeb= esperarTransbordo();
    if (viajeb->v!=3)
      nFatalError("nMain", "Aca debio haber viajado el vehiculo 3\n");
    t4= nEmitTask(isleno, 4);
    nSleep(100);     /* Esto es para asegurarme de que lleguen en este orden */
    t5= nEmitTask(norteno, 5);
    nSleep(100);
    t6= nEmitTask(norteno, 6);
    nSleep(100);
    t7= nEmitTask(isleno, 7);
    /* Hay 3 transbordos en progreso y 4 en cola */
    continuarTransbordo(viajea);
    viajea= esperarTransbordo();
    if (viajea->v==-1)
      viajea= esperarTransbordo();
    continuarTransbordo(viajec);
    viajec= esperarTransbordo();
    if (viajec->v==-1)
      viajec= esperarTransbordo();
    continuarTransbordo(viajeb);
    viajeb= esperarTransbordo();
    if (viajeb->v==-1)
      viajeb= esperarTransbordo();
    continuarTransbordo(viajec);
    viajec= esperarTransbordo();
    if (viajec->v==-1)
      viajec= esperarTransbordo();
    continuarTransbordo(viajea);
    continuarTransbordo(viajeb);
    continuarTransbordo(viajec);

    nWaitTask(t0); nWaitTask(t1); nWaitTask(t2); nWaitTask(t3);
    nWaitTask(t4); nWaitTask(t5); nWaitTask(t6); nWaitTask(t7);
  }

  {
#define T 300
    nTask tasks[T];
    int t;
    nPrintf("Test de esfuerzo.  Se demora bastante!\n");
    nSetTimeSlice(1);
    verificar= FALSE;
    achacao= 0;
    apargua= 0;
    for (t=0; t<T; t++)
      tasks[t]= nEmitTask(automovilista, t, 300);
    for (t=0; t<T; t++) {
      nWaitTask(tasks[t]);
      nPrintf(".");
    }
    nPrintf("\ntotal transbordos a chacao= %d, a pargua=%d\n",
            achacao, apargua);
  }

  finalizar();

  nPrintf("\n\nBien! Su tarea funciono correctamente con estos tests.  Si\n");
  nPrintf("al finalizar este programa nSystem no indica ninguna operacion\n");
  nPrintf("pendiente, Ud. ha completado exitosamente todos los tests\n");
  nPrintf("de la tarea.  Ud. puede entregar su tarea.\n\n");
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
