/* ------------------------------------------------------------------
 *   MODULO: main.
 *   
 *   Modulo principal o de control.
 * ------------------------------------------------------------------- */
#include <sys/types.h>
#include <src.h>
#include <principal.h>
#include <recep.h>
#include <emisor.h>
#include <correos.h>
#include <direc.h>

extern int espera_retorno;
extern int peticiones_pendientes;

int SRC_BuzonEstablecer(int buzon);
int SRC_AsignarBuzon(int buzon);
/* ------------------------------------------------------------------
 *   Funcion: SRC_Open.
 *   
 *   Inicializa los datos necesarios para que el invento funcione,
 *  crea el socket de escucha. 
 * 
 *    Parametros:
 *       Entrada:
 *            host, maquina en la que estamos, si NULL la propia.
 *            servicio, servicio con el que nos tenemos que registrar,
 *                  si NULL cualquiera. 
 *      Retorna:
 *           0<, error.
 *           0, ok.
 * ------------------------------------------------------------------- */
int SRC_Open(char * host, char * servicio,int multicast)
{
  SRC_AddressType MiDireccion;

  if (MakeAddress(&MiDireccion,host,servicio,multicast) < 0)
    return -1;

  return(Correos_Crear(MiDireccion));
}
   
  
/* ------------------------------------------------------------------
 *   Funcion: SRC_End.
 *   
 *   Cerramos el quiosko, cierre del socket.
 * 
 *    Parametros:
 *      Retorna:
 *           0<, error.
 *           0, ok.
 * ------------------------------------------------------------------- */
int SRC_End()
{
  Correos_Destruir();
  return 0;
}

int SRC_DescriptorEnd(int descriptor)
{
  int que_buzon;

  if ((que_buzon = Correos_BuzonDeDescriptor(descriptor)) < 0)
     return que_buzon;
  Correos_DestruirBuzon(que_buzon);
  return 0;
}
   
   
/* ------------------------------------------------------------------
 *   Funcion: SRC_DescriptorLoop.
 *   
 *   Estamos ante un servidor modelo, todo el tiempo lo dedica a 
 * servir las peticiones que nos lleguen. 
 * 
 * ------------------------------------------------------------------- */
int SRC_DescriptorLoop(int descriptor)
{
  struct sockaddr_in Origen;
  char * peticion;
  int logitud;
  int que_buzon;

   while(1)
   {
      if ((que_buzon = Correos_BuzonDeDescriptor(descriptor)) < 0)
        return que_buzon;
      while (!espera_retorno && (peticiones_pendientes > 0))
         Receptor_AtiendePendientes();
      if (Correos_Recoger(&que_buzon,&Origen,&peticion,&logitud) == 0)
      {
         Correos_BuzonEstablecer(que_buzon);
         Receptor_Despachar(que_buzon,Origen,peticion,logitud);
         Correos_BuzonEstablecer(-1);
      }
   }
   return 0;
}

/* ------------------------------------------------------------------
 *   Funcion: SRC_Loop.
 *   
 *   Estamos ante un servidor modelo, todo el tiempo lo dedica a 
 * servir las peticiones que nos lleguen en cualquier buzon. 
 * 
 * ------------------------------------------------------------------- */
int SRC_Loop(void)
{
  struct sockaddr_in Origen;
  char * peticion;
  int logitud;
  int que_buzon;

   while(1)
   {
      que_buzon = -1;
      while (!espera_retorno && (peticiones_pendientes > 0))
         Receptor_AtiendePendientes();
      if (Correos_Recoger(&que_buzon,&Origen,&peticion,&logitud) == 0)
      {
         Correos_BuzonEstablecer(que_buzon);
         Receptor_Despachar(que_buzon,Origen,peticion,logitud);
         Correos_BuzonEstablecer(-1);
      }
   }
   return 0;
}
 
/* ------------------------------------------------------------------
 *   Funcion: SRC_OpenLoop
 *   
 *   Esta funcion se ha de llamar siempre que se pueda, recoge lo 
 *  que haya pendiente en cualquier buzon.
 * ------------------------------------------------------------------- */
int SRC_OpenLoop(void)
{
  struct sockaddr_in Origen;
  char * peticion;
  int logitud;
  int que_buzon = -1;

  while (!espera_retorno && (peticiones_pendientes > 0))
    Receptor_AtiendePendientes();

  while (Correos_Hay(&Origen))
  {
     que_buzon = -1;
     if (Correos_Recoger(&que_buzon,&Origen,&peticion,&logitud) == 0)
     {
        Correos_BuzonEstablecer(que_buzon);
        Receptor_Despachar(que_buzon,Origen,peticion,logitud);
        Correos_BuzonEstablecer(-1);
     }
  }
  return 0;
}
 
/* ------------------------------------------------------------------
 *   Funcion: SRC_DescriptorOpenLoop
 *   
 *   Esta funcion se ha de llamar siempre que se pueda, recoge lo 
 *  que haya pendiente en un buzon.
 * ------------------------------------------------------------------- */
int SRC_DescriptorOpenLoop(int descriptor)
{
  struct sockaddr_in Origen;
  char * peticion;
  int logitud;
  int que_buzon;

  while (!espera_retorno && (peticiones_pendientes > 0))
    Receptor_AtiendePendientes();

  if ((que_buzon = Correos_BuzonDeDescriptor(descriptor)) < 0)
    return que_buzon;

  while (Correos_HayBuzon(que_buzon,&Origen))
  {
     if (Correos_Recoger(&que_buzon,&Origen,&peticion,&logitud) == 0)
     {
        Correos_BuzonEstablecer(que_buzon);
        Receptor_Despachar(que_buzon,Origen,peticion,logitud);
        Correos_BuzonEstablecer(-1);
     }
  }
  return 0;
}

/* ------------------------------------------------------------------
 *   Funcion: SRC_NextTimeout
 *   
 *   Indica el tiempo necesario para la siguiente temporizacion.
 * 
 * ------------------------------------------------------------------- */
void SRC_NextTimeout(struct timeval * timeout)
{
  Correos_Timeout(timeout);
}
 
/* ------------------------------------------------------------------
 *   Funcion: SRC_DescriptorEstablecer
 *   
 *   Establecer un buzon de salida para las peticiones. 
 * 
 * ------------------------------------------------------------------- */
int SRC_BuzonEstablecer(int buzon)
{
   return (Correos_BuzonEstablecer(buzon));
}

/* ------------------------------------------------------------------
 *   Funcion: SRC_AsignarBuzon
 *   
 *   Asignar como buzon un descriptor dado. 
 * 
 * ------------------------------------------------------------------- */
int SRC_AsignarBuzon(int buzon)
{
   return (Correos_BuzonAsignar(buzon));
}

