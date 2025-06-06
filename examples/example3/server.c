#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <src.h>

#define SUCCESS "SUCCESS"

void *runServer( void *ptr );

 int cEX2_ArrayMetodoOut=0;
 int cEX2_SimpleMetodoOut=0;
 int cEX1_ArrayMetodoIn=0;
 int cEX1_SimpleMetodoIn=0;

/* -------------------------------------------------------
 * The same example that before but use a thread for the 
 * server.
 *  
 *  Be careful, you can run several threads that use SRC
 *  it is not thread safe.
 *   
 *  Main thread use a poll over standar input for exit   
 *  
 * --------------------------------------------------------- */
int main()
{
 pthread_t thread1;
 int  iret1;
 struct pollfd pollfds[3];


 iret1 = pthread_create( &thread1, NULL, runServer, (void*) "Server1");
 printf("Server1 returns: %d\n",iret1);

 pollfds[0].fd = 0;
 pollfds[0].events = POLLIN | POLLPRI;

 poll(pollfds, 1,-1);

 pthread_kill(thread1, 15);
 pthread_join( thread1, NULL);

 exit(0);
 return 0;
}

void *runServer( void *ptr )
{
     printf("Running servers over 127.0.0.1/3490-3491.  \n");
     SRC_Open("127.0.0.1","3490",0);
     SRC_Open("127.0.0.1","3491",0);
     SRC_Loop();
     SRC_End();

  return NULL;
}

void EX1_SimpleMetodoIn(char a, int b, long c, float d, double e)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;

 cEX1_SimpleMetodoIn++;
  /*
   * Get origin
   */
  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);

  /* 
   * call to EX1_simpleMetodoIn 
   */
  printf(" EX1_SimpleMetodoIn,receive from %s/%s , data:%c - %d - %ld - %f -  %lf\n",srcOrigen.host,srcOrigen.servicio,a,b,c,d,e);

}

void EX1_ArrayMetodoIn(char *a, char * b,int * c, long * d, float * e, double * f)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;
 int i;

  /*
   * call to EX1_ArrayMetodoIn
   */
 cEX1_ArrayMetodoIn++;
  /*
   * Get origin
   */
  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);

  printf(" EX1_ArrayMetodoIn, receive from: %s/%s \n",srcOrigen.host,srcOrigen.servicio);
  for (i=0;i!=10;i++)
   printf(" %s-%c-%d-%ld-%f-%lf \n",a,b[i],c[i],d[i],e[i],f[i]);
}

void EX2_SimpleMetodoOut(char a, int b, long c, float d, double e,int * result)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;

 cEX2_SimpleMetodoOut++;
  /*
   * Get origin
   */
  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);

  /*
   * call to EX2_SimpleMetodoOut
   */
  printf(" EX2_SimpleMetodoOut from %s/%s, data:%c-%d-%ld-%f-%lf\n",srcOrigen.host,srcOrigen.servicio,a,b,c,d,e);
  /*
   * return 3.
   */
  *result= 3;

}

void EX2_ArrayMetodoOut(char *a, char * b,int * c, long * d, float * e, double * f,char ** resultado)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;
 int i;

  cEX2_ArrayMetodoOut++;
  /*
   * Get origin
   */
  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);

  printf(" EX2_ArrayMetodoOut from %s/%s \n",srcOrigen.host,srcOrigen.servicio);
  for (i=0;i!=10;i++)
   printf("%s-%c-%d-%ld-%f-%lf \n",a,b[i],c[i],d[i],e[i],f[i]);

  /*
   * Return SUCCESS
   * be careful, the return will be free by the library,
   * y for so we use strdup.
   */
  *resultado = strdup(SUCCESS);
}

void EX3_Count(char * metodo,int * resultado)
{
   if (strcmp(metodo,"EX1_ArrayMetodoIn") == 0)
    *resultado=cEX1_ArrayMetodoIn;
   if (strcmp(metodo,"EX1_SimpleMetodoIn") == 0)
    *resultado=cEX1_SimpleMetodoIn;
   if (strcmp(metodo,"EX2_ArrayMetodoOut") == 0)
    *resultado=cEX2_ArrayMetodoOut;
   if (strcmp(metodo,"EX2_SimpleMetodoOut") == 0)
    *resultado=cEX2_SimpleMetodoOut;
}

