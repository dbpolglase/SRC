/* -------------------------------------------------------------------
 *   MODULO: Codifica.
 *
 *    Traduce De/Hacia el "esperanto" de forma que todo el mundo pueda
 *  leer las cartas que enviamos y podamos leer las cartas que nos envian.
 *
 * -------------------------------------------------------------------- */

#include <stdarg.h>
#include <stdio.h>
#ifdef SOLARIS
#include <sys/select.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <stdarg.h>
#include <src.h>
#include <excepcion.h>
#include <textos.h>
#include <recep.h>
#include <correos.h>
#include <codifica.h>

/* ---------------------------------------------------------------
 *  Tamano de los distintos tipos aplanados, segun la RFC1014 (XDR).
 * -------------------------------------------------------------------- */
#define TAMANYO_LONG_APLANADO 4

#define CAB_IDENTIFICATIVO  (TAMANYO_LONG_APLANADO + TAMANYO_LONG_APLANADO)

static XDR vxdr;
/* -------------------------------------------------------------
 *
 *  Funcion: Metodo.
 * 
 *    Traslada los datos relativos a un metodo al esperanto:
 *
 *    Parametros
 *      Entrada:
 *                Clase, a la que pertenece el metodo a invocar. 
 *                Metodo, funcion a disparar de este metodo. 
 *                Version, version valida segun el emisor. 
 *                Parametros, parametros de este metodo.
 *
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Codifica_Metodo(char ** buffer,int * tamano, char * Clase, 
		          char * Metodo, int version, va_list parametros)
{
   
   va_list aux_parametros;
   char * tipo = NULL;
   char t_mensaje = SERV_METODO;
   char * string_variable;
   long long_variable;
   int  int_variable;
   int  aux_l;
   float  float_variable;
   double  double_variable;
   char char_variable;
   unsigned int  lon_array;
   int  long_aux;
   int * v_int;
   long * v_long;
   float * v_float;
   double * v_double;

   va_copy(aux_parametros,parametros);
    /* -------------------------------------------
     *  Calculo tamano cabecera:
     *
     *  Esta cabecera se compone de:
     *    Clase, Metodo y version y lo deben llevar todos los 
     *   mensajes de disparo de metodos. 
     * ---------------------------------------------------------------- */
    *tamano +=4;                               /* tipo mensaje. */

    *tamano += 4; /* Aqui se pone la long. de la clase */
    long_aux = strlen((char *)Clase);         /* tamano nombre clase */
    if (long_aux >= 4)
      *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
      *tamano += 4;
    *tamano += 4; /* Aqui se pone la long. del nombre del metodo */
    long_aux = strlen((char *)Metodo);        /* tamano nombre metodo. */
    if (long_aux >= 4)
      *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
      *tamano += 4;

    *tamano += 4;                             /* tamano version. */

   /* ----------------------------------
    *  Calculo del espacio requerido para los parametros 
    * ---------------------------------------------------- */
   tipo = va_arg(parametros,char *);

   while (tipo != NULL && strcmp(tipo,"") != 0)
   {
     lon_array = va_arg(parametros, int);
     switch(*tipo)
     {
       case 'c':
          *tamano += 4;
          char_variable = (char)va_arg(parametros, int);
          break;
       case 'i':
          *tamano += 4;
          int_variable = va_arg(parametros, int );
          break;
       case 'l':
          *tamano += 4;
          long_variable = va_arg(parametros, long );
          break;
       case 'f':
          *tamano += 4;
          float_variable = (float) va_arg(parametros, double);
          break;
       case 'd':
          *tamano += 8;
          double_variable = va_arg(parametros, double );
          break;
       case 's':
	  *tamano += 4;  /* El del int con la longitud. */
          string_variable = va_arg(parametros, char *);
          long_aux = strlen((char *)string_variable);
          if (long_aux >= 4)
             *tamano += long_aux + (4 - long_aux % 4) + 4;
          else
	     *tamano += 8;
          break;
       case 'C': 
          string_variable = va_arg(parametros, char *);
          *tamano +=4; 
          *tamano +=(4 * (lon_array));
          break;
       case 'I':
          va_arg(parametros, int *);
          *tamano +=4; 
          *tamano +=(4 * (lon_array));
          break;
       case 'L':
          va_arg(parametros, long *);
          *tamano +=4; 
          *tamano +=(4 * (lon_array));
          break;
       case 'F':
          va_arg(parametros, float *);
          *tamano +=4; 
          *tamano +=(4 * (lon_array));
          break;
       case 'S':
          break;
       case 'D':
          va_arg(parametros, double *);
          *tamano +=4; 
          *tamano +=(8 * (lon_array));
          break;
     }
     tipo = va_arg(parametros, char *);
   }
   
   /* ----------------------------------
    *  Ya se el tamano de la carta que requiere esta 
    * peticion de servicio, pido esta memoria. 
    * ---------------------------------------------------- */
  
   *buffer = (char *)malloc(*tamano + 20);
   if (*buffer == NULL)
   {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
   }

   /* ----------------------------------
    *  Y la relleno con los datos de la peticion. 
    * ---------------------------------------------------- */

   xdrmem_create(&vxdr,*buffer,*tamano,XDR_ENCODE);

   /* ----------------------------------
    *  Meto la cabecera en el buffer. 
    * ---------------------------------------------------- */
   if (!xdr_char(&vxdr,&t_mensaje))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

   if (!xdr_wrapstring(&vxdr,&Clase))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   if (!xdr_wrapstring(&vxdr,&Metodo))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   if (!xdr_int(&vxdr,&version))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   /* --------------------------- 
    * Y meto los parametros como puedo. 
    * --------------------------------------- */
   tipo = va_arg(aux_parametros,char *);

   while (tipo != NULL && strcmp(tipo,"") != 0)
   {
     lon_array = va_arg(aux_parametros,int);
     switch(*tipo)
     {
       case 'c':
                 char_variable = (char)va_arg(aux_parametros,int); 
                 if (!xdr_char(&vxdr,&char_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'i':
                 int_variable = va_arg(aux_parametros,int); 
                 if (!xdr_int(&vxdr,&int_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'l':
                 long_variable = va_arg(aux_parametros,long ); 
                 if (!xdr_long(&vxdr,&long_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'f':
                 float_variable = (float)va_arg(aux_parametros,double); 
                 if (!xdr_float(&vxdr,&float_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'd':
                 double_variable = va_arg(aux_parametros,double); 
                 if (!xdr_double(&vxdr,&double_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 's':
                 string_variable = va_arg(aux_parametros,char *); 
                 aux_l = strlen(string_variable);
                 if (!xdr_int(&vxdr,&aux_l))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 if (!xdr_wrapstring(&vxdr,&string_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'C':
                string_variable = va_arg(aux_parametros,char*); 
                if (!xdr_array(&vxdr,&string_variable, &lon_array,
                                  lon_array+1,sizeof(char),(xdrproc_t)xdr_char))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'I':
                v_int = va_arg(aux_parametros,int*); 
                if (!xdr_array(&vxdr,(char **)&v_int,
                              &lon_array,lon_array+1,sizeof(int),(xdrproc_t)xdr_int))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'L':
                v_long = va_arg(aux_parametros,long*); 
                if (!xdr_array(&vxdr,(char **)&v_long, &lon_array,
                                  lon_array+1,sizeof(long),(xdrproc_t)xdr_long))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'F':
                v_float = va_arg(aux_parametros,float*); 
                if (!xdr_array(&vxdr,(char **)&v_float,
                               &lon_array,lon_array+1,sizeof(float),(xdrproc_t)xdr_float))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;
       case 'D':
                v_double = va_arg(aux_parametros,double*); 
                if (!xdr_array(&vxdr,(char **)&v_double,
                          &lon_array,lon_array+1,sizeof(double),(xdrproc_t)xdr_double))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   free(*buffer);
                   *buffer = NULL;
                   return -1;
                 }
                 break;

     }
     tipo = va_arg(aux_parametros,char *); 
   }
   
   return 0;
}


/* -------------------------------------------------------------
 *
 *  Funcion: Mensaje.
 * 
 *    Traslada los datos de un trozo de paquete al esperanto.
 *
 *    Parametros
 *      Entrada:
 *                Identificativo, identificativo del paquete.
 *                NroTrozo,       pedazo del mismo que enviamos.
 *                UltimoTrozo,    no hay mas trozos o si. 
 *                Mensaje,        mensaje de este trozo.
 *                logitud,        logitud del mismo.
 *     Salida:
 *                buffer,         carta lista para su envio.
 *
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Codifica_Mensaje(long identificativo, long nro_trozo,
                           char * mensaje, int logitud, char ** buffer)
{

   xdrmem_create(&vxdr,*buffer,CAB_IDENTIFICATIVO,XDR_ENCODE);
   if (!xdr_long(&vxdr,&identificativo))
   {
     Excepcion_Registra(ERR_CODIFICA);
     return -1;
   }
   if(!xdr_long(&vxdr,&nro_trozo))
   {
     Excepcion_Registra(ERR_CODIFICA);
     return -1;
   }

   memcpy(*buffer+CAB_IDENTIFICATIVO,mensaje,logitud);
   return 0;
}

/* -------------------------------------------------------------
 *
 *  Funcion: Decodifica_CabeceraMetodo.
 * 
 *    Esta decodificacion se debe realizar en dos partes,
 *  primero la cabecera y luego los parametros. ya que tenemos
 *  que saber primero que parametros esperamos en el mensaje.
 *
 *    Parametros
 *      Entrada:
 *                Mensaje, mensaje con la peticion codificada. 
 *                logitud, tamano del mismo.
 *      Salida:
 *                Clase, clase a la que pertence el metodo solitado. 
 *                metodo, metodo solicitado. 
 *                version, version del mismo.
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Decodifica_CabeceraMetodo(char ** clase, char ** metodo, 
                              int * version, char * mensaje,int longitud)
{

  xdrmem_create(&vxdr,mensaje,longitud,XDR_DECODE);

  /* ---------------------------------------------------------
   *  Obtener la cabecera de la llamada al metodo.
   * ---------------------------------------------------------- */
  if (!xdr_wrapstring(&vxdr,clase))
  {
     Excepcion_Registra(ERR_DECODIFICA);
     return -1;
  }
  if (!xdr_wrapstring(&vxdr,metodo))
  {
     Excepcion_Registra(ERR_DECODIFICA);
     return -1;
  }
  if (!xdr_int(&vxdr,version))
  {
     Excepcion_Registra(ERR_DECODIFICA);
     return -1;
  }

  return 0;
}

/* -------------------------------------------------------------
 *
 *  Funcion: Decodifica_ParametrosMetodo.
 * 
 *    Segunda parte de la decodificacion del metodo, decodificacion 
 *  de los parametros del mismo. 
 *
 *    Parametros
 *      Entrada:
 *                Mensaje, mensaje con los parametros codificados. 
 *      Salida:
 *                p_entrada, estructura donde se dejan los valores
 *                           de los parametros leidos.
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Decodifica_ParametrosMetodo(s_parametros * p_entrada, char * mensaje)
{
 int l_string;

  /* --------------------------------------------------------
   *   Para cada parametro recibido en el mensaje de acuerdo
   * con el tipo esperado, lo decodifico y lo introduzco en 
   * una trozo de memoria que he creado al efecto.
   * ---------------------------------------------------------- */
  while (strcmp(p_entrada->tipo,"")!= 0)
  {
     switch(*p_entrada->tipo)
     {
       case 'c':
                 if (p_entrada->valor == NULL)
                 {
                   if ((p_entrada->valor = 
                             (void *) malloc(sizeof(int))) == NULL)
         
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
                 }

                if (!xdr_char(&vxdr,(char *)p_entrada->valor))
                {
                  Excepcion_Registra(ERR_DECODIFICA);
                  return -1;
                }
                break;
        case 'i':
                if (p_entrada->valor == NULL)
                {
                   if ((p_entrada->valor =  
                        (void *)malloc(sizeof(int))) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
                }
                if (!xdr_int(&vxdr,(int *)p_entrada->valor))
                {      
                    Excepcion_Registra(ERR_DECODIFICA);
                    return -1;
                }
                break;
        case 'l':
                if (p_entrada->valor == NULL)
                {
                   if ((p_entrada->valor = 
                                  (void *)malloc(sizeof(long))) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	              return -1;
                   }
                }

                if (!xdr_long(&vxdr,(long *)p_entrada->valor))
                {
                  Excepcion_Registra(ERR_DECODIFICA);
                  return -1;
                }
              break;
        case 'f':
              if (p_entrada->valor == NULL)
              {
                if ((p_entrada->valor = (void *)malloc(sizeof(double))) == NULL)
                {
                  Excepcion_Registra(ERR_MALLOC);
	          return -1;
                }
              }
              if (!xdr_float(&vxdr,(float *)p_entrada->valor))
              {
                Excepcion_Registra(ERR_DECODIFICA);
                return -1;
              }
              break;

        case 'd':
              if (p_entrada->valor == NULL)
              {
                if ((p_entrada->valor = (void *)malloc(sizeof(double))) == NULL)
                {
                  Excepcion_Registra(ERR_MALLOC);
       	          return -1;
                }
              }
              if (!xdr_double(&vxdr,(double *)p_entrada->valor))
              {
                  Excepcion_Registra(ERR_DECODIFICA);
                  return -1;
              }
              break;

        case 's':
              if (!xdr_int(&vxdr,&l_string))
              {
                Excepcion_Registra(ERR_DECODIFICA);
                return -1;
              }
              if (p_entrada->valor != NULL) 
  	      {
		free(p_entrada->valor);
		p_entrada->valor = NULL;
              }
              p_entrada->valor = (void *)malloc((l_string) + 4);
              if ((p_entrada->valor) == NULL)
              {
                Excepcion_Registra(ERR_MALLOC);
	        return -1;
              }
              if (!xdr_wrapstring(&vxdr,(char **)&p_entrada->valor))
              {
                Excepcion_Registra(ERR_DECODIFICA);
                return -1;
              }
              break;
        case 'C':
              if (p_entrada->valor == NULL)
              {
                   if ((p_entrada->valor = 
                       (void *) malloc(sizeof(char) * p_entrada->lon)) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
               }

               if (!xdr_array(&vxdr,(char **)&p_entrada->valor,&p_entrada->lon,
			        p_entrada->lon+1,sizeof(char),(xdrproc_t)xdr_char))
               {
                   Excepcion_Registra(ERR_DECODIFICA);
                   return -1;
               }
               break;
        case 'I':
               if (p_entrada->valor == NULL)
               {
                   if ((p_entrada->valor = 
                       (void *) malloc(sizeof(int) * p_entrada->lon)) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
               }

               if (!xdr_array(&vxdr,(char **)&p_entrada->valor,&p_entrada->lon,
			        p_entrada->lon+1,sizeof(int),(xdrproc_t)xdr_int))
               {
                   Excepcion_Registra(ERR_DECODIFICA);
                   return -1;
               }
               break;
        case 'L':
               if (p_entrada->valor == NULL)
               {
                   if ((p_entrada->valor = 
                       (void *) malloc(sizeof(long) * p_entrada->lon)) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
               }

               if (!xdr_array(&vxdr,(char **)&p_entrada->valor,&p_entrada->lon,
			        p_entrada->lon+1,sizeof(long),(xdrproc_t)xdr_long))
               {
                   Excepcion_Registra(ERR_DECODIFICA);
                   return -1;
               }
               break;
        case 'F':
               if (p_entrada->valor == NULL)
               {
                   if ((p_entrada->valor = 
                      (void *) malloc(sizeof(float) * p_entrada->lon)) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
               }

               if (!xdr_array(&vxdr,(char **)&p_entrada->valor,&p_entrada->lon,
			        p_entrada->lon+1,sizeof(float),(xdrproc_t)xdr_float))
               {
                   Excepcion_Registra(ERR_DECODIFICA);
                   return -1;
               }
               break;
        case 'D':
               if (p_entrada->valor == NULL)
               {
                   if ((p_entrada->valor = 
                      (void *) malloc(sizeof(double) * p_entrada->lon)) == NULL)
                   {
                     Excepcion_Registra(ERR_MALLOC);
	             return -1;
                   }
                 }

               if (!xdr_array(&vxdr,(char **)&p_entrada->valor,&p_entrada->lon,
			        p_entrada->lon+1,sizeof(double),(xdrproc_t)xdr_double))
                 {
                   Excepcion_Registra(ERR_DECODIFICA);
                   return -1;
                 }
                 break;

     }
     p_entrada ++; 
   }
   return 0;
}

/* -------------------------------------------------------------
 *
 *  Funcion: Decodifica_Mensaje.
 * 
 *    Traslada los datos de un trozo de paquete desde esperanto.
 *
 *    Parametros
 *      Entrada:
 *                Mensaje,        mensaje de este trozo.
 *     Salida:
 *                Identificativo, identificativo del paquete.
 *                NroTrozo,       pedazo del mismo que enviamos.
 *
 *       Retorna:
 *                 >0, tamano leido del mensaje. 
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Decodifica_Mensaje(char * mensaje, long * identificativo,
                                                long * numero_trozo)
{

   xdrmem_create(&vxdr,mensaje,CAB_IDENTIFICATIVO,XDR_DECODE);
   if (!xdr_long(&vxdr,identificativo))
   {
     Excepcion_Registra(ERR_DECODIFICA);
     return -1;
   }
   if(!xdr_long(&vxdr,numero_trozo))
   {
     Excepcion_Registra(ERR_DECODIFICA);
     return -1;
   }

   return CAB_IDENTIFICATIVO;
}

/* -------------------------------------------------------------
 *
 *  Funcion: Decodifica_TipoPeticion.
 * 
 *    Decodifica el tipo de mensaje que enviamos, peticion, respuesta,
 *  excepcion,... 
 *
 *    Parametros
 *      Entrada:
 *                t_mensaje,        tipo de mensaje. 
 *                buffer,           en buffer.
 *                tamano,           con un tamano de.
 *       Retorna:
 *                 0, ok. 
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Decodifica_TipoMensaje(char * t_mensaje, char **buffer, int * tamano)
{

   xdrmem_create(&vxdr,*buffer,4,XDR_DECODE);
   if (!xdr_char(&vxdr,t_mensaje))
   {
     Excepcion_Registra(ERR_CODIFICA);
     return -1;
   }
   *tamano = 4;
   return 0;
}

/* -------------------------------------------------------------
 *
 *  Funcion: ParametrosRespuesta.
 * 
 *    Traslada los datos relativos a una respuesta del esperanto 
 * a las variables dadas.
 *
 *    Parametros
 *      Entrada:
 *                mensaje, buffer con la respuesta.
 *                tamano,  tamano de dicho buffer.
 *                parametros, lista de parametros a rellenar.
 *
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Decodifica_ParametrosRespuesta(char * respuesta,int tamano,
                                                  va_list parametros)
{
   int l_string;
   char * tipo;
   int   int_variable, * v_int;
   long  long_variable,* v_long;
   char  char_variable,* v_char;
   float float_variable,*v_float;
   double double_variable,*v_double;
   unsigned int lon_array;
   char ** v_string;
   char * string;

  /* ----------------------------------
   *  Decodifico los parametros de respuesta. 
   * ---------------------------------------------------- */
   tipo = va_arg(parametros,char *);

   while (tipo != NULL && strcmp(tipo,"") != 0)
   {
     lon_array = va_arg(parametros,int);
     switch(*tipo)
     {
       case 'c':
                 v_char = va_arg(parametros,char *); 
                 if (!xdr_char(&vxdr,&char_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 *v_char = char_variable;
                 break;
      case 'i':
                 v_int = va_arg(parametros,int *); 
                 if (!xdr_int(&vxdr,&int_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 *v_int = int_variable;
                 break;
      case 'l':
                 v_long = va_arg(parametros,long *); 
                 if (!xdr_long(&vxdr,&long_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 *v_long = long_variable;
                 break;
      case 'f':
                 v_float = va_arg(parametros,float *); 
                 if (!xdr_float(&vxdr,&float_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 *v_float = float_variable;
                 break;
      case 'd':
                 v_double = va_arg(parametros,double *); 
                 if (!xdr_double(&vxdr,&double_variable))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 *v_double = double_variable;
                 break;
      case 's':
                 if (!xdr_int(&vxdr,&l_string))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 v_string = va_arg(parametros,char **); 
                 string = (char *)malloc(l_string+ 200);
                 if ((string) == NULL)
                 {
                   Excepcion_Registra(ERR_MALLOC);
	           return -1;
                 }
                 if (!xdr_wrapstring(&vxdr,&string))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 *v_string = string;
                 break;
       case 'C':
                 v_char = va_arg(parametros,char *); 
                 if (!xdr_array(&vxdr,(char **)&v_char,&lon_array,
				    lon_array+1,sizeof(char),(xdrproc_t)xdr_char))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 break;
       case 'I':
                 v_int = va_arg(parametros,int *); 
                 if (!xdr_array(&vxdr,(char **)&v_int,&lon_array,
				    lon_array+1,sizeof(int),(xdrproc_t)xdr_int))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 break;
       case 'L':
                 v_long = va_arg(parametros,long *); 
                 if (!xdr_array(&vxdr,(char **)&v_long,&lon_array,
				    lon_array+1,sizeof(long),(xdrproc_t)xdr_long))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 break;
       case 'F':
                 v_float = va_arg(parametros,float *); 
                 if (!xdr_array(&vxdr,(char **)&v_float,&lon_array,
				    lon_array+1,sizeof(float),(xdrproc_t)xdr_float))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 break;
       case 'D':
                 v_double = va_arg(parametros,double *); 
                 if (!xdr_array(&vxdr,(char **)&v_double,&lon_array,
				    lon_array+1,sizeof(double),(xdrproc_t)xdr_double))
                 {
                   Excepcion_Registra(ERR_CODIFICA);
                   return -1;
                 }
                 break;
     }
     tipo = va_arg(parametros,char *);
   }
   return 0;
}

/* -------------------------------------------------------------
 *
 *  Funcion: RespuestaMetodo.
 * 
 *    Traslada los datos relativos a los parametros de un metodo 
 *  de vuelta al origen de la peticion.
 *
 *    Parametros
 *      Entrada:
 *                Clase, a la que pertenece el metodo a invocar. 
 *                Metodo, funcion a disparar de este metodo. 
 *                Version, version valida segun el emisor. 
 *                Parametros, parametros de este metodo.
 *
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Codifica_RespuestaMetodo(char ** buffer,int * tamano, char * Clase, 
		             char * Metodo, int version, 
                             s_parametros * parametros)
{
   
   char t_mensaje = SERV_RESPUESTA;
   int  long_aux;
   int  aux_l;
   s_parametros * aux_parametros;

    aux_parametros = parametros;

    /* -------------------------------------------
     *  Calculo tamano cabecera:
     *
     *  Esta cabecera se compone de:
     *    Clase, Metodo y version y lo deben llevar todos los 
     *   mensajes de disparo de metodos. 
     * ---------------------------------------------------------------- */
    *tamano +=4;                               /* tipo mensaje. */

    *tamano += 4; /* Aqui se pone la long. del nombre de la clase */
    long_aux = strlen((char *)Clase);         /* tamano nombre clase */
    if (long_aux >= 4)
      *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
      *tamano += 4;

    *tamano += 4; /* Aqui se pone la long. del nombre del metodo */
    long_aux = strlen((char *)Metodo);        /* tamano nombre metodo. */
    if (long_aux >= 4)
      *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
      *tamano += 4;

    *tamano += 4;                             /* tamano version. */

   /* ----------------------------------
    *  Calculo del espacio requerido para los parametros 
    * de retorno.
    * ---------------------------------------------------- */
  while (strcmp(aux_parametros->tipo,"")!= 0)
  {
    switch(*aux_parametros->tipo)
    {
     case 'c':
       *tamano += 4;
       break;
     case 'i':
       *tamano += 4;
       break;
     case 'l':
       *tamano += 4;
       break;
     case 'f':
       *tamano += 4;
       break;
     case 'd':
       *tamano += 8;
       break;
     case 's':
     {
       *tamano += 4;  /* El del long con la longitud */
       long_aux = strlen((char *)aux_parametros->valor);
       if (long_aux >= 4)
          *tamano += long_aux + (4 - long_aux % 4) + 4;
       else
	  *tamano += 8;
       break;
     }
     case 'C': case 'I': case 'L': case 'F': 
	  *tamano += 4 + (aux_parametros->lon * 4);
          break;
     case 'D':
	  *tamano += 8 + (aux_parametros->lon * 8);
          break;
    }
    aux_parametros ++; 
  }

   /* ----------------------------------
    *  Ya se el tamano de la carta que requiere esta 
    * respuesta de servicio, pido esta memoria. 
    * ---------------------------------------------------- */
   *buffer = (char *)malloc(*tamano + 20);
   if (*buffer == NULL)
   {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
   }

   /* ----------------------------------
    *  Y la relleno con los datos de la peticion. 
    * ---------------------------------------------------- */

   xdrmem_create(&vxdr,*buffer,*tamano,XDR_ENCODE);

   /* ----------------------------------
    *  Meto la cabecera en el buffer. 
    * ---------------------------------------------------- */
   if (!xdr_char(&vxdr,&t_mensaje))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

   if (!xdr_wrapstring(&vxdr,&Clase))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   if (!xdr_wrapstring(&vxdr,&Metodo))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   if (!xdr_int(&vxdr,&version))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

   /* --------------------------- 
    * Y meto los parametros como puedo. 
    * --------------------------------------- */
   aux_parametros = parametros;

   while (strcmp(aux_parametros->tipo,"") != 0)
   {
     switch(*aux_parametros->tipo)
     {
       case 'c':
          {
            if (!xdr_char(&vxdr,(char *)aux_parametros->valor))
            {
              Excepcion_Registra(ERR_CODIFICA);
              free(*buffer);
              *buffer = NULL;
              return -1;
            }
            break;
     	 }
     case 'i':
     	{
       	    if (!xdr_int(&vxdr,(int *)aux_parametros->valor))
       	    {
         	Excepcion_Registra(ERR_CODIFICA);
         	free(*buffer);
                *buffer = NULL;
         	return -1;
       	    }
            break;
     	}
     case 'l':
        {
       	    if (!xdr_long(&vxdr,(long *)aux_parametros->valor))
       	    {
                Excepcion_Registra(ERR_CODIFICA);
         	free(*buffer);
                *buffer = NULL;
         	return -1;
            }
            break;
     	}
     case 'f':
        {
            if (!xdr_float(&vxdr,(float *)aux_parametros->valor))
       	    {
         	Excepcion_Registra(ERR_CODIFICA);
         	free(*buffer);
                *buffer = NULL;
         	return -1;
       	    }
            break;
        }
     case 'd':
        {
            if (!xdr_double(&vxdr,(double *)aux_parametros->valor))
            {
               Excepcion_Registra(ERR_CODIFICA);
               free(*buffer);
               *buffer = NULL;
               return -1;
            }
            break;
        }
     case 's':
        {
           aux_l = strlen(aux_parametros->valor);
           if (!xdr_int(&vxdr,&aux_l))
           {
             Excepcion_Registra(ERR_CODIFICA);
             free(*buffer);
             *buffer = NULL;
             return -1;
           }
           if (!xdr_wrapstring(&vxdr,(char **)&aux_parametros->valor))
           {
             Excepcion_Registra(ERR_CODIFICA);
             free(*buffer);
             *buffer = NULL;
             return -1;
           }
           break;
         }
      case 'C':
          {
            if (!xdr_array(&vxdr,(char **)&aux_parametros->valor, 
                           &aux_parametros->lon,aux_parametros->lon+1,
                           sizeof(char),(xdrproc_t)xdr_char))
            {
              Excepcion_Registra(ERR_CODIFICA);
              free(*buffer);
              *buffer = NULL;
              return -1;
            }
            break;
          }
       case 'I':
          {
            if (!xdr_array(&vxdr,(char **)&aux_parametros->valor, 
                           &aux_parametros->lon,aux_parametros->lon+1,
                           sizeof(int),(xdrproc_t)xdr_int))
            {
              Excepcion_Registra(ERR_CODIFICA);
              free(*buffer);
              *buffer = NULL;
              return -1;
            }
            break;
         }
       case 'L':
          {
            if (!xdr_array(&vxdr,(char **)&aux_parametros->valor, 
                           &aux_parametros->lon,aux_parametros->lon+1,
                           sizeof(long),(xdrproc_t)xdr_long))
            {
              Excepcion_Registra(ERR_CODIFICA);
              free(*buffer);
              *buffer = NULL;
              return -1;
            }
            break;
         }
       case 'F':
          {
            if (!xdr_array(&vxdr,(char **)&aux_parametros->valor, 
                           &aux_parametros->lon,aux_parametros->lon+1,
                           sizeof(float),(xdrproc_t)xdr_float))
            {
              Excepcion_Registra(ERR_CODIFICA);
              free(*buffer);
              *buffer = NULL;
              return -1;
            }
            break;
         }
       case 'D':
          {
            if (!xdr_array(&vxdr,(char **)&aux_parametros->valor, 
                           &aux_parametros->lon,aux_parametros->lon+1,
                           sizeof(double),(xdrproc_t)xdr_double))
            {
              Excepcion_Registra(ERR_CODIFICA);
              free(*buffer);
              *buffer = NULL;
              return -1;
            }
            break;
         }
    }
    aux_parametros ++;
   }

   return 0;
}


/* -------------------------------------------------------------
 *
 *  Funcion: ErrorMetodo.
 * 
 *    Traslada los datos relativos a un error de vuelta al nodo 
 *  que realizo la peticion. 
 *
 *    Parametros
 *      Entrada:
 *                Clase, a la que pertenece el metodo a invocar. 
 *                Metodo, funcion a disparar de este metodo. 
 *                Version, version valida segun el emisor. 
 *                Error, parametros de este metodo.
 *
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Codifica_ErrorMetodo(char ** buffer,int * tamano, char * Clase, 
		             char * Metodo, int version,char * error)
{
   
   char t_mensaje = SERV_ERROR;
   int  long_aux;
   int  aux_l;


    /* -------------------------------------------
     *  Calculo tamano cabecera:
     *
     *  Esta cabecera se compone de:
     *    Clase, Metodo y version y lo deben llevar todos los 
     *   mensajes de disparo de metodos. 
     * ---------------------------------------------------------------- */
    *tamano +=4;                               /* tipo mensaje. */

    *tamano += 4; /* Aqui se pone la long. del nombre de la clase */
    long_aux = strlen((char *)Clase);         /* tamano nombre clase */
    if (long_aux >= 4)
      *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
      *tamano += 4;

    *tamano += 4; /* Aqui se pone la long. del nombre del metodo */
    long_aux = strlen((char *)Metodo);        /* tamano nombre metodo. */
    if (long_aux >= 4)
      *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
      *tamano += 4;

    *tamano += 4;                             /* tamano version. */

    long_aux = strlen((char *)error); /* tamano error */
    *tamano +=4; /* el del la longitud */
    if (long_aux >= 4)
        *tamano += long_aux + (4 - long_aux % 4) + 4;
    else
        *tamano += 8;

   /* ----------------------------------
    *  Ya se el tamano de la carta que requiere esta 
    * respuesta de servicio, pido esta memoria. 
    * ---------------------------------------------------- */
   *buffer = (char *)malloc(*tamano + 20);
   if (*buffer == NULL)
   {
     Excepcion_Registra(ERR_MALLOC);
     return -1;
   }

   /* ----------------------------------
    *  Y la relleno con los datos de la peticion. 
    * ---------------------------------------------------- */

   xdrmem_create(&vxdr,*buffer,*tamano,XDR_ENCODE);

   /* ----------------------------------
    *  Meto la cabecera en el buffer. 
    * ---------------------------------------------------- */
   if (!xdr_char(&vxdr,&t_mensaje))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

   if (!xdr_wrapstring(&vxdr,&Clase))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   if (!xdr_wrapstring(&vxdr,&Metodo))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

   if (!xdr_int(&vxdr,&long_aux))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

  /* --------------------------- 
   * Y meto el error. 
   * --------------------------------------- */
   aux_l = strlen(error);
   if (!xdr_int(&vxdr,&aux_l))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }
   if (!xdr_wrapstring(&vxdr,(char **)&error))
   {
     Excepcion_Registra(ERR_CODIFICA);
     free(*buffer);
     *buffer = NULL;
     return -1;
   }

   return 0;
}

/* -------------------------------------------------------------
 *
 *  Funcion: ErrorRespuesta.
 * 
 *    Traslada los datos relativos a una respuesta del esperanto 
 * a las variables dadas.
 *
 *    Parametros
 *      Entrada:
 *                mensaje, buffer con la respuesta.
 *                tamano,  tamano de dicho buffer.
 *                error, error dado. 
 *
 *       Retorna:
 *                 0, de puta madre.
 *                 0<, problemas.
 * 
 * ------------------------------------------------------------- */
int Decodifica_ErrorRespuesta(char * respuesta,int tamano, char ** error)
{
  int l_string;

   if (!xdr_int(&vxdr,&l_string))
   {
     Excepcion_Registra(ERR_DECODIFICA);
     return -1;
   }
   *error = (char *)malloc(l_string+ 2);
/*
   if (!xdr_string(&vxdr,error,(l_string + 2)))
*/
   if (!xdr_wrapstring(&vxdr,error))
   {
     Excepcion_Registra(ERR_CODIFICA);
     return -1;
   }
   return 0;
}

