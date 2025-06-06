/* -----------------------------------------------------------------
 *
 *   Modulo para la generacion de el sistema de paso de mensajes 
 * basado en descripcion.
 *
 * ---------------------------------------------------------------- */
/* -------------------------------
 *  Modulo de definiciones. 
 * ------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src_stri.h"
#include "src_gen.h"
#include "scaner.h"
#include "parser.h"
#include "codi.h"

/* -------------------------------
 *  Comprueba que los parametros pasados son los adecuados. 
 * --------------------------------------------------------------- */
static int comprobacion_parametros(
                       int , char * argv[], FILE ** ,FILE ** , FILE ** );

/* -------------------------------
 *  Variables globales.
 * ------------------------------- */
s_token * CabeceraListaTokens = NULL;   /* Tokens descritos por usuario */ 
s_token * CodigoIntermedio    = NULL;   /* Tokens con el lenguaje intermedio.*/ 

/* -------------------------------
 *  Arranque del programa de generacion de los stubs necesarios para
 * las llamadas remotas de acuerdo con la descripcion dada.
 * ----------------------------------------------------------------------- */
int main(int argc, char * argv[])
{
FILE * f_descripcion;           /* Fichero descripcion del Interfaz. */
FILE * f_fuente;                /* Fichero Fuente Generado para el Interfaz. */
FILE * f_prototipo;             /* Fichero con el prototipo de llamadas. */

 /* ---------------------------------
  *  Comprobar los parametros que nos pasan.
  * -------------------------------------------------- */
  if (comprobacion_parametros(argc,argv,&f_descripcion,&f_fuente,&f_prototipo))
  {
   /* ---------------------------------
    *  Realizar las operaciones clasicas de scaner, parser y generacion
    * de codigo. 
    * ------------------------------------------------------------------- */
     Scaner(f_descripcion);
     Parser();
     genera_codigo(f_prototipo,f_fuente);
  }
  else
  {
    fprintf(stderr," %s \n",ERR_NRO_PARAMETROS);
    fprintf(stderr,
       " Sintasix: src modelo \n");
    exit(1);
   }
  return 0;
}

/* --------------------------------------------------------------
 *  Funcion : Comprobacion de parametros.
 *
 *  Comprueba que los parametros dados a la herramienta son los 
 * correctos de acuerdo con la sintasix:
 *
 *         src_gen fichero_descripcion 
 * -------------------------------------------------------------- */
int comprobacion_parametros(int argc, char * argv[], FILE ** f_descripcion,
			    FILE ** f_fuente, FILE ** f_prototipo)
{
  char n_fichero[50];
  char * ext;

  switch(argc)
  {
       case 2: break;
       default: return FALSE;
  }

  /* ----------------------------------------
   *  El fichero tiene que tener la extension (src).
   * ------------------------------------------------------ */
  strcpy(n_fichero,argv[1]);
  ext = strrchr(n_fichero, '.');
  if (!ext || strcmp(ext,".src")!=0) 
  {
     fprintf(stderr," %s \n",ERR_PARAMETROS);
     fprintf(stderr," %s %s \n", ERR_EXT_F_FUENTE,n_fichero);
     exit(1);
  }

  /* ----------------------------------------
   *  Si lo puedo abrir de lectura. 
   * ------------------------------------------------------ */
  if ((*f_descripcion = fopen(n_fichero,"r")) == NULL)
  {
     fprintf(stderr," %s \n",ERR_PARAMETROS);
     fprintf(stderr," %s %s \n", ERR_ABRIR_F_DESCRIP,n_fichero);
     exit(1);
  }

  /* ----------------------------------------
   *  Si puedo crear el .h y .c correspondiente al 
   * fichero de descripcion. 
   * ------------------------------------------------------ */
  n_fichero[strlen(n_fichero)-4]='\0';
  strcat(n_fichero,".h");
  if ((*f_prototipo = fopen(n_fichero,"w")) == NULL)
  {
     fprintf(stderr," %s \n",ERR_PARAMETROS);
     fprintf(stderr," %s %s \n", ERR_ABRIR_F_PROTO,n_fichero);
     exit(1);
  }
  n_fichero[strlen(n_fichero)-2]='\0';
  strcat(n_fichero,".c");
  if ((*f_fuente = fopen(n_fichero,"w")) == NULL)
  {
     fprintf(stderr," %s \n",ERR_PARAMETROS);
     fprintf(stderr," %s %s \n",ERR_ABRIR_F_FUENTE,n_fichero);
     exit(1);
  }
  /* ----------------------------------------
   *  Todo ira bien. 
   * ------------------------------------------------------ */
  return TRUE;
}
