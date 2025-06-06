#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <limits.h>
#include <src.h>
#include <example3.h>

/* -------------------------------------------------------
 *   Use of poll with SRC.
 *   Server locate in 127.0.0.1/3491.
 *   Server alive until you press enter.
 * ------------------------------------------------- */
int main()
{
 int result;
 char *sresult;
 int i,j;
 SRC_AddressType buzon2={"127.0.0.1","3491",0};

struct sockaddr_in destino1,destino2;
destino1.sin_family = AF_INET;
destino1.sin_port = htons(3490);
inet_aton("127.0.0.1", &destino1.sin_addr);

  if (GetNormalAddress(&destino2,buzon2) < 0)
  {
    printf("Error in Direccion_Fisica \n");
    exit(1);
  }

  char a[10] = {'1','2','3','4','5','6','7','8','9'};
  int b[10] = {1,2,3,4,5,6,7,8,9};
  long c[10] = {1,2,3,4,5,6,7,8,9};
  float d[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
  double e[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};

  SRC_Open(NULL,"",0);

  printf("Write number of calls \n");
  scanf("%d",&j);
for (i=0;i!=j;i++)
{
  printf("Calling EX1_SimpleMetodoIn and EX1_ArrayMetodoIn \n");
  printf("not return expected \n");
  EX1_SimpleMetodoIn(destino1,'a',INT_MAX,LONG_MAX,429496.1,4294967295.3333);
  EX1_ArrayMetodoIn(destino1,"HELLO",a,b,c,d,e);

  printf("Calling EX2_SimpleMetodoOut and EX2_ArrayMetodoOut \n");
  printf("wait for return 1sg. \n");
  PutTimeoutService(1);
  EX2_SimpleMetodoOut(destino2,'a',INT_MAX,LONG_MAX,429496.1,4294967295.3333,&result);
  printf("result EX2_SimpleMetodoOut: <%d>\n",result);
  EX2_ArrayMetodoOut(destino2,"HELLO",a,b,c,d,e,&sresult);
  printf("result EX2_ArrayMetodoOut: <%s>\n",sresult);
}
  EX3_Count(destino1,"EX1_SimpleMetodoIn",&result);
  printf("Server said total call to EX1_SimpleMetodoIn %d\n",result);
  EX3_Count(destino1,"EX1_ArrayMetodoIn",&result);
  printf("Server said total call to EX1_ArrayMetodoIn %d\n",result);
  EX3_Count(destino1,"EX2_SimpleMetodoOut",&result);
  printf("Server said total call to EX2_SimpleMetodoOut %d\n",result);
  EX3_Count(destino1,"EX2_ArrayMetodoOut",&result);
  printf("Server said total call to EX2_ArrayMetodoOut %d\n",result);
  SRC_End();

  return 0;
}
