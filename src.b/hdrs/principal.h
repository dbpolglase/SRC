#ifndef __SRC
#define __SRC
/* ---------------------------------------------------------------------
 *  MODULO: main.
 * 
 *  Modulo principal de control de la aplicacion.
 * --------------------------------------------------------------------- */

int  SRC_Inicio(char * host,char * servicio,int multicast);
int  SRC_Fin(void);
int  SRC_FinDescriptor(int descriptor);
int  SRC_Continuo(void);
int  SRC_Discontinuo(void);
int  SRC_ContinuoDescriptor(int descriptor);
int  SRC_DiscontinuoDescriptor(int descriptor);
int  SRC_BuzonEstablecer(int buzon);
int  SRC_AsignarBuzon(int buzon);
struct timeval  SRC_SiguienteTemporizacion(void);
#endif /* __SRC */
