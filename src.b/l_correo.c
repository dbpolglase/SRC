/* ---------------------------------------------------------------
 *  
 *  MODULO: Lista Correos.
 *
 *   Mantiene la lista de correos, esta lista es la de los pedacitos 
 *  que estamos recibiendo para su recomposicion.
 * --------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <textos.h>
#include <excepcion.h>
#include <src.h>
#include <recep.h>
#include <codifica.h>
#include <direc.h>
#include <correos.h>
#include <l_correo.h>

/* ----------------------------------------------------------------
 *  Estructuras necesarias para recomponer los pedazos de mensajes
 * recibidos.
 * ----------------------------------------------------------------- */ 

struct s_TROZO{
		  char * trozo;
		  int tamano;
		  struct s_TROZO * siguiente;
	       };

typedef struct s_TROZO s_trozo;

struct s_MENSAJE{
                   time_t tiempo;
                   int buzon;
		   struct sockaddr_in origen;
		   long identificativo;
		   long nro_pedazo;
		   long tamano;
		   s_trozo * p_trozo;
		   s_trozo * u_trozo;
		   struct s_MENSAJE * siguiente;
		   };

typedef struct s_MENSAJE s_mensaje;
		   
/* --------------------------------------------------------------
 *  Cada cuanto pasa el camion de la basura. 
 * -------------------------------------------------------------- */
#define RECOGER_BASURA   100
/* --------------------------------------------------------------
 *  Tiempo que debe trascurrir sin recibir ningun trozo de un mensaje
 * para que consideremos que es historia. 
 * -------------------------------------------------------------- */
#define TIEMPO_DESCOMPOSICION   100

/* ----------------------------------------------------------------
 *  Cabecera a la lista de mensajes en recomposicion y al  
 * mensaje que estamos recomponiendo ahora. 
 * ----------------------------------------------------------------- */ 
static s_mensaje * ListaMensajes = NULL; 
static s_mensaje * mensaje_actual = NULL; 

/* ----------------------------------------------------------------
 *  Funciones privadas al modulo necesarias. 
 * ----------------------------------------------------------------- */ 

static s_mensaje * LCorreos_Buscar(struct sockaddr_in origen,
                                               long identificativo);
static void LCorreos_Libera(void);
static void LCorreos_Enlaza(void);
static void LCorreos_RecogerBasura(void);

/* ---------------------------------------------------------------------
 *  Funcion: NuevoTrozo.
 * 
 *   Funcion que se encarga de annadir un nuevo trozo a la estrutura
 * que mantiene la lista de trozos.
 * 
 *   Parametros
 *      Entrada:  
 *               buzon,  buzon del que se ha leido. 
 *               origen, origen del mensaje.
 *               trozo, mensaje fragmentado recibido.
 *               longitud, tamano de este mensaje. 
 *      Retorna:
 *        0<, error.
 *        0, completado.
 * ---------------------------------------------------------------------- */

