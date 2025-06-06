/* ---------------------------------------------------------------
 *  
 *  MODULO: Receptor.
 *
 *   Modulo receptor de peticiones de servicio remotas.
 * 
 * --------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <setjmp.h>
#include <textos.h>
#include <excepcion.h>
#include <src.h>
#include <direc.h>
#include <correos.h>
#include <recep.h>
#include <codifica.h>
#include <emisor.h>

static char * clase = NULL;
static char * metodo = NULL;

extern int espera_retorno;  /* Estamos a la espera de una respuesta */
extern int espera_retorno_timeout;  /* Estado de la temporizacion de la resp.*/ 
extern jmp_buf jumper;

int peticiones_pendientes = 0;  /*  Peticiones de servicio sin atender */

/* -----------------------------------------------------
 *  Variables para el mantenimiento de las peticiones pendientes.
 * ----------------------------------------------------------------- */
static s_peticiones  * peticion_pendiente_actual = NULL;
static s_peticiones  * cabecera_peticiones_pendientes = NULL;


/* ------------------------------------------------------
 *  Guardar una peticion. 
 * ------------------------------------------------------- */
static int Receptor_Pendiente(int que_buzon, 
                              struct sockaddr_in origen, char t_mensaje, 
                              char * mensaje, char * parametros, 
                              int t_parametros);

/* ------------------------------------------------------
 *  Crear la memoria para las variables de retorno de un
 * metodo. 
 * ------------------------------------------------------- */
static int Receptor_CreaMemoriaSalida(s_parametros *);

/* ----------------------------------------------------------------------
 *  Funcion: Despachar.
 * 
 *    Despacha una peticion de servicio, es decir llama a las rutinas
 * que lo enlazan con el metodo que lo sirve si es que existe este.
 * 
 *    Esta operacion no se realizara si estamos a la espera de una 
 *  respuesta.
 *  
 *    Parametros
 *        Entrada:
 *                 origen,   origen del mensaje.
 *                 peticion, peticion recibida.
 *                 tamanno,  tamano de la peticion. 
 *
 *        Retorna:
 *          0, ok despachado.
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
int Receptor_Despachar(int que_buzon, struct sockaddr_in origen, 
                       char * peticion, int tamano)
{
  int version;
  int t_t_peticion;
  char t_mensaje;

   /* -----------------------------------------------
    *   Si hay servicio y hay TIMEOUT.
    * ------------------------------------------------- */
   if (espera_retorno && espera_retorno_timeout)
   {
      espera_retorno = 0;
      longjmp(jumper,1);
    }

   SetRequestAddress(&origen);

   Decodifica_TipoMensaje(&t_mensaje,&peticion,&t_t_peticion);
    
   switch(t_mensaje)
   {
     case SERV_METODO:
     {
        /* ---------------------------------------------
         *  Si no estamos esperando una respuesta
         * dar curso a la peticion actual.
         * ----------------------------------------------- */
        if (!espera_retorno)
        {
          if (clase == NULL) clase = (char *)malloc(MAX_CLASE);
          if (metodo == NULL) metodo = (char *)malloc(MAX_METODO);
          Decodifica_CabeceraMetodo(&clase,&metodo,
                      &version,(peticion+t_t_peticion),(tamano-t_t_peticion));
   
          LlamaAFuncion(clase,metodo,version,(peticion+t_t_peticion),
						   (tamano-t_t_peticion));

          free(peticion);
	  peticion = NULL;
          return 0;
       }
       else
       {
         Receptor_Pendiente(que_buzon, origen,SERV_METODO,peticion,
			    (peticion+t_t_peticion),(tamano-t_t_peticion));
         return 0;
       }
     }
     case SERV_RESPUESTA:
     {
        if (espera_retorno)
        {
          Receptor_Pendiente(que_buzon, origen,SERV_RESPUESTA,peticion,
			     (peticion+t_t_peticion), (tamano-t_t_peticion));
          espera_retorno = 0;
          longjmp(jumper,1);
        }
       /* ----------------------------------------------
        *   Ignorar, fuera de secuencia.
        * ------------------------------------------------ */
        break;
     }
     case SERV_ERROR:
     {
        if (espera_retorno)
        {
          Receptor_Pendiente(que_buzon, origen,SERV_ERROR,peticion,
			     (peticion+t_t_peticion),(tamano-t_t_peticion));
          espera_retorno = 0;
          longjmp(jumper,1);
        }
       /* ----------------------------------------------
        *   Ignorar, error fuera de secuencia.
        * ------------------------------------------------ */
     }
  }
  return 0;
}

