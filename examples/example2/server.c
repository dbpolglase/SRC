#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <src.h>

#define SUCCESS "SUCCESS"

 int cEX2_ArrayMetodoOut=0;
 int cEX1_ArrayMetodoIn=0;
 int cEX2_SimpleMetodoOut=0;
 int cEX1_SimpleMetodoIn=0;

/* ----------------------------------------------------
 *  Repeat the example1 but using two server port for listen
 *  
 *  Server1: 127.0.0.1 / 3490
 *  Server2: 127.0.0.1 / 3491
 * 
 * ----------------------------------------------------------- */

int main()
{
 /*
  *  Let to listen  on local ip (127.0.0.1), port 3490 udp
  *  and is not multicast.
  */
  SRC_Open("127.0.0.1","3490",0);
  SRC_Open("127.0.0.1","3491",0);
  /*
  * Receive request for ever.
  */
  SRC_Loop();
  /*
  * End server
  */
  SRC_End();
  return 0;
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
   if (strcmp(metodo,"EX2_ArrayMetodoOut") == 0)
    *resultado=cEX2_ArrayMetodoOut;
   if (strcmp(metodo,"EX1_ArrayMetodoIn") == 0)
    *resultado=cEX1_ArrayMetodoIn;
   if (strcmp(metodo,"EX2_SimpleMetodoOut") == 0)
    *resultado=cEX2_SimpleMetodoOut;
   if (strcmp(metodo,"EX1_SimpleMetodoIn") == 0)
    *resultado=cEX1_SimpleMetodoIn;
}

