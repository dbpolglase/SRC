#ifndef __SRC
#define __SRC
/* -----------------------------------------------------------
 *
 *  MODULO: SRC
 *
 *  Descripcion de estructuras y datos comunes a todos los elementos. 
 * 
 * ------------------------------------------------------------- */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>


#define MAX_CLASE         30
#define MAX_METODO        30

#define MAX_LONG_HOST       30
#define MAX_LONG_SERVICIO   30

typedef struct {
		 char host[MAX_LONG_HOST];
		 char servicio[MAX_LONG_SERVICIO];
		 int  multicast;
                }SRC_AddressType;

/* 
 * Create a server mailbox.
 */
int  SRC_Open(char * host,char * servicio,int multicast);
/* 
 * End SRC server.
 */
int  SRC_End(void);
/* 
 * End SRC mailbox server
 */
int  SRC_FinDescriptor(int descriptor);
/* 
 * RESPONDS TO REQUESTS for EVER.
 */
int  SRC_Loop(void);
/* 
 * RESPONDS PENDING REQUESTS. 
 */
int  SRC_OpenLoop(void);
/* 
 * RESPONDS REQUESTS of SERVER MAILBOX for EVER
 */
int  SRC_DescriptorLoop(int descriptor);
/* 
 * RESPONDS PENDING REQUESTS of SERVER MAILBOX 
 */
int  SRC_DescriptorOpenLoop(int descriptor);
void SRC_NextTimeout(struct timeval * );

#include <direc.h>
#include <recep.h>
#include <emisor.h>
#include <tiempo.h>
#endif /* __SRC */