/* ----------------------------------------------------------------------
 *  Funcion: BuscaMetodo.
 * 
 *    Busca en la tabla de metodos de este servidor aquel que satisfaga 
 * la peticion realizada. 
 * 
 *    Parametros
 *        Entrada:
 *                 clase, clase a la que pertenece el metodo a servir.
 *                 metodo, metodo a disparar.
 *                 METODOS, tabla de metodos.
 *
 *        Retorna:
 *          0,>0, lugar en la tabla. 
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
int Receptor_BuscaMetodo(char * Clase, char * Metodo, s_metodos METODOS[])
{
  int i = 0;

   char f_metodo[MAX_CLASE+MAX_METODO + 2];
   strcpy(f_metodo,Clase);
   strcat(f_metodo,"_");
   strcat(f_metodo,Metodo);

   while (strcmp(METODOS[i].metodo,"")!= 0)
   {
     if (strcmp(METODOS[i].metodo,f_metodo) == 0) 
     {
        if (METODOS[i].salida != NULL)
        {
           if (strcmp(METODOS[i].salida[0].tipo,"") != 0)
              Receptor_CreaMemoriaSalida(METODOS[i].salida);
        }
       return i;
     }
     i ++;
   }
   return -1;
}


/* ----------------------------------------------------------------------
 *  Funcion: Pendiente
 * 
 *    Incluye un mensaje en la lista de peticiones pendientes de 
 * atender. 
 * 
 *    Parametros
 *        Entrada:
 *                 origen,   origen de la peticion.
 *                 t_mensaje, tipo de mensaje.
 *                 mensaje,   mensaje completo.
 *                 origen,    origen del mismo.
 *                 parametros, parametros.
 *                 t_parametros, tamano de los parametros.
 *
 *        Retorna:
 *          0, ok.
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
static int Receptor_Pendiente(int buzon, struct sockaddr_in origen, 
                              char t_mensaje, 
                              char * mensaje,char * parametros, 
                              int t_parametros)
{
  s_peticiones * pet_aux;

  pet_aux = (s_peticiones *)malloc(sizeof(s_peticiones));
  if (pet_aux == NULL)
  {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
  }

  memcpy(&pet_aux->origen,&origen,sizeof(struct sockaddr_in));
  pet_aux->tipo = t_mensaje;
  pet_aux->buzon = buzon;
  pet_aux->mensaje = parametros;
  pet_aux->recibido = mensaje;
  pet_aux->tamano = t_parametros;
  pet_aux->siguiente = NULL;
  peticion_pendiente_actual = pet_aux;

  /* ---------------------------------------------
   * La enlazamos en la lista de pendientes.
   * --------------------------------------------- */
  if (cabecera_peticiones_pendientes == NULL )
    cabecera_peticiones_pendientes = peticion_pendiente_actual;
  else
  {
    /* ----------------------------------------------------
     *  Para que luego se atiendan en orden de llegada.
     * ----------------------------------------------------- */
    pet_aux = cabecera_peticiones_pendientes;
    while ( pet_aux->siguiente != NULL)
      pet_aux = pet_aux->siguiente;
   
    pet_aux->siguiente = peticion_pendiente_actual;
  }

  peticiones_pendientes ++;
  return 0;
}

