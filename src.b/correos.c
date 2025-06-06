/* --------------------------------------------------------------------------
 *    MODULO: CORREOS.
 * 
 *   Este modulo realiza la tarea de correos, recibe cartas, envia cartas,
 * dice si hay cartas. 
 *   En definitiva se comporta como un sistema de correos normal, a veces
 * pierde la correspondencia.
 *
 * -------------------------------------------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <sys/types.h>
#ifdef sun
#include <sys/filio.h>
#endif
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <sys/time.h>
#include <textos.h>
#include <excepcion.h>
#include <src.h>
#include <direc.h>
#include <recep.h>
#include <codifica.h>
#include <l_correo.h>
#include <correos.h>
#include <tiempo.h>

int close(int);

/* Nro. de buzones de los que podemos recibir */
#define NRO_BUZONES  30

/* Direccion por defecto de multicast. */ 
#define DIREC_DEF_MULTICAST  "224"

/* Nro. de veces que reenviamos cuando hay error */
#define N_REINTENTOS_EMISION  0   

/* Tiempo de espera de confirmacion de paquete. */
#define TIEMPO_ESPERA_ACK 80; /* tiempo de espera de ack. 100 mili. */

/* Reconocimiento de mensaje (ACK). */ 
#define ACK  '#'  
#define RETURN_ACK  -2  

/* Tamano maximo de la carta que cabe en este buzon. */ 
#define MSG_UDP_BUFFER   512 

static char    * trozo_mensaje = NULL; /* Trozo de mensaje emitimos. */
static int espera_ack; /* A la espera de confirmacion de mensaje. */
static int espera_ack_timeout; /* Variable indicando si hay o no 
				      timeout en el ack. */

static int buzon_defecto = 0;  /* buzon por defecto a utilizar */
static int max_select_buzon; /* max descriptor a tener en cuenta. */

extern int espera_retorno; /* A la espera de retorno de servicio. */
extern int espera_retorno_timeout; /* Variable indicando si hay o no 
				      timeout en el servicio. */
int tamBufEnv = 5000;
int tamBufRec = 5000;

/* -------------------------------------------------------------
 *  Funciones estaticas.
 * -------------------------------------------------------------- */
static int Correos_DepositarTrozo(int que_buzon, struct sockaddr_in direcion, 
                                  char *buffer,int logitud);

static int Correos_RecogerTrozo(int que_buzon, char * buffer, int logitud,
                                struct sockaddr_in * remitente);

static int Correos_HayTrozo(int buzon, char * buffer, int logitud,
                            struct sockaddr_in * remite);

/* --------------------------------------------------------------------------
 *   El buzon donde vamos a recibir nuestras cartas y poner las que 
 * queremos enviar.
 * -------------------------------------------------------------------------- */
static int buzon[NRO_BUZONES];  
static int nro_buzones = 0;   /* Nro. de buzones que estoy usando. */

/* --------------------------------------------------------------------------
 *   Funcion: Crear.
 *
 *  Construye el buzon, es decir crea el buzon udp para recepcion de 
 *  mensajes.
 *
 *  Parametros 
 *     Entrada:
 *             Direccion, direccion que se le asigna a este buzon.
 *                        en esta implementacion coincidira con un 
 *                        nro. de servicio.
 *                        si no se indica direccion el sistema le
 *                        asignara una cualquiera pero si no se  
 *                        publica nadie podra hablar con nosotros.
 * 
 *      Retorna:
 *          0, todo perfecto.
 *          0<, no hemos podido crear el buzon, la direccion esta
 *             asignada, no podemos pagar la cuota de enganche..,.. 
 * ------------------------------------------------------------------------- */
