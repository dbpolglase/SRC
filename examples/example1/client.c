#include <stdio.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <limits.h>
#include <src.h>
#include <example1.h>

/* ----------------------------------
 *   Make the calls to SERVER 
 *   Build the server address by hand.
 * --------------------------------------- */
int main()
{
 int result;
 char *sresult;
 int i,j;
struct sockaddr_in destino;
destino.sin_family = AF_INET;
destino.sin_port = htons(3490);
inet_aton("127.0.0.1", &destino.sin_addr);

  char a[10] = {'1','2','3','4','5','6','7','8','9'};
  int b[10] = {1,2,3,4,5,6,7,8,9};
  long c[10] = {1,2,3,4,5,6,7,8,9};
  float d[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
  double e[10] = {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};

  SRC_Open(NULL,"",0); /* Create the mailbox for the answers */

  printf("Write number of calls \n");
  scanf("%d",&j);

for (i=0;i!=j;i++)
{
  printf("Calling EX1_SimpleMetodoIn and EX1_ArrayMetodoIn \n");
  printf("not return expected \n");
  EX1_SimpleMetodoIn(destino,'a',INT_MAX,LONG_MAX,429496.1,4294967295.3333);
  EX1_ArrayMetodoIn(destino,"HELLO",a,b,c,d,e);

  printf("Calling EX2_SimpleMetodoOut and EX2_ArrayMetodoOut \n");
  printf("set timeout for return 1 sg.\n");
  PutTimeoutService(1); /* Put the timeout for services in 1 sg. */
  EX2_SimpleMetodoOut(destino,'a',INT_MAX,LONG_MAX,429496.1,4294967295.3333,&result);
  printf("result EX2_SimpleMetodoOut: <%d>\n",result);
  EX2_ArrayMetodoOut(destino,"HELLO",a,b,c,d,e,&sresult);
  printf("result EX2_ArrayMetodoOut: <%s>\n",sresult);
}
  EX3_Count(destino,"EX1_SimpleMetodoIn",&result);
  printf("Server said total call to EX1_SimpleMetodoIn %d\n",result);
  EX3_Count(destino,"EX1_ArrayMetodoIn",&result);
  printf("Server said total call to EX1_ArrayMetodoIn %d\n",result);
  EX3_Count(destino,"EX2_SimpleMetodoOut",&result);
  printf("Server said total call to EX2_SimpleMetodoOut %d\n",result);
  EX3_Count(destino,"EX2_ArrayMetodoOut",&result);
  printf("Server said total call to EX2_ArrayMetodoOut %d\n",result);

  printf(" ====================================================== \n");
  printf(" Now you can play with several servers/clients          \n");
  printf(" And Let see what happen                                \n");
  printf(" ====================================================== \n");
  SRC_End();
  return 0;
}