int LCorreos_NuevoTrozo(int buzon, struct sockaddr_in origen, 
                                        char * trozo,int t_trozo)
{
  s_trozo * trozo_actual;
  long identificativo, pedazo;
  static int recolector = 0;


  /* --------------------------------------------------
   *  Obtener el identificativo y nro. de trozo. 
   * -------------------------------------------------- */
   if (Decodifica_Mensaje(trozo,&identificativo,&pedazo) < 0)
      return -1;

  /* --------------------------------------------------
   *  Crear la estructura mensaje si es necesaria. 
   * -------------------------------------------------- */
   if ((mensaje_actual = LCorreos_Buscar(origen,identificativo)) == NULL)
   {
     /* --------------------------------------------------
      *  Primer trozo crear estructura mensaje. 
      * -------------------------------------------------- */
       if (pedazo == 1)
       {
	 mensaje_actual = (s_mensaje *) malloc(sizeof(s_mensaje));
	 if (mensaje_actual == NULL)
         {
           Excepcion_Registra(ERR_MALLOC);
           return -1;
         }
	 memcpy((void *)&mensaje_actual->origen,
                            (void *)&origen,sizeof(struct sockaddr_in));
	 mensaje_actual->buzon = buzon;
	 mensaje_actual->nro_pedazo = 0;
	 mensaje_actual->identificativo = identificativo;
	 mensaje_actual->tamano = 0;
	 mensaje_actual->p_trozo = NULL;
	 mensaje_actual->u_trozo = NULL;
	 mensaje_actual->siguiente = NULL;
	 LCorreos_Enlaza();
       } 
       else 
       {
         Excepcion_Registra(ERR_PEDAZO_FUERA_SECUENCIA);
         return -1;
       }
   }

  /* --------------------------------------------------
   *  Actualizamos el tiempo que nos sirve de refencia 
   * para eliminar los mensajes incompletos.
   * -------------------------------------------------- */
   mensaje_actual->tiempo = time(0);

  /* --------------------------------------------------
   *  Crear la estructura trozo.
   * -------------------------------------------------- */

   if (mensaje_actual->nro_pedazo + 1 !=  pedazo)
   {
     Excepcion_Registra(ERR_PEDAZO_FUERA_SECUENCIA);
     return -1;
   }

   trozo_actual = (s_trozo *)malloc(sizeof(s_trozo));
   if (trozo_actual == NULL)
   {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
   }
   trozo_actual->trozo = (char *)malloc(t_trozo - CAB_IDENTIFICATIVO);
   if (trozo_actual->trozo == NULL)
   {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
   }
   memcpy(trozo_actual->trozo,trozo + CAB_IDENTIFICATIVO,
                                    t_trozo - CAB_IDENTIFICATIVO);
   trozo_actual->tamano = t_trozo - CAB_IDENTIFICATIVO; 
   trozo_actual->siguiente = NULL;

  /* --------------------------------------------------
   *  Enlazarla adecuadamente al mensaje. 
   * -------------------------------------------------- */
   mensaje_actual->tamano += t_trozo - CAB_IDENTIFICATIVO;
   mensaje_actual->nro_pedazo ++;
   if (mensaje_actual->p_trozo == NULL)
     mensaje_actual->p_trozo = mensaje_actual->u_trozo = trozo_actual;
   else
   {
     mensaje_actual->u_trozo->siguiente = trozo_actual;
     mensaje_actual->u_trozo = trozo_actual;
   }
   /* ------------------------------------------
    *  Cada 100 mensajes darse una vuelta para recoger la 
    * basura que haya podido quedar.
    * -------------------------------------------------------- */
   recolector ++;
   if (recolector == RECOGER_BASURA)
   {
      recolector = 0;
      LCorreos_RecogerBasura();
   }
   
   return 0;
}


/* --------------------------------------------------------------------
 *  Funcion: Buscar.
 *
 *  Busca en la lista de mensajes que estamos recibiendo si tenemos
 * alguna referencia al dado.
 * 
 *    Parametros
 *      Entrada:
 *            origen, origen del mensaje cuya referencia se busca.
 *            identificativo, nro. de mensaje de este origen. 
 *      Retorna:
 *          NULL, sin referencia.
 *          mensaje, estructura que representa al mensaje.
 * ------------------------------------------------------------------- */
static s_mensaje * LCorreos_Buscar(struct sockaddr_in origen,
                                   long identificativo)
{
  s_mensaje * aux;

  aux = ListaMensajes;

  /* -----------------------------------------------
   *   Mientras tengamos mensajes a mirar en la lista de mensajes
   * pendientes.
   * ------------------------------------------------------------------ */
  while (aux != NULL)
  {
     if (aux->identificativo == identificativo)
      if (memcmp((const void *)&aux->origen,(const void *)&origen,
                               sizeof(struct sockaddr_in)) == 0)
	return aux;

     aux = aux->siguiente;
  }
  return NULL;
}

/* ------------------------------------------------------------------- 
 *  Funcion: Enlaza
 *
 *  Enlaza la estructura del mensaje actual en la lista de mensajes 
 * en recepcion. 
 *
 * --------------------------------------------------------------------- */
static void LCorreos_Enlaza()
{
   mensaje_actual->siguiente = ListaMensajes;
   ListaMensajes = mensaje_actual;
}
 

/* ------------------------------------------------------------------- 
 *  Funcion: Obtener. 
 *
 *  Retorna el mensaje actual, es decir la estructura de mensaje 
 * correspondiente al ultimo trozo de mensaje recibido.
 *
 *  Parametros:
 *      Salida:
 *           buzon,   buzon origen del mensaje. 
 *           Origen,  origen del mensaje.
 *           mensaje, mensaje completado.
 *           logitud, tamano del mismo.
 *
 *      Retorno:
 *            0, ok.
 *            0<, el mensaje no esta completo.
 * --------------------------------------------------------------------- */