int Correos_Crear(SRC_AddressType direccion)
{
  struct sockaddr_in   udp_srv_addr;
  struct servent * sp;
  struct ip_mreq mreq;
  int valor=1;
  int reuse=1;
  unsigned char loop = 0;
  int l_valor=0;


   if (nro_buzones < NRO_BUZONES)
   {
     /* ----------------------------------------------------------
      *    Creo un nuevo buzon / socket.
      * ------------------------------------------------------------ */
     memset((char *)&udp_srv_addr,0,sizeof(udp_srv_addr));
     buzon[nro_buzones] = socket(AF_INET,SOCK_DGRAM,0);
     if (buzon[nro_buzones] < 0)        /* Falla la creacion del socket ? */
     {
       Excepcion_Registra(ERR_CREAR_SOCKET);
       return -1;
     }
     if (buzon[nro_buzones] > max_select_buzon ) 
			max_select_buzon = buzon[nro_buzones];
   }
   else
    return -1;           /* No hay mas buzones posibles */

  /* ---------------------------------------------------------
   *    Le pongo la direccion a este buzon. 
   * --------------------------------------------------------- */
   udp_srv_addr.sin_family  = AF_INET;
   udp_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* ---------------------------------------------------------
   *    Servicio. 
   * --------------------------------------------------------- */
   if ((sp = getservbyname(direccion.servicio,"udp")) == NULL)
      udp_srv_addr.sin_port = htons(atoi(direccion.servicio));
   else
      udp_srv_addr.sin_port = sp->s_port;

  /* ---------------------------------------------------------
   *    Si utilizo multicast. 
   * --------------------------------------------------------- */
   if (direccion.multicast)
   {
     mreq.imr_multiaddr.s_addr = inet_addr(direccion.host);
     mreq.imr_interface.s_addr = INADDR_ANY;
     
     if (setsockopt(buzon[nro_buzones],IPPROTO_IP,IP_ADD_MEMBERSHIP,
                                      (const char *) &mreq,sizeof(mreq)) == 1)
     {
       Excepcion_Registra(ERR_DIR_MULTICAST);
       return -1;
     }
     /*
       Ya se lo que he mandado asi que no me interesa.
     */
     setsockopt(buzon[nro_buzones],IPPROTO_IP,IP_MULTICAST_LOOP,
				     (const char *)&loop,sizeof(loop));


   }
   l_valor=sizeof(valor);
   valor=0;
   setsockopt(buzon[nro_buzones],IPPROTO_TCP,TCP_NODELAY,&valor,l_valor);

   l_valor=sizeof(valor);
   valor=1;
   setsockopt(buzon[nro_buzones],SOL_SOCKET,SO_BROADCAST,&valor,l_valor);
  /*
     Aumento el tamanno del buffer. 
   */
  {
   setsockopt(buzon[nro_buzones],SOL_SOCKET,SO_SNDBUF,
				      (const char *)&tamBufEnv,sizeof(tamBufEnv));
   setsockopt(buzon[nro_buzones],SOL_SOCKET,SO_RCVBUF,
				      (const char *)&tamBufRec,sizeof(tamBufRec));
  }
  /*
    Permitir mas de un bind.
   */
   setsockopt(buzon[nro_buzones],SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT ,
				      (const char *)&reuse,sizeof(reuse));
   
  /* ---------------------------------------------------------
   *    Y lo doy de alta en la admnistracion de correos. 
   * --------------------------------------------------------- */
   if (bind(buzon[nro_buzones],(struct sockaddr *)&udp_srv_addr,
                                 sizeof(struct sockaddr_in)) == -1 )
   {
     Excepcion_Registra(ERR_BIND_SOCKET);
     return -1;
   }

   nro_buzones ++;
   return buzon[nro_buzones - 1];    /* perfecto. */
}

/* --------------------------------------------------------------------------
 *   Funcion: Destructor. 
 *
 *  Destruye el buzon, es decir elimina fisicamente el buzon y dalo 
 *  de baja de correos. 
 *
 * ------------------------------------------------------------------------- */
void Correos_Destruir(void)
{
 int i;
   /*  --------------------------------
    *   Cierro el socket. 
    * ----------------------------------- */
   for (i= 0; i!=nro_buzones; i++)
   if (buzon[i] != -1) close(buzon[i]);
   max_select_buzon = 0; 
    
   return;
}

