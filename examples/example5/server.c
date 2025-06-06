#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <poll.h>
#include <src.h>

#define SUCCESS "SUCCESS"

 int cEX2_ArrayMetodoOut=0;
 int cEX1_ArrayMetodoIn=0;
 int cEX2_SimpleMetodoOut=0;
 int cEX1_SimpleMetodoIn=0;

/* ================================================
 *   Now with use broadcast IP for the server
 * ============================================== */
int main()
{

 /*
  *  Create socket broadcast in port 3490/3491 for 
  *  the server.
  */
  SRC_Open("255.255.255.255","3490",0);
  SRC_Open("255.255.255.255","3491",0);

  SRC_Loop();
  SRC_End();
  return 0;
}

void EX1_SimpleMetodoIn(char a, int b, long c, float d, double e)
{
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;

 cEX1_SimpleMetodoIn++;
  /* 
   * call to EX1_simpleMetodoIn 
   */
  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);
  printf(" EX1_SimpleMetodoIn,receive from %s/%s , data:%c - %d - %ld - %f -  %lf\n",srcOrigen.host,srcOrigen.servicio,a,b,c,d,e);
}

void EX1_ArrayMetodoIn(char *a, char * b,int * c, long * d, float * e, double * f)
{
  /* 
   * call to EX1_ArrayMetodoIn 
   */
 int i;
 struct sockaddr_in origen;
 SRC_AddressType srcOrigen;

  GetRequestAddress(&origen);
  GetLogicAddress(origen,&srcOrigen);
  cEX1_ArrayMetodoIn++;
  printf(" EX1_ArrayMetodoIn, receive from: %s/%s",srcOrigen.host,srcOrigen.servicio);
  for (i=0;i!=10;i++)
   printf(" %s-%c-%d-%ld-%f-%lf \n",a,b[i],c[i],d[i],e[i],f[i]);
   
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

