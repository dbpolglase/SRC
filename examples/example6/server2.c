#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <limits.h>
#include <poll.h>
#include <src.h>
#include <ex1.h>
#define SUCCESS "SUCCESS"
/* -------------------------------------------------------
 *   This is the more complex example
 *   
 *   The processes are simultaneously clients and servers.
 *
 *   One is server to EX1 class in 3490 port and the other 
 *   is server to EX2 class in 3491 port.
 *   When you enter something it call to EX1 or EX2 and
 *   as a consequence receive a request in response.
 *
 *   More they use a propietary poll not a thread or 
 *   similar, observe the use of SRC_NextTimeout
 *   and SRC_OpenLoop.
 *
 * ------------------------------------------------- */
struct sockaddr_in destino;
SRC_AddressType buzon={"127.0.0.1","3491",0};
 int cEX2_ArrayMetodoOut=0;
 int cEX1_ArrayMetodoIn=0;
 int cEX2_SimpleMetodoOut=0;
 int cEX1_SimpleMetodoIn=0;


int main()
{
 int fdserver;
 int pollResult;
 struct pollfd pollfds[2];
 struct timeval timeout;
 char data[50];

  if (GetNormalAddress(&destino,buzon) < 0)
  {
    printf("Error in GetNormalAddress \n");
    exit(1);
  }

  fdserver = SRC_Open("127.0.0.1","3490",0);

 /* ----
  * Create a pool for stdin a fddestino1
  * ------------ */

  pollfds[0].fd = 0;
  pollfds[0].events = POLLIN | POLLPRI;
  pollfds[1].fd = fdserver;
  pollfds[1].events = POLLIN | POLLPRI;

  while (1)
  {
    SRC_NextTimeout(&timeout);
    printf("Enter something to go or end to finish  \n");
    if (timeout.tv_sec ==0 && timeout.tv_usec == 0)
     pollResult = poll(pollfds, 2, -1);
    else
     pollResult = poll(pollfds, 2, (timeout.tv_sec * 1000 + timeout.tv_usec));

    if (pollResult < 0)
    {
      if (errno == EINTR  )
      {
        printf("Timeout \n");
        SRC_OpenLoop();
      }
      else
      {
       printf("Error at poll <%d>\n",errno);
       exit(1);
      }
    }
    if (pollfds[1].revents)
      SRC_OpenLoop();

    if (pollfds[0].revents)
    {
      scanf("%s",data);
      if (strcmp(data,"end") ==0) break;
      PutTimeoutService(1);
      printf("Calling EX1_SimpleMetodoIn expected a call to EX2_SimpleMetodoOut \n");
      EX1_SimpleMetodoIn(destino,'a',INT_MAX,LONG_MAX,429496.1,4294967295.3333);
    }
  }

  SRC_End();
  exit(0);

}
void EX2_SimpleMetodoOut(char a, int b, long c, float d, double e,int * result)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;

  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);
  cEX2_SimpleMetodoOut++;
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
 int i;
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;

  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);
  cEX2_ArrayMetodoOut++;
  printf(" EX2_ArrayMetodoOut from %s/%s \n",srcOrigen.host,srcOrigen.servicio);
  for (i=0;i!=10;i++)
   printf("%s-%c-%d-%ld-%f-%lf \n",a,b[i],c[i],d[i],e[i],f[i]);

  /*
   * Return SUCCESS
   * be careful, the return will be free by the library,
   * y for so we use strdup.
   */

  char ra[10] = {'1','2','3','4','5','6','7','8','9'};
  int rb[10] = {1,2,3,4,5,6,7,8,9};
  long rc[10] = {1,2,3,4,5,6,7,8,9};
  float rd[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
  double re[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
  printf("Call to EX1_ArrayMetodoIn\n");
  EX1_ArrayMetodoIn(destino,"HELLO",ra,rb,rc,rd,re);
  *resultado = strdup(SUCCESS);
}

void EX2_Count(char * metodo,int * resultado)
{
   if (strcmp(metodo,"EX2_ArrayMetodoOut") == 0)
    *resultado=cEX2_ArrayMetodoOut;
   if (strcmp(metodo,"EX1_ArrayMetodoIn") == 0)
    *resultado=cEX1_ArrayMetodoIn;
   if (strcmp(metodo,"EX2_SimpleMetodoOut") == 0)
    *resultado=cEX2_SimpleMetodoOut;
   if (strcmp(metodo,"EX1_SimpleMetodoIn") == 0)
    *resultado=cEX1_SimpleMetodoIn;
}