/* --------------------------------------------------------------------------
 *   Funcion: DestructorBuzon. 
 *
 *  Destruye un buzon en particular. 
 *
 * ------------------------------------------------------------------------- */
void Correos_DestruirBuzon(int que_buzon)
{
 int i;

   max_select_buzon = 0;
   for (i= 0; i!=nro_buzones; i++)
   {
       if (buzon[i] == que_buzon) 
       {
	   close(buzon[i]);   /* Cierro el socket */
           buzon[i] =-1;
       }
       else
         if (buzon[i] > max_select_buzon) max_select_buzon  = buzon[i]; 
   }
    
   return;
}

/* --------------------------------------------------------------------------
 *   Funcion: SetTamBufferEnvio / SetTamBufferRecepcion.
 *
 *   Ajusta el buffer de envio o el de recepcion del socket.
 *   Tiene efecto sobre los siguientes Buzon creados.
 *
 *  Parametros 
 *     Entrada:
 *             tamano, nuevo tamaño del buffer.
 * ------------------------------------------------------------------------- */
void Correos_SetTamBufferEnvio(int tamano)
{
  tamBufEnv = tamano;
}
void Correos_SetTamBufferRecepcion(int tamano)
{
  tamBufRec = tamano;
}

/* --------------------------------------------------------------------------
 *   Funcion: RecogerTrozo.
 *
 *   Recoger un pedazo de la correspondencia que tengamos en el buzon. 
 *
 *  Parametros 
 *     Entrada:
 *             que_buzon,  de que buzon hay que coger. 
 *     Salida:
 *             buffer,    carta recogida.
 *             logitud,   tamano maximo de la carta que podemos coger.
 *             remitente, remitente de la carta.
 * 
 *      Retorna:
 *             0, completado.
 *             >0, nro. de caracteres leidos. 
 *             0<, error al ir a por cartas.
 * ------------------------------------------------------------------------- */
static int Correos_RecogerTrozo(int que_buzon, char * buffer, int logitud,
                                struct sockaddr_in * remitente)
{
   int leidos;
   int i;
   socklen_t l_direcion = sizeof(struct sockaddr_in);
   int r_select;
   struct timeval timeout;
   int max_select;
#ifdef SEGURO
   char buffer_ack[2]={ACK,'\0'};
#endif

#ifdef FD_SETSIZE
   fd_set mascara;
#else
   int mascara;
#endif

  /* ----------------------------------------------------------------
   *  Recoger del buzon cartas por un tamano inferior o igual al 
   * indicado.
   * ---------------------------------------------------------------- */
#ifdef FD_SETSIZE
   FD_ZERO(&mascara);
   /* -----------------------------------------------------
    * De que buzon queremos recoger.
    * ------------------------------------------------------- */
   if (que_buzon < 0)
   {
     max_select = max_select_buzon;
     for (i=0;i!= nro_buzones;i++)
      if (buzon[i] != -1) FD_SET(buzon[i],&mascara);
   }
   else
   {
        max_select = buzon[que_buzon];
        FD_SET(buzon[que_buzon],&mascara);
   }
#else
   /* -----------------------------------------------------
    * De que buzon queremos recoger.
    * ------------------------------------------------------- */
    if (que_buzon < 0)
    {
      max_select = max_select_buzon;
      for (i=0;i!= nro_buzones;i++)
       if (buzon[i] != -1) mascara = (1<<(buzon[i]));
    }
    else
    {
       max_select = buzon[que_buzon];
       mascara = (1<<que_buzon);
    }
#endif

   /* --------------------------------------
    *  Nos interesa el select para controlar el TIMEOUT, y solo nos  
    * interesa el TIMEOUT cuando estamos bloqueados a la espera del
    * retorno de una peticion. 
    * -------------------------------------------------------------------- */
   if (espera_retorno || espera_ack)
   {
     if (espera_ack)
     {
       timeout.tv_sec = 0;     
       timeout.tv_usec = TIEMPO_ESPERA_ACK;
     }
     else
     {
       timeout.tv_sec = Put_TimeoutServicioPendiente();     
       timeout.tv_usec = 0;
       if (timeout.tv_sec <= 0)
       {
         espera_retorno_timeout = 1;
         return 0;
       }
     }
     r_select = select(max_select + 1,&mascara,NULL,NULL,&timeout);
   }
   else
     r_select = select(max_select + 1,&mascara,NULL,NULL,NULL);

