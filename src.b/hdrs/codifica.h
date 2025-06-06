#ifndef __CODIFICA
#define __CODIFICA
/* -------------------------------------------------------
 *  MODULO:Codifica.
 * 
 *  Modulo que traduce de y al esperanto, es decir convierte
 * los datos en independientes de maquina para su envio 
 * recepcion. 
 * ------------------------------------------------------- */

/* ---------------------------------------------------------------
 *  Definiciones de mensajes.
 * -------------------------------------------------------------------- */
#define SERV_METODO    'M'
#define SERV_RESPUESTA 'R'
#define SERV_ERROR     'E'

/* ---------------------------------------------------------------
 *  Tamano de los distintos tipos aplanados, segun la RFC1014 (XDR).
 * -------------------------------------------------------------------- */
#define TAMANYO_LONG_APLANADO 4
#define CAB_IDENTIFICATIVO  (TAMANYO_LONG_APLANADO + TAMANYO_LONG_APLANADO)

int Codifica_Mensaje(long identificativo, long nro_trozo,
                           char * mensaje, int logitud, char ** buffer);

int Decodifica_Mensaje(char * mensaje, 
                       long * identificativo, long * nro_trozo);

int Codifica_Metodo(char ** buffer,int * tamano, char * Clase, 
		          char * Metodo, int version, va_list parametros);

int Codifica_RespuestaMetodo(char ** buffer, int * tamano, char * Clase,
			     char * metodo, int version, 
                             s_parametros * parametros);

int Codifica_ErrorMetodo(char ** buffer, int * tamano, char * Clase,
			 char * metodo, int version,char * error);

int Decodifica_TipoMensaje(char * t_mensaje, char ** mensaje,int * longitud);

int Decodifica_CabeceraMetodo(char ** clase, char ** metodo,
			      int * version, char * mensaje, int longitud);

int Decodifica_ParametrosMetodo(s_parametros * p_entrada, char * mensaje);

int Decodifica_ParametrosRespuesta(char * mensaje, int tamano, 
                                                    va_list parametros);

int Decodifica_ErrorRespuesta(char * mensaje, int tamano,char ** error);

#endif /*__CODIFICA*/
