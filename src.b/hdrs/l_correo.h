#ifndef __LCORREOS
#define __LCORREOS

/* ----------------------------------------------------------------
 *   Modulo: Lista Correos.
 *
 *   Recompone los pedazos de cartas recibidas. 
 * ----------------------------------------------------------------- */ 
int LCorreos_NuevoTrozo(int que_buzon, struct sockaddr_in origen,
		        char * trozo, int t_trozo);
int LCorreos_Obtener(int * que_buzon, struct sockaddr_in * origen,
		        char ** mensaje, int * t_mensaje);
#endif /* __LCORREOS */