   switch(r_select)
   {
       case -1:
              Excepcion_Registra(ERR_CORREOS_SELECT);
              return -1;
       case 0:
              if (espera_ack)
                espera_ack_timeout = 1;
              else
                espera_retorno_timeout = 1;
              return 0;
    }


   for (i=0;i!=nro_buzones;i++)
   {
     if (FD_ISSET(buzon[i],&mascara))
     {
       leidos = recvfrom(buzon[i],buffer,logitud,
                              0,(struct sockaddr *)remitente, &l_direcion); 

      /* ---------------------------------------------------------------
       * Si algo a fallado al recoger el correo. 
       * --------------------------------------------------------------- */
      if (leidos < 0)
      {
        Excepcion_Registra(ERR_CORREOS_COGER);
        return leidos;
      }

     /* ---------------------------------------------
      * Si solo leemos un byte es un ACK. del origen.
      * ------------------------------------------------ */
#ifdef SEGURO
      if (leidos == 1) return RETURN_ACK;

     /* ------------------------------------------------------------
      * Enviamos el ACK del mensaje. 
      * ----------------------------------------------------------- */
      Correos_DepositarTrozo(buzon[i],*remitente,buffer_ack,1);
#endif

     /* ------------------------------------------------------------
      * Introducimos el trozo recibido en la lista de trozos. 
      * ----------------------------------------------------------- */
      if (LCorreos_NuevoTrozo(i,*remitente,buffer,leidos) < 0)
      {
        printf("Mensaje fuera de secuencia \n");
        return -1;
      }

      return leidos;
   }
  }
  return 0;
}

/* --------------------------------------------------------------------------
 *   Funcion: HayTrozo.
 *
 *   Indica si hay trozo de correo, mira la carta pero la vuelve a dejar en 
 *  el buzon. 
 *
 *  Parametros 
 *     Entrada:
 *             buzon,    buzon que hay que mirar. 
 *     Salida:
 *             buffer,    carta mirada.
 *             logitud,   tamano maximo de la carta que podemos mirar. 
 *             remite,    quien la manda.
 * 
 *      Retorna:
 *             0, >0 nro, de caracteres mirados.
 *             0<, error al ir a por cartas.
 * ------------------------------------------------------------------------- */
static int Correos_HayTrozo(int que_buzon, char * buffer, int logitud,
                            struct sockaddr_in * remite)
{
   int leidos;
   socklen_t l_direcion = sizeof(struct sockaddr_in);
   int bloqueo;

  /* ---------------------------------------------------------
   *   Mirar en el buzon el  que cartas hay y de que tamano
   *  y remite. 
   * ------------------------------------------------------------------ */
  
   bloqueo = 1;
   ioctl(que_buzon,FIONBIO,&bloqueo);
   leidos = recvfrom(que_buzon,buffer,logitud, MSG_PEEK,
		      (struct sockaddr *)&remite, &l_direcion); 

   bloqueo = 0;
   ioctl(que_buzon,FIONBIO,&bloqueo);

  /* -------------------------------------------------------
   *  Si no se ha podido mirar el buzon. 
   * -------------------------------------------------------------- */
   if (leidos < 0)
   {
     Excepcion_Registra(ERR_CORREOS_COGER);
     return leidos;
   }
  /* --------------------------------------------------------
   *   Habra que ver si ha sido por una interrupcion o un fallo.
   * ------------------------------------------------------------- */
   return leidos;
}

