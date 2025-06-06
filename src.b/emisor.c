/* -------------------------------------------------------------------
 *   MODULO: Emisor.
 *
 *    Modulo emisor de mensajes de peticion de servicio. Este modulo
 *  traduce las peticiones de servicios remotos (SRC), en mensajes 
 *  (cartas) para su envio por el sistema de correos.
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <textos.h>
#include <excepcion.h>
#include <src.h>
#include <principal.h>
#include <recep.h>
#include <codifica.h>
#include <correos.h>
#include <emisor.h>
#include <direc.h>
#include <tiempo.h>

/* -------------------------------------------------------------
 *  Punto de retorno de peticion de procedimiento.
 * ------------------------------------------------------------- */
jmp_buf jumper;

static char * aux_clase = NULL;
static char * aux_metodo = NULL;
/* -------------------------------------------------------------
 *  Indica que esperamos un retorno a un servicio. 
 * ------------------------------------------------------------- */
int espera_retorno = 0;
int espera_retorno_timeout = 0;

/* ---------------------------------------------------------
 *  
 *  Funcion: PeticionProcedimiento
 * 
 *  Enviar un mensaje de src modo sincrono (espera de respuesta), 
 * a un destino.
 *
 *  Parametros
 *     Entrada:
 *              Destino, servidor de esta peticion.
 *              Clase,   clase del servicio solicitado.
 *              Metodo,  metodo del servicio a activar.
 *              Version, version del servicio a activar.
 *              Tipo y valor de los Parametros de Entrada  al
 *                        metodo.
 *              Tipo y valor de los Parametros de Salida  al
 *                        metodo.
 * ------------------------------------------------------------------- */
