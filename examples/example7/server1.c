#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <src.h>
#include <ex2.h>

#define SUCCESS "SUCCESS"

void *runServer( void *ptr );
 int cEX1_ArrayMetodoIn=0;
 int cEX1_SimpleMetodoIn=0;

/* -------------------------------------------------------
 *   This is the fail example
 *
 *   The processes are simultaneously clients and servers.
 *
 *   One is server to EX1 class in 3490 port and the other
 *   is server to EX2 class in 3491 port.
 *   When you enter something it call to EX1 or EX2 and
 *   as a consequence receive a request in response.
 *
 *   But we are one thread as server and the main thread
 *   to make request.
 *
 *   You can observe that this example fail in the case of 
 *   use complex two-way methods because of memory issues so 
 *   if you need this architecture use one-way methods.
 *   
 * ------------------------------------------------- */
struct sockaddr_in destino;


int main()
{
 pthread_t thread1;
 int  iret1;
 struct pollfd pollfds[3];
 SRC_AddressType buzon={"127.0.0.1","3490",0};
 char data[30];

 char ra[10] = {'1','2','3','4','5','6','7','8','9'};
 int rb[10] = {1,2,3,4,5,6,7,8,9};
 long rc[10] = {1,2,3,4,5,6,7,8,9};
 float rd[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
 double re[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
 char * sresult;
 if (GetNormalAddress(&destino,buzon) < 0)
 {
    printf("Error in GetNormalAddress \n");
    exit(1);
 }


 iret1 = pthread_create( &thread1, NULL, runServer, (void*) "Server1");
 printf("Server1 returns: %d\n",iret1);

 pollfds[0].fd = 0;
 pollfds[0].events = POLLIN | POLLPRI;

 while (1)
 {
   printf("Enter something to go or end to finish \n"); 
   poll(pollfds, 1,-1);
   scanf("%s",data);
   if (strcmp(data,"end")==0) break;

   /* 
    * call to EX2_ArrayMetodoOut
    */
   printf("Call to EX2_ArrayMetodoOut expected a call to EX1_arrayMetodoIn\n");
   EX2_ArrayMetodoOut(destino,"HELLO",ra,rb,rc,rd,re,&sresult);
   printf("result EX2_ArrayMetodoOut: <%s>\n",sresult);
 }
 pthread_kill(thread1, 15);
 pthread_join( thread1, NULL);

 exit(0);
 return 0;
}

void *runServer( void *ptr )
{
     printf("Running servers over 127.0.0.1/3491.  \n");
     SRC_Open("127.0.0.1","3491",0);
     SRC_Loop();
     SRC_End();

  return NULL;
}

void EX1_SimpleMetodoIn(char a, int b, long c, float d, double e)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;
 int result;

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

  /* 
   * call to EX2_simpleMetodoIn 
   */
  EX2_SimpleMetodoOut(destino,'a',INT_MAX,LONG_MAX,429496.1,4294967295.3333,&result);
/*  printf("result EX2_SimpleMetodoOut: <%d>\n",result);*/

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
   * Get origing
   */
  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);

  printf(" EX1_ArrayMetodoIn, receive from: %s/%s \n",srcOrigen.host,srcOrigen.servicio);
  for (i=0;i!=10;i++)
   printf(" %s-%c-%d-%ld-%f-%lf \n",a,b[i],c[i],d[i],e[i],f[i]);
}

void EX1_Count(char * metodo,int * resultado)
{
   if (strcmp(metodo,"EX1_ArrayMetodoIn") == 0)
    *resultado=cEX1_ArrayMetodoIn;
   if (strcmp(metodo,"EX1_SimpleMetodoIn") == 0)
    *resultado=cEX1_SimpleMetodoIn;
}

