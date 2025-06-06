#ifndef __CORREOS
#define __CORREOS
/* ======================================================================== 
 * 
 *    MODULO: CORREOS
 * 
 *    Interfaz del modulo que hace de intermediario con la administracion
 *  de correos, es decir maneja el buzon.
 *
 *     Este buzon se compone con un puerto "udp" y este modulo permite
 *    Crearlo, Destruirlo, Enviar a traves de el, Recoger de el y  
 *    consultar si hay algo en el. 
 * 
 * ======================================================================= */
int Correos_Crear(SRC_AddressType direccion);    /* Crear el buzon. */

void Correos_Destruir(void);                  /* Abajo el telon. */

void Correos_DestruirBuzon(int buzon);        /* Abajo un telon. */

void Correos_SetTamBufferEnvio(int tamBuffer); /* Ajustar Tamanno buffer envio */
void Correos_SetTamBufferRecepcion(int tamBuffer); /* Ajustar Tamanno buffer recepcion */

int Correos_Depositar(int que_buzon, 
                      struct sockaddr_in destino,
		      char * carta,
		      int logitud);         /* Mandar una carta */

int Correos_Recoger(int * que_buzon, struct sockaddr_in * remite,
		    char ** carta,
		    int * logitud);           /* Recoger una carta */

int Correos_Hay(struct sockaddr_in * origen);

int Correos_HayBuzon(int que_buzon, struct sockaddr_in * origen);

int Correos_BuzonAsignar(int n_buzon);

int Correos_BuzonEstablecer(int buzon);

int Correos_BuzonDeDescriptor(int descriptor);
void Correos_Timeout(struct timeval * timeout);

#endif /* CORREOS */