/* --------------------------------------------------------------------------
 *   Funcion: DepositarTrozo.
 *
 *   Madar un trozo de la carta a un conocido, indicar el tamano para 
 *  que el franqueo sea el adecuado. 
 *
 *  Parametros 
 *     Entrada:
 *             destino,   a quien va dirigida.
 *             buffer,    carta a enviar.
 *             logitud,   tamano de la misma. 
 * 
 *      Retorna:
 *             0, >0 nro, de caracteres enviados.
 *             0<, error al enviar las cartas. 
 * ------------------------------------------------------------------------- */
static int Correos_DepositarTrozo(int que_buzon, struct sockaddr_in direcion, 
                                  char *buffer,int logitud)
{
   int enviados;
#ifdef SEGURO
   char buffer_ack[4];
   struct sockaddr_in remite_ack;
#endif

  /* --------------------------------------------------------
   * Entregar la carta a la administracion de correos. 
   * ----------------------------------------------------------- */
   enviados = sendto(buzon[que_buzon],buffer,logitud,0,
                     (struct sockaddr *) &direcion, sizeof(direcion)); 

  /* ----------------------------------------------------------
   * Si no se ha podido enviar todo el mensaje error 
   * --------------------------------------------------------------- */
   if (enviados != logitud)
   {
     Excepcion_Registra(ERR_CORREOS_MANDAR);
     return enviados;
   }
   /* -------------------------
    * He enviado el ACK.
    * ------------------------------------------------- */
#ifdef SEGURO
   if (enviados == 1)
    return 0;

  /* ----------------------------------------------------------
   * Esperar el ACK. 
   * --------------------------------------------------------------- */
   espera_ack_timeout = 0;
   espera_ack = 1;
   while (espera_ack_timeout == 0 && 
     Correos_RecogerTrozo(que_buzon,buffer_ack,3,&remite_ack) != RETURN_ACK);

   espera_ack = 0;
   if (espera_ack_timeout == 1)
   {
     Excepcion_Registra(ERR_ACK_TIMEOUT);
     return -1;
   }

#endif

   return 0;
}


/* ----------------------------------------------------------------- 
 *   Funcion: Depositar.
 * 
 *   Deposita una carta en el sistema de comunicaciones. 
 *  Para evitar problemas de franqueo hay que divirla en trozitos. 
 *  Estos trocitos se identifican por numero de mensaje / nro. de 
 * trozo.  
 *
 *   Parametros 
 *     Entrada:
 *         destino, emisor hacia.
 * 	   mensaje, mensaje a enviar.
 * 	   logitud, logitud del mensaje. 
 *     Retorna:
 *         0, ok.
 *         0<, error.
 * ------------------------------------------------------------------- */
int Correos_Depositar(int que_buzon,struct sockaddr_in destino, 
                      char * buffer, int logitud)
{
  static long identificativo = 0;
  long   enviados = 0;
  long   numero_trozo = 1;
  int    l_trozo_mensaje;
  int    i,resultado;

  /* -----------------------------------------------
   *  Si tengo como buzon un negativo. Utilizo el primer buzon.
   * ---------------------------------------------------------------- */
   if (que_buzon < 0)
      que_buzon = buzon_defecto; 

  /* -----------------------------------------------
   *  Identificativo del mensaje para su recepcion.
   * ------------------------------------------------ */
  identificativo ++;

  /* -----------------------------------------------
   *  Para que no se queje la SUN no utilizamos 
   * un buffer estatico. 
   * ------------------------------------------------ */
  if (trozo_mensaje == NULL) trozo_mensaje = malloc(MSG_UDP_BUFFER + 10);

  while ((logitud - enviados) >= (MSG_UDP_BUFFER - CAB_IDENTIFICATIVO))
  {
      if ((numero_trozo % 100) == 0)
        sleep(1);

     /* ---------------------------------------------------
      * Codificamos el mensaje y su cabecera.
      * ---------------------------------------------------- */ 
      Codifica_Mensaje(identificativo,numero_trozo,buffer+enviados,
		       (MSG_UDP_BUFFER - CAB_IDENTIFICATIVO),&trozo_mensaje);

       enviados += (MSG_UDP_BUFFER - CAB_IDENTIFICATIVO);
       l_trozo_mensaje = (int)MSG_UDP_BUFFER;

     /* ---------------------------------------------------
      * y lo enviamos, o lo intentamos durante un nro. de veces. 
      * ----------------------------------------------------------- */ 
      for (i= 0;i <= N_REINTENTOS_EMISION; i++)
      {
        if ((resultado = Correos_DepositarTrozo(que_buzon,destino,
                                                trozo_mensaje,
					        l_trozo_mensaje)) == 0)
         break; 
      }
     if (resultado < 0)
      return -1;

     numero_trozo ++; 
   }

  /* -----------------
   * resto de mensaje.
   * --------------------------- */
   Codifica_Mensaje(identificativo,numero_trozo,buffer+enviados,
		       (logitud - enviados),&trozo_mensaje);

  l_trozo_mensaje = logitud - enviados + CAB_IDENTIFICATIVO; 
  /* ---------------------------------------------------
   * y lo enviamos, o al menos lo intentamos un nro. de veces. 
   * ----------------------------------------------------------- */ 
  for (i= 0;i <= N_REINTENTOS_EMISION; i++)
  {
     resultado = Correos_DepositarTrozo(que_buzon,destino,trozo_mensaje,
                                        l_trozo_mensaje);
     if (resultado == 0)
       break;
  }
  if (resultado < 0)
   return -1;

  return 0;
}

