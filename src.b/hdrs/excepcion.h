#ifndef __EXCEPCION
#define __EXCEPCION
/* -----------------------------------------------------------
 *   Interfaz MODULO 
 *   EXCEPCIONES, control de excepciones. 
 * ----------------------------------------------------------- */
/* ----------------------------------------
 *   Registra una excepcion o error.
 * ------------------------------------------------------------ */
void Excepcion_Registra(int error);
/* ----------------------------------------
 *   Registra una excepcion recibida. 
 * ------------------------------------------------------------ */
void Excepcion_RegistraServicio(char * error);
/* ----------------------------------------
 *   Consulta una excepcion o error.
 * ------------------------------------------------------------ */
void Excepcion_Consulta(int * error, int * nivel, char ** texto);
/* ----------------------------------------
 *   Escribe por el stderr el error. 
 * ------------------------------------------------------------ */
void Excepcion_Escribe(void);
/* ----------------------------------------
 *   Borra el ultimo error. 
 * ------------------------------------------------------------ */
void Excepcion_Limpia(void);
/* ----------------------------------------
 *  Manejador de excepciones de la aplicacion. 
 * ------------------------------------------------------------ */
void Excepcion_Manejador(void (*f_manejador)(int,int,char *));
#endif /* __EXCEPCION */
