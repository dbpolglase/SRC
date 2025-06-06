#ifndef __DIREC
#define __DIREC

/* -----------------------------------------------------------
 *
 *  MODULO: Direcciones 
 *
 *  Traduce entre direcciones logicas y fisicas.
 * 
 * ------------------------------------------------------------- */

/* ----------------------------------------
 * Obtiene una direccion fisica de una logica.
 * --------------------------------------------------- */
int GetNormalAddress( struct sockaddr_in * ,SRC_AddressType logicAddress);
/* ----------------------------------------
 * Obtiene una direccion logica de una fisica.
 * --------------------------------------------------- */
int GetLogicAddress( struct sockaddr_in  ,SRC_AddressType * logicAddress);
/* ----------------------------------------
 * Compone una direccion logica. 
 * --------------------------------------------------- */
int MakeAddress( SRC_AddressType *, char * ,char *, int );
/* ------------------------------------------
 *  indicar la direccion de la peticion que estamos tratando.
 * -------------------------------------------------------------- */
void SetRequestAddress(struct sockaddr_in *);
/* ------------------------------------------
 *  Obtener la direccion de la peticion que estamos tratando.
 * -------------------------------------------------------------- */
void GetRequestAddress(struct sockaddr_in *);

#endif /* __DIREC*/