int Emisor_PeticionProcedimiento(struct sockaddr_in Destino, char * Clase, 
                                 char * Metodo, int version,...)
{
   char * buffer;                /* Buffer para el mensaje a componer */
   va_list parametros;           /* recorrer la lista de parametros */
   int tamano = 0;               /* tammano del mensaje */
   s_peticiones * resp_peticion;
   int aux_version;
   char * v_parametro;
   va_list aux_parametros;
   char * error;

   if (aux_clase == NULL) aux_clase = (char *)malloc(MAX_CLASE);
   if (aux_metodo == NULL) aux_metodo = (char *)malloc(MAX_CLASE);

   /* -----------------------------------------------------------
    *  Recorre la lista de parametros y codificalos de forma 
    * independiente de la maquina en un buffer para su envio por
    * el sistema de comunicaciones.
    * ----------------------------------------------------------- */
   va_start(parametros,version);    

   va_copy(aux_parametros,parametros);

   if (Codifica_Metodo(&buffer,&tamano,Clase,Metodo,
				           version,parametros) != 0)
   {
      Excepcion_Registra(ERR_CODIFICA);
      va_end(parametros);
      return -1;
   }

   /* -----------------------------------------------------------
    *  Envia esta carta a traves del sistema de comunicaciones
    * correos.
    * ---------------------------------------------------------- */
   if (Correos_Depositar(-1,Destino,buffer,tamano) < 0)
     return -1;

   free(buffer);
   buffer = NULL;
   espera_retorno = 1;    /* A la espera de la constestacion */
   espera_retorno_timeout = 0;  /* El timeout del servicio aun no ha saltado */ 
   Reset_TimeoutServicioPendiente(); /* Timeout empieza a correr. */

punto_entrada:
   setjmp(jumper);

   /* ---------------------------------------------------------
    *  Si llegamos aqui por un retorno o no.
    * ---------------------------------------------------------- */
   if (espera_retorno)
     SRC_Loop();  /* Lee todo lo que llegue esperado la respuesta. */  
   else
   {
        /* ---------------------------------------------
         *  No llego la respuesta ? 
         * --------------------------------------------- */
      if (espera_retorno_timeout)
      {
         Excepcion_Registra(ERR_TIMEOUT);
         va_end(parametros);
         return -1;
      }

        /* ---------------------------------------------
         *  Llego la esperada respuesta.
         * --------------------------------------------- */

      Receptor_Actual(&resp_peticion);

        /* ---------------------------------------------
         *  Es lo que esperabamos. 
         * --------------------------------------------- */
      if (Decodifica_CabeceraMetodo(&aux_clase,&aux_metodo,
             &aux_version,resp_peticion->mensaje,resp_peticion->tamano) != 0)
      {
          espera_retorno = 1;
          free(resp_peticion->recibido);
	  resp_peticion->recibido = NULL;
          Receptor_LiberaActual();
          goto punto_entrada;
      }

      if ((strcmp(Clase,aux_clase) != 0) || (strcmp(Metodo,aux_metodo)!= 0) 
	                                          || (version != aux_version))
      {
          /* ---------------------------------------
           *  No es la esperada, ignorar.
           * ------------------------------------------ */
          Excepcion_Registra(ERR_RESPUESTA);
          espera_retorno = 1;
          free(resp_peticion->recibido);
	  resp_peticion->recibido = NULL;
          Receptor_LiberaActual();
          goto punto_entrada;
      }

      /* ---------------------------
       *  Respuesta es un error, registrar el error y retornar -1.
       * ----------------------------------------------------------- */
      if (resp_peticion->tipo == SERV_ERROR)
      {
         Decodifica_ErrorRespuesta(resp_peticion->mensaje,
                                   resp_peticion->tamano,
 				   &error);
         Excepcion_RegistraServicio(error);
	 free(error);
	 error = NULL;
         free(resp_peticion->recibido);
	 resp_peticion->recibido = NULL;
         Receptor_LiberaActual();
	 return -1;
      }

     /* ----------------------------------
      *  Saltamos los parametros de entrada.
      * -------------------------------------------- */
     v_parametro = va_arg(aux_parametros,char *);
     while (strcmp(v_parametro,"") != 0)
     {
        va_arg(aux_parametros,int);
        switch(*v_parametro)
        {
          case 'c': (void)va_arg(aux_parametros,int);
                    break;
          case 'i': (void)va_arg(aux_parametros,int);
                    break;
          case 'l': (void)va_arg(aux_parametros,long);
                    break;
          case 'f': (void)va_arg(aux_parametros,double);
                    break;
          case 'd': (void)va_arg(aux_parametros,double);
                    break;
          case 's': (void)va_arg(aux_parametros,char *);
                    break;
          case 'C': (void)va_arg(aux_parametros,char *);
                    break;
          case 'I': (void)va_arg(aux_parametros,int *);
                    break;
          case 'L': (void)va_arg(aux_parametros,long *);
                    break;
          case 'F': (void)va_arg(aux_parametros,float *);
                    break;
          case 'D': (void)va_arg(aux_parametros,double *);
                    break;
        }
       v_parametro = va_arg(aux_parametros,char *);
    }
    
    Decodifica_ParametrosRespuesta(resp_peticion->mensaje,
                                        resp_peticion->tamano,
 				        aux_parametros);
     
     free(resp_peticion->recibido);
     resp_peticion->recibido = NULL;
     Receptor_LiberaActual();
     va_end(parametros);    
   }
  return 0;
}

/* ---------------------------------------------------------
 *  
 *  Funcion: Peticion de Operacion 
 * 
 *   Enviar un mensaje de peticion de servicio de forma asincrona 
 * es decir sin esperar respuesta. Esto significa que nadie nos garantiza
 * que llegue o no la peticion al otro lado y se sirva.
 *
 *  Parametros
 *     Entrada:
 *              Destino, servidor de esta peticion.
 *              Clase,   clase del servicio solicitado.
 *              Metodo,  metodo del servicio a activar.
 *              Version, version del servicio a activar.
 *              Tipo y valor de los Parametros de Entrada  al
 *                        metodo.
 *     Retorna:
 *              0, ok.
 *              0<, Error algo fallo en la codificacion y envio.
 *
 * ------------------------------------------------------------------- */
int Emisor_PeticionOperacion(struct sockaddr_in Destino, char * Clase, 
                             char * Metodo, int version, ...)
{
   char * buffer;                /* Buffer para el mensaje a componer */
   int retorno;                  /* variable de retorno.  */
   va_list parametros;           /* recorrer la lista de parametros */
   int tamano = 0;               /* tammano del mensaje */


   /* -----------------------------------------------------------
    *  Recorre la lista de parametros y codificalos de forma 
    * independiente de la maquina en un buffer para su envio por
    * el sistema de comunicaciones.
    * ----------------------------------------------------------- */
   va_start(parametros,version);    

   if (Codifica_Metodo(&buffer,&tamano,Clase,Metodo,
				           version,parametros) != 0)
   {
      Excepcion_Registra(ERR_CODIFICA);
      va_end(parametros);
      return -1;
   }
   va_end(parametros);

   /* -----------------------------------------------------------
    *  Envia esta carta a traves del sistema de comunicaciones
    * correos.
    * ---------------------------------------------------------- */
   retorno = Correos_Depositar(-1,Destino,buffer,tamano);

   free(buffer);
   buffer = NULL;

   return retorno;
}