/* ----------------------------------------------------------------- 
 *   Funcion: Recoger.
 * 
 *   Recoge del buzon los trozos de carta que hubiera y si con lo 
 *  recibido se recompone un mensaje, retorna este mensaje al nivel
 *  superior. 
 *
 *   Parametros 
 *     Salida:
 *         remite, emisor hacia.
 * 	   mensaje, mensaje recibido. 
 * 	   logitud, logitud del mensaje. 
 *     Entrada/Salida:
 *         buzon, buzon del que se recogen y de donde se obtiene.
 *     Retorna:
 *         0, completado.
 *         >0, recibidos. 
 *         0<, error.
 * ------------------------------------------------------------------- */
int Correos_Recoger(int * que_buzon, struct sockaddr_in * remite, 
                    char ** buffer, int * logitud)
{
  char t_buffer[MSG_UDP_BUFFER];
  int  resultado;

  /* ------------------------------------------------------------
   *   Si no nos dan buzon, por el de defecto el 0. 
   * ----------------------------------------------------------- */
/*
   if (*que_buzon < 0) *que_buzon = buzon[0];
*/

  /* ------------------------------------------------------------
   *   Recogemos un trozo.
   * ----------------------------------------------------------- */
   resultado = Correos_RecogerTrozo(*que_buzon,t_buffer, MSG_UDP_BUFFER,remite);

   if (resultado > 0 && resultado != MSG_UDP_BUFFER)
   {
      /* -------------------------------------------------
       *  Completado, obtener el mensaje.
       * -------------------------------------------------- */
      if (LCorreos_Obtener(que_buzon,remite,buffer,logitud) < 0) 
        return -1;

      return 0;
   }

   return resultado;
}


/* ----------------------------------------------------------------- 
 *   Funcion: Hay.
 * 
 *   Indica hacia arriba si hay algo en espera de ser procesado en 
 *  el buzon y de donde es. 
 *
 *   Parametros 
 *     Salida:
 *         remite, origen del mensaje en espera.
 *     Retorna:
 *         1, Hay.
 *         0, no hay.
 *         0<, error.
 * ------------------------------------------------------------------- */
int Correos_Hay(struct sockaddr_in * remite)
{
  char t_buffer[MSG_UDP_BUFFER];
  int i;

  /* ------------------------------------------------------------
   *   Miramos a ver si hay algun trozo en espera. 
   * ----------------------------------------------------------- */
   for (i=0;i!=nro_buzones;i++)
    if (Correos_HayTrozo(buzon[i],t_buffer, MSG_UDP_BUFFER, remite) > 0 )
     return 1;

   return 0;
}