int LCorreos_Obtener(int * buzon, struct sockaddr_in * origen, 
                     char ** mensaje,int * longitud)
{
   s_trozo * aux_trozo;
   long pegados = 0;

   /* -------------------------------------------------------
    *   Origen del mensaje. 
    * -------------------------------------------------------- */
    memcpy((void *)origen,(void *)&mensaje_actual->origen, 
                                 sizeof(struct sockaddr_in));

   /* -------------------------------------------------------
    *   Buzon origen. 
    * -------------------------------------------------------- */

    *buzon = mensaje_actual->buzon;
  
   /* -------------------------------------------------------
    *  Tamano del mensaje.
    * -------------------------------------------------------- */
   *longitud = mensaje_actual->tamano;
   *mensaje = (char *)malloc(*longitud);

   if (*mensaje == NULL)
   {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
   }

   /* -------------------------------------------------------
    *  Mete los trozos en el buffer de mensaje. 
    * -------------------------------------------------------- */
   aux_trozo = mensaje_actual->p_trozo;

   while (aux_trozo != NULL)
   {
      memcpy((*mensaje)+pegados,aux_trozo->trozo,aux_trozo->tamano);
      pegados +=aux_trozo->tamano;
      aux_trozo = aux_trozo->siguiente;
   } 

   LCorreos_Libera();   /* Elimina los trozos, consumido */
   return 0;
}

/* --------------------------------------------------------------------- 
 *  Funcion: Libera
 *
 *  Libera la estructura correspondiente al mensaje actual, consumido
 * o con fecha de caducidad vencida.
 * 
 * --------------------------------------------------------------------- */
static void LCorreos_Libera()
{
  s_mensaje * aux_mensaje, * aux2_mensaje;
  s_trozo * aux_trozo, * aux2_trozo;

  /* -----------------------------------------------------------------
   *  Eliminar los pedacitos.
   * ----------------------------------------------------------------- */
  aux2_trozo = aux_trozo = mensaje_actual->p_trozo;

  while (aux_trozo != NULL)
  {
    aux_trozo = aux_trozo->siguiente;
    free(aux2_trozo->trozo);
    free(aux2_trozo);
    aux2_trozo = aux_trozo;
   }
    
  /* -----------------------------------------------------------------
   *  Eliminar esta estructura de la lista de mensajes pendientes. 
   * ----------------------------------------------------------------- */
  aux_mensaje = ListaMensajes;

  if (aux_mensaje == NULL) return;
  if (aux_mensaje == mensaje_actual)
  {
     ListaMensajes = ListaMensajes->siguiente;
     free(aux_mensaje);
     return;
  }

  while(aux_mensaje->siguiente != NULL && aux_mensaje->siguiente != mensaje_actual)
    aux_mensaje = aux_mensaje->siguiente;
 
  aux2_mensaje = aux_mensaje->siguiente;
  aux_mensaje->siguiente = aux_mensaje->siguiente->siguiente;
  free(aux2_mensaje);

}

/* --------------------------------------------------------------------- 
 *  Funcion: RecogerBasura
 *
 *   Borrar todos aquellos mensajes que lleven un tiempo sin estar 
 * completados. 
 * 
 * --------------------------------------------------------------------- */
static void LCorreos_RecogerBasura()
{
  s_mensaje * aux, * aux2;
  s_trozo * aux_trozo, * aux2_trozo;
  time_t    t_actual;

  aux2 = aux = ListaMensajes;
  t_actual = time(0);

   while (aux != NULL)
   {
      if ((t_actual - aux->tiempo) > TIEMPO_DESCOMPOSICION)
      {

         /* -----------------------------------------------------------------
          *  Eliminar los pedacitos.
          * ----------------------------------------------------------------- */
         aux2_trozo = aux_trozo = mensaje_actual->p_trozo;

         while (aux_trozo != NULL)
         {
           aux_trozo = aux_trozo->siguiente;
           free(aux2_trozo->trozo);
           free(aux2_trozo);
           aux2_trozo = aux_trozo;
          }
         /* -----------------------------------------------------------------
          *  Desenlazar y liberar la estructura. 
          * ----------------------------------------------------------------- */
          if (aux == ListaMensajes)
            ListaMensajes = ListaMensajes->siguiente;
          else
            aux2->siguiente = aux->siguiente;

          free(aux);
       }
       aux2 = aux;
       aux = aux->siguiente;
     }
}