/* ---------------------------------------------------------
 *  
 *  Funcion: MensajeError.
 * 
 *   Enviar un mensaje de error al solicitante de un servicio u operacion. 
 *   En esta version no se implementa esta facilidad. 
 *
 *  Parametros
 *     Entrada:
 *              Destino,    servidor de este error.
 *              tipo_error, tipo del error a enviar.
 *              texto,      texto del error.
 *     Retorna:
 *              0, ok.
 *              0<, Error algo fallo en la codificacion y envio.
 *
 * ------------------------------------------------------------------- */
int Emisor_MensajeError(struct sockaddr_in Destino, int tipo_error,char * texto)
{
   return 0;
}

/* ---------------------------------------------------------
 *  
 *  Funcion: RespuestaProcedimiento. 
 * 
 *   Enviar de vuelta los parametros de salida del procedimiento
 *   disparado. 
 *
 *  Parametros
 *     Entrada:
 *              clase,      clase a la que se refiere la contestacion.
 *              metodo,     metodo a la que se refiere la contestacion.
 *              version,    version a la que se refiere la contestacion.
 *              parametros, parametros a enviar de vuelta. 
 *     Retorna:
 *              0, ok.
 *              0<, Error algo fallo en la codificacion y envio.
 *
 * ------------------------------------------------------------------- */
int Emisor_RespuestaProcedimiento(char * clase, char * metodo, int version,
				  s_parametros * p_salida)
{
  struct sockaddr_in destino;
  char * buffer;
  int  tamano = 0;
  int retorno;

  GetRequestAddress(&destino);
  if (Codifica_RespuestaMetodo(&buffer,&tamano,clase,metodo,
				           version,p_salida) != 0)
  {
      Excepcion_Registra(ERR_CODIFICA);
      return -1;
   }

  /* ----------------------------------------------------------
   *  Mandalo por correos de vuelta a origen.
   * ---------------------------------------------------------- */
  retorno = Correos_Depositar(-1,destino,buffer,tamano);

  /* ----------------------------------------------------------
   *  Libera el buffer del mensaje. 
   * ---------------------------------------------------------- */
  free(buffer);
  buffer = NULL;

  /* ----------------------------------------------------------
   *  Libera la memoria obtenida para los procedimientos del metodo. 
   * ---------------------------------------------------------- */
   while (strcmp(p_salida->tipo,"") != 0)
   {
     if (p_salida->valor != NULL) 
     {
       free(p_salida->valor);
       p_salida->valor = NULL;
     }
     p_salida++;
   }

  return retorno;

}

/* ---------------------------------------------------------
 *  
 *  Funcion: ErrorMetodo. 
 * 
 *   Enviar de vuelta un error, porque la version no es adecuada.... 
 *
 *  Parametros
 *     Entrada:
 *              clase,      clase a la que se refiere la contestacion.
 *              metodo,     metodo a la que se refiere la contestacion.
 *              error,      error producido. 
 *     Retorna:
 *              0, ok.
 *              0<, Error algo fallo en la codificacion y envio.
 *
 * ------------------------------------------------------------------- */
int Emisor_ErrorMetodo(char * clase, char * metodo, int version, char * error)
{
  struct sockaddr_in destino;
  char * buffer;
  int  tamano = 0;
  int retorno;


  GetRequestAddress(&destino);
  if (Codifica_ErrorMetodo(&buffer,&tamano,clase,metodo,
				           version,error) != 0)
  {
      Excepcion_Registra(ERR_CODIFICA);
      return -1;
  }

  /* ----------------------------------------------------------
   *  Mandalo por correos de vuelta a origen.
   * ---------------------------------------------------------- */
  retorno = Correos_Depositar(-1,destino,buffer,tamano);

  /* ----------------------------------------------------------
   *  Libera el buffer del mensaje. 
   * ---------------------------------------------------------- */
  free(buffer);
  buffer = NULL;

  return retorno;
}

