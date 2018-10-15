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
  // testUnTransbordo: se invoca cuando debe haber un transbordador
  //   disponible en la misma orilla del vehiculo
  // testUnTransbordoVacio: se invoca cuando no debe haber ningun
  //   transbordador disponible en la misma orilla del vehiculo
  for (k=0; k<10; k++)
  { // Se transbordan 3 vehiculos a Chacao secuencialmente
    // No pueden haber transbordos sin vehiculo
    int i0= testUnTransbordo(norteno, 0); // entrega el transbordador usado
    int i1= testUnTransbordo(norteno, 1);
    int i2= testUnTransbordo(norteno, 2);
    if (i0==i1 || i1==i2 || i0==i2)
      nFatalError("nMain", "Los transbordadores debieron ser distintos\n");

    // Ahora todos los transbordadores estan en Chacao
    // Este transbordo hacia Chacao requiere traer transbordador vacio a Pargua
    testUnTransbordoVacio(nortenoConMsg, 3, TRUE);
    // Estos dos transbordos no requieren viajes sin vehiculo
    testUnTransbordo(isleno, 4);
    testUnTransbordo(norteno, 5);
    // Todos los transbordadores estan en Chacao

    i0= testUnTransbordo(isleno, 0); // entrega el transbordador usado
    i1= testUnTransbordo(isleno, 1);
    i2= testUnTransbordo(isleno, 2);
    if (i0==i1 || i1==i2 || i0==i2)
      nFatalError("nMain", "Los transbordadores debieron ser distintos\n");

    // Todos los transbordadores estan en Pargua
    // Este transbordo hacia Pargua requiere traer transbordador vacio a Chacao
    testUnTransbordoVacio(islenoConMsg, 3, FALSE);
    // Estos dos transbordos no requieren viajes sin vehiculo
    testUnTransbordo(norteno, 4);
    testUnTransbordo(isleno, 5);
  }

  // Todos los transbordadores estan en Pargua

  { // Se transbordan 4 vehiculos a Chacao en paralelo
    nPrintf("Test 2: se transbordan 4 vehiculos a Chacao en paralelo\n");
    nTask t0= nEmitTask(norteno, 0);
    nTask t1= nEmitTask(norteno, 1);
    nTask t2= nEmitTask(norteno, 2);
    nTask t3, t4, t5, t6, t7;
    // Lea atentamente los comentarios de esperarTransbordo y
    // continuarTransbordo al final de este archivo.
    // La funcion esperarTransbordo espera a que su implementacion de
    // transbordoAChacao gatille la llamada a haciaChacao.
    Viaje *viajea= esperarTransbordo();
    // Ok, la invocacion de haciaChacao ocurrio.  Pero ahora haciaChacao
    // espera que nMain invoque continuarTransbordo(viajea) para poder
    // retornar
    Viaje *viajeb= esperarTransbordo(); // Espera llamada a haciaChacao
    Viaje *viajec= esperarTransbordo(); // Espera llamada a haciaChacao
    // Todos los transbordadores van haciaChacao.  Cada viaje llegara
    // a Chacao cuando nMain invoque continuarTransbordo(viaje).
    if (viajea->i==viajeb->i || viajeb->i==viajec->i || viajea->i==viajec->i)
      nFatalError("nMain", "Los transbordadores debieron ser distintos\n");
    t3= nEmitTask(isleno, 3); // No hay transbordadores, debe esperar
    if ((Viaje*)nReceive(NULL, 1)!=NULL)
      nFatalError("nMain", "De donde salio un transbordador adicional?\n");
    // Se invoca continuarTransbordo(viajeb) lo que significa que ese
    // transbordador llego a Chacao.  Esto se traduce en que la funcion
    // haciaChacao que lleva el vehiculo viajeb->v retorne.
    // Esto permite que su implementacion de transbordoAChacao tambien retorne.
    continuarTransbordo(viajeb);
    // Se acaba de liberar un transbordador.  Servira para el vehiculo 3.
    viajeb= esperarTransbordo(); // Espera nueva llamada a haciaChacao
    if (viajeb->v!=3)
      nFatalError("nMain", "Aca debio haber viajado el vehiculo 3\n");
    // 4 nuevos vehiculos esperan transbordo
    t4= nEmitTask(isleno, 4);
    nSleep(100);     // Esto es para asegurarme de que se inicien en este orden
    t5= nEmitTask(norteno, 5);
    nSleep(100);
    t6= nEmitTask(norteno, 6);
    nSleep(100);
    t7= nEmitTask(isleno, 7);
    // Hay 3 transbordos en progreso (probablemente vehiculos 0, 2 y 3)
    // y 4 en cola
    continuarTransbordo(viajea); // Termina transbordo, libera transbordador.
    viajea= esperarTransbordo(); // Puede ser un transbordo vacio
    if (viajea->v==-1) {
      continuarTransbordo(viajea);
      viajea= esperarTransbordo(); // No puede ser un transbordo vacio.
    }
    continuarTransbordo(viajec); // Termina transbordo, libera transbordador
    viajec= esperarTransbordo(); // Puede ser un transbordo vacio
    if (viajec->v==-1) {
      continuarTransbordo(viajec);
      viajec= esperarTransbordo();  // No puede ser un transbordo vacio
    }
    continuarTransbordo(viajeb); // Termina transbordo, libera transbordador
    viajeb= esperarTransbordo(); // Puede ser un transbordo vacio
    if (viajeb->v==-1) {
      continuarTransbordo(viajeb);
      viajeb= esperarTransbordo(); // No puede ser un transbordo vacio
    }
    continuarTransbordo(viajec); // Termina transbordo, libera transbordador
    viajec= esperarTransbordo(); // Puede ser un transbordo vacio
    if (viajec->v==-1) {
      continuarTransbordo(viajec);
      viajec= esperarTransbordo(); // No puede ser un transbordo vacio
    }
    continuarTransbordo(viajea); // Termina transbordo
    continuarTransbordo(viajeb); // Termina transbordo
    continuarTransbordo(viajec); // Termina transbordo

    nWaitTask(t0); nWaitTask(t1); nWaitTask(t2); nWaitTask(t3);
    nWaitTask(t4); nWaitTask(t5); nWaitTask(t6); nWaitTask(t7);
  }

  // Algunos transbordadores en Pargua y otros en Chacao

  {
#define T 600
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
  // Precondicion: hay transbordadores disponibles en la misma orilla
  // del vehiculo v.
  // Embarca, transborda y desembarca vehiculo v.  Retorna el transbordador
  // usado que queda estacionado en la otra orilla.
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
  // Precondicion: *no* hay transbordadores disponibles en la misma orilla
  // del vehiculo v.
  // Embarca, transborda y desembarca vehiculo v.  Retorna el transbordador
  // usado que queda estacionado en la otra orilla.
  // Verifica que un transbordador hizo un viaje vacio.
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
  // Si transbordoAPargua retorna antes de invocar haciaPargua, este
  // mensaje va hacer fallar los tests
  return nSend(ctrl, &falso);
}

