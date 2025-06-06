/* -----------------------------------------------------------
 *   Modulo de control de errores.
 * ----------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <excepcion.h>
#include <textos.h>
#include <error.h>

static int l_error = 0;
void (*funcion_manejo)(int,int,char*) = NULL;

void Excepcion_Registra(int error)
{
 int i;

 for (i=1;i!= MAX_LISTA;i++)
 {
   if (ListaERROR[i].error == error)
   {
     l_error = i;
     if (funcion_manejo != NULL)
      funcion_manejo(ListaERROR[i].error,ListaERROR[i].nivel,
                                  ListaERROR[i].descripcion);
     return;
   }
 }
 if (funcion_manejo != NULL)
    funcion_manejo(ListaERROR[0].error,ListaERROR[0].nivel,
                                  ListaERROR[0].descripcion);
}

void Excepcion_Consulta(int * error, int * nivel, char ** texto)
{
   *error = ListaERROR[l_error].error;
   *nivel = ListaERROR[l_error].nivel;
   *texto = ListaERROR[l_error].descripcion;
}

void Excepcion_Escribe(void)
{
   fprintf(stderr," EXCEPCION \n");
   fprintf(stderr,"ERROR: %d \n",ListaERROR[l_error].error);
   fprintf(stderr,"NIVEL: %d \n",ListaERROR[l_error].nivel);
   fprintf(stderr,"TEXTO: %s \n",ListaERROR[l_error].descripcion);
}

void Excepcion_Limpia(void)
{
  l_error = 0;
}

void Excepcion_Manejador(void(*f_excepcion)(int,int,char*))
{
  funcion_manejo = f_excepcion;
}

void Excepcion_RegistraServicio(char * error)
{
 int i;

 for (i=1;i!= MAX_LISTA;i++)
 {
   if (strcmp(ListaERROR[i].descripcion,error) == 0)
   {
     l_error = i;
     if (funcion_manejo != NULL)
      funcion_manejo(ListaERROR[i].error,ListaERROR[i].nivel,
                                    ListaERROR[i].descripcion);
   }
  }
  if (funcion_manejo != NULL)
     funcion_manejo(ListaERROR[0].error,ListaERROR[0].nivel,
                                    ListaERROR[0].descripcion);
}

