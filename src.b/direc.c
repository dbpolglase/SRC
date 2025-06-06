/* -------------------------------------------------------
 *  MODULO: Direcciones. 
 *
 *  Modulo de manejo de direcciones, traduce las direcciones 
 * logicas a direcciones fisicas y a la inversa.
 * --------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <src.h>
#include <direc.h>

#ifndef INADDR_NONE 
#define INADDR_NONE     0xffffffff
#endif

static struct sockaddr_in dir_actual;

/* --------------------------------------------------------------------
 *  Funcion: GetNormalAddress
 * 
 *  Dada una direccion logica, devulve una direccion fisica que puede
 * ser utilizada por el sistema de comunicaciones.
 *
 *  Esta implementacion de la libreria compone las direcciones logicas
 * como host + servicio.
 *
 *  Parametros:
 *    Entrada:
 *             dir_logica, direccion logica a traducir.
 *    Salida:
 *             dir_fisica, direccion fisica que se puede utilizar.
 *    Retorna:
 *             0, ok.
 *             0<, error.
 * ---------------------------------------------------------------------- */
int GetNormalAddress(struct sockaddr_in * dir_fisica, 
                           SRC_AddressType  dir_logica)
{
   struct servent * sp;
   struct hostent * hp;
   unsigned long inaddr;

   memset((char *)&(*dir_fisica),0,sizeof((*dir_fisica)));
   (*dir_fisica).sin_family = AF_INET;

   if ((sp = getservbyname(dir_logica.servicio,"udp")) == NULL)
      (*dir_fisica).sin_port = htons(atoi(dir_logica.servicio));
   else
      (*dir_fisica).sin_port = sp->s_port;

   if ((inaddr = inet_addr(dir_logica.host)) != INADDR_NONE)
   {
     memcpy((char *)&(*dir_fisica).sin_addr,(char *) &inaddr,sizeof(inaddr));
   }
   else
   {
     if ((hp = gethostbyname(dir_logica.host)) == NULL)
       return -1;
     memcpy((char *) &(*dir_fisica).sin_addr,hp->h_addr,hp->h_length);
   }

   return 0;
}
   
/* --------------------------------------------------------------------
 *  Funcion: GetLogicAddress
 * 
 *  Dada una direccion fisica, devuelve una direccion logica que puede
 * ser reconocida por la aplicacion. 
 *
 *  Esta implementacion de la libreria compone las direcciones logicas
 * como host + servicio.
 *
 *  Parametros:
 *    Entrada:
 *             dir_fisica, direccion fisica que se puede utilizar.
 *    Salida:
 *             dir_logica, direccion logica a traducir.
 *    Retorna:
 *             0, ok.
 *             0<, error.
 * ---------------------------------------------------------------------- */
int GetLogicAddress(struct sockaddr_in dir_fisica, 
                           SRC_AddressType  * dir_logica)
{
   struct hostent * hp;
   struct servent * sp;
  
   hp = gethostbyaddr((const char *)&dir_fisica.sin_addr,
		      sizeof(struct in_addr),AF_INET);

   sp = getservbyport(dir_fisica.sin_port,"udp"); 

   if (hp  != NULL)
    strcpy(dir_logica->host,hp->h_name);
   else
    strcpy(dir_logica->host,inet_ntoa(dir_fisica.sin_addr));

   if (sp != NULL)
    strcpy(dir_logica->servicio,sp->s_name);
   else
   {
    char sport[10];
    snprintf( sport, 10, "%d", dir_fisica.sin_port );
    strcpy(dir_logica->servicio,sport);
   }
   return 0;
}
   
/* --------------------------------------------------------------------
 *  Funcion: MakeAddress
 * 
 *  Dado un hosts y un puerto compone una direccion logica valida.
 *
 *  Parametros:
 *    Entrada:
 *             hosts, host que compone la direc. logica, si es NULL
 *                    suponemos que es el propio.
 *             servicio, servicio a utilizar, si NULL cualquiera.
 *    Salida:
 *             dir_logica, direccion logica a traducir.
 *    Retorna:
 *             0, ok.
 *             0<, error.
 * ---------------------------------------------------------------------- */
int MakeAddress(SRC_AddressType  * dir_logica,
                      char * host, char * servicio, int multicast)
{
  struct hostent * aux_host;

   /* ------------------------------------------------
    *  Uso multicast ?
    * -------------------------------------------------- */
   if (multicast == 1)
    dir_logica->multicast = 1;
   else
    dir_logica->multicast = 0;

   strcpy(dir_logica->servicio,servicio);
   /* -----------------------------------------------
    *  No direccion la nuestra. 
    * ---------------------------------------------------- */
   if (host == NULL || (strcmp(host,"") == 0)) { 
     if ((aux_host = gethostbyname("localhost")) == NULL)
       return -1;
     strcpy(dir_logica->host,aux_host->h_name);
   }
   else
   {
     /* ----------------------------------------
      *  Direccion bien formada ?
      * ---------------------------------------- */
      if (inet_addr(host) == -1 && (gethostbyname(host) == NULL))
        return -1;
      else
        strcpy(dir_logica->host,host);
   }

   return 0;
}
  
/* --------------------------------------------------------------------
 *  Funcion: Peticion
 * 
 *  Devuelve la direccion desde la que se realizo la peticion que se 
 * esta tratando en este momento.
 *
 *  Parametros:
 *    Salida:
 *             dir_logica, direccion logica.
 * ---------------------------------------------------------------------- */
void GetRequestAddress(struct sockaddr_in  * dir_logica)
{
   memcpy((char *)dir_logica,(char *)&dir_actual,sizeof(struct sockaddr_in));
}
  
/* --------------------------------------------------------------------
 *  Funcion: GrabarPeticion
 * 
 *  Indica la direccion desde la que se realiza la peticion que estamos 
 * tratando en este momento. 
 *
 *  Parametros:
 *    Entrada:
 *             dir_logica, direccion logica de la peticion.
 * ---------------------------------------------------------------------- */
void SetRequestAddress(struct sockaddr_in * dir_logica)
{
   memcpy((char *)&dir_actual,(char *)dir_logica,sizeof(struct sockaddr_in));
}