/* ----------------------------------------------------------------- 
 *   Funcion: HayBuzon.
 * 
 *   Indica hacia arriba si hay algo en espera de ser procesado en 
 *  un determinado buzon. 
 *
 *   Parametros 
 *     Entrada:
 *         buzon,  buzon en el que hay que mirar. 
 *     Salida:
 *         remite, origen del mensaje en espera.
 *     Retorna:
 *         1, Hay.
 *         0, no hay.
 *         0<, error.
 * ------------------------------------------------------------------- */
int Correos_HayBuzon(int que_buzon, struct sockaddr_in * remite)
{
  char t_buffer[MSG_UDP_BUFFER];

  /* ------------------------------------------------------------
   *   Miramos a ver si hay algun trozo en espera. 
   * ----------------------------------------------------------- */
   if (Correos_HayTrozo(que_buzon,t_buffer, MSG_UDP_BUFFER, remite) > 0 )
    return 1;

   return 0;
}


/* ----------------------------------------------------------------- 
 *   Funcion: BuzonAsignar.
 * 
 *   Asignar la variable buzon. Cuando ponemos un invento en el inetd.conf 
 *  el socket se nos pasa como parametro esta funcion nos permite asignarlo. 
 *
 *   Parametros 
 *     Entrada:
 *         buzon, buzon a utilizar. 
 * ------------------------------------------------------------------- */
int Correos_BuzonAsignar(int n_buzon)
{
  if (nro_buzones < NRO_BUZONES)
  {
    buzon[nro_buzones] = n_buzon;
    nro_buzones ++;
    return 1;
  }
  else 
   return -1;
}


/* ----------------------------------------------------------------- 
 *   Funcion: Correos_BuzonEstablecer.
 * 
 *   Establece el buzon de salida para una peticion o respuesta. 
 *
 *   Parametros 
 *     Entrada:
 *         buzon, buzon a utilizar. 
 * ------------------------------------------------------------------- */
int Correos_BuzonEstablecer(int que_buzon)
{
 int i;

  if (que_buzon < 0)
  {
   buzon_defecto = 0;
   return 0;
  }

  for (i=0;i!=nro_buzones;i++)
  {
    if (buzon[i] == que_buzon)
    {
     buzon_defecto =  i;
     return 0;
    }
  }
 
  return -1;


}

/* ----------------------------------------------------------------- 
 *   Funcion: BuzonDeDescriptor.
 * 
 *   Indica que buzon ha de usarse para el descriptor indicado. 
 *
 *   Parametros 
 *     Entrada:
 *         descriptor, descriptor a buscar. 
 *     Salida:
 *         buzon, buzon correspondiente. 
 * ------------------------------------------------------------------- */
int Correos_BuzonDeDescriptor(int descriptor)
{
 int i;

   for (i=0;i!=nro_buzones;i++)
    if (buzon[i] == descriptor)
     return i;

   return -1;
}

/* ----------------------------------------------------------------- 
 *   Funcion: Timeout.
 * 
 *   Calcula el siguinte timeout para el select.
 *
 *   Parametros 
 *     Salida:
 *         timeout o null sino existe.
 * ------------------------------------------------------------------- */
void Correos_Timeout(struct timeval * timeout)
{
   /* --------------------------------------
    *  Nos interesa el select para controlar el TIMEOUT, y solo nos  
    * interesa el TIMEOUT cuando estamos bloqueados a la espera del
    * retorno de una peticion. 
    * -------------------------------------------------------------------- */
   if (espera_retorno || espera_ack)
   {
     if (espera_ack)
     {
       (*timeout).tv_sec = 0;     
       (*timeout).tv_usec = TIEMPO_ESPERA_ACK;
     }
     else
     {
       (*timeout).tv_sec = Put_TimeoutServicioPendiente();     
       timeout->tv_usec = 0;
       if ((*timeout).tv_sec <= 0)
       {
         espera_retorno_timeout = 1;
         (*timeout).tv_sec = 0;     
       }
     }
   }
   else
   {
    (*timeout).tv_usec=0; (*timeout).tv_sec=0;
   }
   
}