/* ----------------------------------------------------------------------
 *  Funcion: Actual
 * 
 *    Indica la peticion pendiente actual. 
 * 
 *    Parametros
 *        Salida:
 *                 origen,   origen de la peticion.
 *                 t_mensaje, tipo de mensaje.
 *                 origen,    origen del mismo.
 *                 clase,     clase a la que pertenece el metodo a servir.
 *                 metodo,    metodo a disparar.
 *                 version,   version del mismo.
 *                 parametros, parametros.
 *                 t_parametros, tamano de los parametros.
 *
 *        Retorna:
 *          0, ok.
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
int Receptor_Actual(s_peticiones ** actual)
{
    *actual = peticion_pendiente_actual;
    return 0;
}

/* ----------------------------------------------------------------------
 *  Funcion: AtiendePendientes
 * 
 *    Atiende si es posible la primera peticion pendiente que  
 *  encontremos.
 *
 *    Parametros
 *        Retorna:
 *          0, ok.
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
int Receptor_AtiendePendientes()
{
  s_peticiones * aux;
  int version;

   aux = cabecera_peticiones_pendientes;

   if (aux == NULL  || espera_retorno)
   {
     peticiones_pendientes = 0;
     return 0;
   }

   Correos_BuzonEstablecer(aux->buzon);
   
   switch(aux->tipo)
   {
     case SERV_METODO:
     {
        if (clase == NULL) clase = (char *)malloc(MAX_CLASE);
        if (metodo == NULL) metodo = (char *)malloc(MAX_METODO);
        Decodifica_CabeceraMetodo(&clase,&metodo, &version,
                                      aux->mensaje,aux->tamano);
   
        LlamaAFuncion(clase,metodo,version,aux->mensaje,aux->tamano);

        free(aux->recibido);
	aux->recibido = NULL;
        break;
     }
     case SERV_RESPUESTA:
        return 0;
   }
     
   aux = cabecera_peticiones_pendientes;
   cabecera_peticiones_pendientes = aux->siguiente;
   free(aux);
   aux = NULL;
   peticiones_pendientes --;

   Correos_BuzonEstablecer(-1);
   return 0;
}

/* ----------------------------------------------------------------------
 *  Funcion: LiberaActual 
 * 
 *    Peticion actual atendida en otro punto, por ser una respuesta 
 *  u otro tipo de mensaje. liberar la memoria. 
 *
 *    Parametros
 *        Retorna:
 *          0, ok.
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
int Receptor_LiberaActual()
{
  s_peticiones * aux;
 
  aux = cabecera_peticiones_pendientes;
  if (cabecera_peticiones_pendientes == peticion_pendiente_actual)
  {
    cabecera_peticiones_pendientes = cabecera_peticiones_pendientes->siguiente;
    free(aux);
    return 0;
  }

  while (aux->siguiente != peticion_pendiente_actual)
   aux = aux->siguiente;

  if (aux->siguiente == peticion_pendiente_actual)
  {
   aux->siguiente = NULL;
   free(peticion_pendiente_actual);
   peticion_pendiente_actual = NULL;
  }
  peticiones_pendientes --;
  return 0;
}

/* ----------------------------------------------------------------------
 *  Funcion: Receptor_CreaMemoriaSalida 
 * 
 *    Crea la memoria para las variables de retorno de un metodo. 
 *
 *    Parametros
 *        Retorna:
 *          0, ok.
 *          0<, algun tipo de error.
 * ----------------------------------------------------------------------- */
static int Receptor_CreaMemoriaSalida(s_parametros * parametros)
{
   while (strcmp(parametros->tipo,"") != 0)
   {
     if (parametros->valor == NULL)
     {
       switch(*parametros->tipo)
       {
         case 'c': parametros->valor = (void *)malloc(sizeof(char));
                   break;
         case 'i': parametros->valor = (void *)malloc(sizeof(int));
                   break;
         case 'l': parametros->valor = (void *)malloc(sizeof(long));
                   break;
         case 'f': parametros->valor = (void *)malloc(sizeof(float));
                   break;
         case 'd': parametros->valor = (void *)malloc(sizeof(double));
                   break;
         case 's': parametros->valor = (void *)malloc(sizeof(char *));
                   break;
         case 'C': 
           parametros->valor = (void *)malloc(sizeof(char)*parametros->lon);
                   break;
         case 'I': 
           parametros->valor = (void *)malloc(sizeof(int)*parametros->lon);
                   break;
         case 'L': 
           parametros->valor = (void *)malloc(sizeof(long)*parametros->lon);
                   break;
         case 'F': 
           parametros->valor = (void *)malloc(sizeof(float)*parametros->lon);
                   break;
         case 'D': 
           parametros->valor = (void *)malloc(sizeof(double)*parametros->lon);
                   break;
       }
       if (parametros->valor == NULL)
        return -1;
     }
   
     parametros++;
   }
   return 0;
}

