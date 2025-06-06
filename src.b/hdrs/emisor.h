#ifndef __EMISOR
#define __EMISOR
/* -------------------------------------------------------
 *  MODULO: Emisor.
 * 
 *   Interfaz del modulo de envio de solicitud de servicios. 
 * ----------------------------------------------------------- */

/* -----------------------------------------------------------
 *  Envio de peticion de servicio con espera de contestacion.
 * ------------------------------------------------------------ */
int Emisor_PeticionOperacion(struct sockaddr_in destino, char * clase, 
                                 char * metodo, int version, ...);
/* -----------------------------------------------------------
 *  Envio de peticion de servicio sin espera de contestacion.
 * ------------------------------------------------------------ */
int Emisor_PeticionProcedimiento(struct sockaddr_in destino, char * clase, 
                                   char * metodo, int version, ...);

/* -----------------------------------------------------------
 *  Envio de la respuesta a un procedimiento. 
 * ------------------------------------------------------------ */
int Emisor_RespuestaProcedimiento(char * clase, char * metodo, 
                                  int version, s_parametros * p_salida);

/* -----------------------------------------------------------
 *  Envio un mensaje de error. 
 * ------------------------------------------------------------ */
int Emisor_MensajeError(struct sockaddr_in destino, 
			int tipo_error, char * error);


/* -----------------------------------------------------------
 *  Envio un mensaje de error como respuesta a un metodo.
 * ------------------------------------------------------------ */
int Emisor_ErrorMetodo(char * clase, char * metodo,int version, char * error);

#endif /* __EMISOR */