// haciaChacao: Ud. invoca esta funcion en transbordo.c cuando se invoca
// transbordoAChacao.
void haciaChacao(int i, int v) {
  if (!verificar)
    achacao++;
  else {
    Viaje viaje;
    viaje.i= i;
    viaje.v= v;
    viaje.haciaChacao= TRUE;
    // Notifica al nMain que se invoco haciaChacao.  El vehiculo v va
    // hacia Chacao.  Llegara cuando nMain invoque continuarTransbordo(&viaje).
    nSend(ctrl, &viaje);
    // Ahora si se desembarco v.  La funcion retorna con lo que
    // transbordoAChacao puede retornar.
  }
}


// haciaPargua: Ud. invoca esta funcion en transbordo.c cuando se invoca
// transbordoAPargua.
void haciaPargua(int i, int v) {
  if (!verificar)
    apargua++;  // Solo valido para el test de esfuerzo
  else {
    Viaje viaje;
    viaje.i= i;
    viaje.v= v;
    viaje.haciaChacao= FALSE;
    // Notifica al nMain que se invoco haciaPargua.  El vehiculo v va
    // hacia Pargua.  Llegara cuando nMain invoque continuarTransbordo(&viaje).
    nSend(ctrl, &viaje);
    // Termino el transbordo.  La funcion retorna con lo que
    // transbordoAPargua puede retornar.
  }
}

Viaje *esperarTransbordo() {
  nTask t;
  // Espera la notificacion del inicio de cualquier transbordo llamando
  // haciaPargua o haciaChacao.
  // Retorna en viaje el transbordador usado y el vehiculo que lleva.
  Viaje *viaje= nReceive(&t, -1);
  viaje->t= t;
  return viaje;
}

void continuarTransbordo(Viaje *viaje) {
  // Invocada por nMain.
  // Hace que termine el transbordo de viaje->i con el vehiculo viaje->v.
  // Ese transbordador llego a la orilla y por lo tanto la
  // llamada a haciaPargua o haciaChacao retorna.  El transbordador queda
  // libre para otro viaje.
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