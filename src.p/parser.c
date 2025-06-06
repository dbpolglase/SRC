/* ---------------------------------------------------------------------
 *   MODULO PARSER.
 *
 *   Modulo encargado de realizar el parser del fichero de descripcion
 * de servicios de un fuente.
 * 
 *   La gramatica admisible es:
 * 
 *   CONST XXXXX = aaaaaaaa
 *   CONST XXXXX = numero.
 *   
 *   Clase NombreObjeto    
 *   { 
 *   Metodo :: in  tipo [variable],
 *                 tipo [variable],
 *              ................
 *             out tipo [variable],
 *                 tipo [variable],
 *              ................
 *             = version;
 *   Metodo :: ..................
 *   }
 *
 * ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "src_stri.h"
#include "src_gen.h"

/* ------------------
 *  Definicion de constantes utilizadas. 
 * ------------------------------------- */
#define MAX_NOMBRE_CONSTANTE  MAX_TEXTO
#define MAX_VALOR_CONSTANTE   MAX_TEXTO

/* ------------------
 *  Variables globales utilizadas.
 * ------------------------------------- */
extern s_token * CabeceraListaTokens;
extern s_token * CodigoIntermedio;

/* ------------------
 *  Estructuras a usar.
 * ------------------------------------- */
struct ListaConstantes {
			char * Nombre;
			char * Valor;
 			struct ListaConstantes * siguiente;
		      };
typedef struct ListaConstantes s_lconstantes;

/* ------------------
 *  Funciones propias del modulo. 
 * ------------------------------------- */

/* ------------------
 *  Realiza el analisis de la descripcion de una constante. 
 * ----------------------------------------------------------- */
static s_token * AnalizaConstante(s_token * );
/* ------------------
 *  Realiza el analisis de la descripcion de un objeto.
 * ----------------------------------------------------------- */
static s_token * AnalizaClase(s_token * );
/* ------------------
 *  Realiza el analisis de la descripcion de un metodo. 
 * ----------------------------------------------------------- */
static s_token * AnalizaMetodo(s_token * );
/* ------------------
 *  Hace el analisis de los parametros de un metodo. 
 * ----------------------------------------------------------- */
static s_token * AnalizaParametros(s_token * );
/* ------------------
 *  Da un error indicado simbolo inexperado en analisis. 
 * ----------------------------------------------------------- */
static void ErrorSimboloInesperado(s_token * );
/* ------------------
 *  Indica si un determinado token representa un parametro. 
 * ----------------------------------------------------------- */
static int EsTipoParametro(s_token * );
/* ------------------
 *  Obtener el siguiente token a analizar. 
 * -------------------------------------------- */
static s_token * ObtenerSiguienteToken(s_token * );
/* ------------------
 *  Annade una constante a la lista de Constantes. 
 * -------------------------------------------------- */
static void  AnnadeConstate(char * ,char * );
/* ------------------
 *  Annade un elemento al lenguaje intermedio.
 * -------------------------------------------------- */
static void  AnnadeIntermedio(int token, char * valor);

/* ----------------------------------------------------------------------
 *   Sustituye un texto si corresponde a una constante por su valor. 
 * ---------------------------------------------------------------------- */
static void  SustituyeConstante(s_token * token);

/* ---------------------------------------
 *  Cabecera de la lista de constantes.
 * ----------------------------------------- */
static s_lconstantes * CabeceraConstantes = NULL;

/* ----------------------------------------------------------------------
 *  Funcion: Parser.
 *   Realiza el parser de la gramatica.
 *   Como la gramatica es muy simple no es necesario hacer un parser 
 *  descendente ni nada similar, basta con ir recorriendo los tokens
 *  y ver que viene en su orden. 
 * ---------------------------------------------------------------------- */
void Parser()
{
   s_token * aux;

   aux = CabeceraListaTokens;

   while(aux != NULL)
   {
      switch(aux->tipo)
      {
        case T_CLASE:       aux = AnalizaClase(aux);
			    break;
        case T_CONSTANTE:   aux = AnalizaConstante(aux);
			    break;
        default:  
		fprintf(stderr," ERROR PASER "); 
		fprintf(stderr," %s %d %s \n",ERR_EN_LINEA,aux->linea,ERR_METODO_SIN_CLASE);
		exit(1);
      }
      aux = aux->siguiente;
   }
}

/* ----------------------------------------------------------------------
 *  Funcion: AnalizaConstante. 
 *   Analiza una constante y annadela a la lista de constante por si 
 * se hace uso de ella mas adelante. 
 *
 *   Parametros
 *       Entrada:
 *           token, token que representa a una constante por lo 
 *                  que hay que estudiar y asignar un valor. 
 *       Retorna:
 *          Siguiente token en la lista a estudiar.
 * ---------------------------------------------------------------------- */
static s_token * AnalizaConstante(s_token * token) 
{
   char NombreConstante[MAX_NOMBRE_CONSTANTE];
   char ValorConstante[MAX_VALOR_CONSTANTE];

   /* ------------------------------------------
    *   El siguiente token debe ser un token de texto con el nombre 
    * de la variable cuyo valor vamos a dar.
    * ------------------------------------------------------------ */
   token = ObtenerSiguienteToken(token);

   if (token->tipo != T_TEXTO)
        ErrorSimboloInesperado(token);

   if (strlen(token->texto) >= MAX_NOMBRE_CONSTANTE)
   {
	fprintf(stderr," ERROR PASER "); 
        fprintf(stderr," %s %s %s %d \n", ERR_NOM_CONST_LARGA,
			token->texto,ERR_EN_LINEA,token->linea); 
                exit(1);
   }
   else
     strcpy(NombreConstante,token->texto);
   
    /* ------------------------------------
     *  VARIABLE = VALOR;
     * ------------------------------------- */

   token = ObtenerSiguienteToken(token);

   if (token->tipo != T_IGUALDAD)
        ErrorSimboloInesperado(token);

   /* ------------------------------------------
    *   El siguiente token debe ser un token de texto con el valor 
    * de la constante.
    * ------------------------------------------------------------ */
   token = ObtenerSiguienteToken(token);

   if (token->tipo != T_TEXTO)
        ErrorSimboloInesperado(token);

   if (strlen(token->texto) >= MAX_VALOR_CONSTANTE)
   {
	fprintf(stderr," ERROR PASER "); 
        fprintf(stderr," %s %s linea %d \n", ERR_VAL_CONST_LARGA,
			token->texto,token->linea); 
		exit(1);
   }
   else
     strcpy(ValorConstante,token->texto);
 
   /* -------------------------------------
    *   La ponemos en la lista de constantes.
    * ---------------------------------------------- */
   AnnadeConstate(NombreConstante,ValorConstante); 

   /* -------------------------------------
    *   Y se acabo la descripcion de la constante.
    * ---------------------------------------------- */
   token = ObtenerSiguienteToken(token);

   if (token->tipo != T_FIN_METODO)
        ErrorSimboloInesperado(token);

   return(token);
}

/* ----------------------------------------------------------------------
 *  Funcion: AnalizaClase.
 *   Analiza la descripcion de una clase, es decir de los metodos que
 * la componen.
 *
 *    Parametros
 *        Entrada:
 *           token, token de la lista que representa una clase.
 *        Retorna:
 *           token, token de la lista donde acaba la descripcion de 
 *                  la clase. 
 * ---------------------------------------------------------------------- */
static s_token * AnalizaClase(s_token * token)
{
   /* -----------------------------------
    * El siguiente token es el nombre de la clase.
    * ------------------------------------------------- */
   token = ObtenerSiguienteToken(token);

   if (token->tipo != T_TEXTO)        /* Nombre del Token */
   {

     ErrorSimboloInesperado(token);

/*
     fprintf(stderr,"%s \n %s %s %d  \n",
           ERR_PARSER,ERR_SIMBOLO_INESPERADO,ERR_EN_LINEA,token->linea);
*/
     exit(1);
   }

   /* -----------------------------------
    * Lo anadimos al lenguaje intermedio que luego nos facilita la 
    * vida a la hora de generar el codigo de interfaz y fuente. 
    * ------------------------------------------------------------- */
   AnnadeIntermedio(T_CLASE,token->texto);

   /* ---------------------------
    *  Clase nombre {
    * ------------------------------------- */
   token = ObtenerSiguienteToken(token);

   if (token->tipo != T_P_CLASE)
   {
     fprintf(stderr,"%s \n %s %s %d  \n",
               ERR_PARSER,ERR_PRI_CLASE,ERR_EN_LINEA,token->linea);
     exit(1);
   }

   /* ---------------------------------
    *  Comienzo de Metodos.
    * ------------------------------------- */
   token = ObtenerSiguienteToken(token);

   /* ---------------------------
    *  Mientras no encontremos el fin de la clase }
    *  todo deben ser metodos analizarlos.
    * ------------------------------------- */
   while(token->tipo != T_FIN_CLASE)
   {
     if (token->tipo == T_TEXTO)
       token = AnalizaMetodo(token);
     else
        ErrorSimboloInesperado(token);
   }
   return token;
 }
   
/* ----------------------------------------------------------------------
 *  Funcion: AnalizaMetodo. 
 *   Analiza la descripcion de un metodo. 
 *
 *    Parametros
 *        Entrada:
 *           token, token de la lista que representa un metodo.
 *        Retorna:
 *           token, token de la lista donde acaba la descripcion del 
 *                  metodo. 
 * ---------------------------------------------------------------------- */
static s_token * AnalizaMetodo(s_token * token) 
{
   AnnadeIntermedio(T_METODO,token->texto);

   /* --------------------------------------------
    *  nombre metodo ::
    * --------------------------------------------- */
   token = ObtenerSiguienteToken(token); 

   if (token->tipo != T_P_PARAMETROS)
   {
     fprintf(stderr,"%s \n %s %s %d  \n",
           ERR_PARSER,ERR_P_PARAMETROS,ERR_EN_LINEA,token->linea);
     exit(1);
   }

   token = ObtenerSiguienteToken(token); 

   /* --------------------------------------------
    *  Mientras no encontremos el = de fin de metodo
    * estudiar los parametros. 
    * --------------------------------------------- */

   while(token->tipo != T_IGUALDAD)
   {
     switch (token->tipo) 
     {
       case T_IN:
              /* -------------------------------------
               *   De entrada.
               * ------------------------------------ */
              AnnadeIntermedio(T_IN,"");
	      token = AnalizaParametros(token);
              break;
       case T_OUT: 
              /* -------------------------------------
               *   Y de salida.
	       * ------------------------------------ */
              AnnadeIntermedio(T_OUT,"");
	      token = AnalizaParametros(token);
              break;
       default:
              ErrorSimboloInesperado(token);
      }
   }

   /* --------------------------------------------
    *  nombre metodo ::
    *              parametros ......
    *               = version
    * --------------------------------------------- */
   token = ObtenerSiguienteToken(token); 

   if (token->tipo != T_TEXTO)
      ErrorSimboloInesperado(token);

   AnnadeIntermedio(T_VERSION,token->texto);

   /* --------------------------------------------
    *  nombre metodo ::
    *              parametros ......
    *               = version;
    * --------------------------------------------- */
   token = ObtenerSiguienteToken(token); 

   if (token->tipo != T_FIN_METODO)
      ErrorSimboloInesperado(token);

    return (ObtenerSiguienteToken(token)); 
}

   
/* ----------------------------------------------------------------------
 *  Funcion: AnalizaParametros. 
 *   Analiza la descripcion de los parametros de un metodo. 
 *
 *    Parametros
 *        Entrada:
 *           token, token de la lista que representa un 
 *                  comienzo de parametros.
 *        Retorna:
 *           token, token de la lista donde acaba la descripcion de
 *                  los parametros. 
 * ---------------------------------------------------------------------- */
static s_token * AnalizaParametros(s_token * token) 
{
   int tipo_parametros;
   int i;

   tipo_parametros = token->tipo;  /* son de entrada o de salida */

   /* -------------------------------------------------
    *   Mientras no encontremos un finalizador de parametros.
    * --------------------------------------------------------- */
   token = ObtenerSiguienteToken(token); 

   while(token->tipo != T_IGUALDAD && token->tipo != T_OUT && 
         token->tipo != T_IN)
   {
       /* ------------------------------------------
        *  El token representa un parametro ?
        * ------------------------------------------ */
       if (!EsTipoParametro(token))
	 ErrorSimboloInesperado(token);

       AnnadeIntermedio(token->tipo,"");   /* Indica un nuevo parametro */

       /* ------------------------------------------
        *  Nombre que le das al parametro. 
        * ------------------------------------------ */
       token = ObtenerSiguienteToken(token); 
       if (token->tipo == T_P_ARRAY)
       {
         /* ------------------------------------------
	  *  Se trata de un array.
          * ------------------------------------------ */
         token = ObtenerSiguienteToken(token); 
         /* ------------------------------------------
          *  La dimension del mismo es correcta. 
          * ------------------------------------------ */
         if (token->tipo != T_TEXTO)
           ErrorSimboloInesperado(token);

	 for(i=0; token->texto[i] != '\0';i ++)
	 {
	    if (!isdigit(token->texto[i]))
	    {
		fprintf(stderr,"%s \n",ERR_PARSER);
		fprintf(stderr,"%s %s %d ",ERR_VALOR_ARRAY_ERRONEO,
					   ERR_EN_LINEA,token->linea);
		exit(1);
	    }
	 }

	 /* ----------------
	  *  El array acaba bien.
	  * ---------------------------- */

	 AnnadeIntermedio(token->tipo,token->texto);
	 token = ObtenerSiguienteToken(token);
         if (token->tipo != T_FIN_ARRAY)
         {
                fprintf(stderr,"%s \n",ERR_PARSER);
                fprintf(stderr,"%s %s %d ",ERR_FIN_ARRAY,
                                           ERR_EN_LINEA,token->linea);
                exit(1);
        }


       /* ----------------
        *  Siguiente token. 
        * ---------------------------- */
        token = ObtenerSiguienteToken(token); 
     }
     /* -------------------------
      * Si el siguiente es texto suponer que es el nemo del parametro.
      * -------------------------------------------------------------- */
     if (token->tipo == T_TEXTO)
     /* ------------------------------------------
      *  Mas parametros. 
      * ------------------------------------------ */
       token = ObtenerSiguienteToken(token); 

     if (token->tipo == T_S_PARAMETROS)
       token = ObtenerSiguienteToken(token);
   }

  /* ------------------------------------------
   *  A terminado adecuadamente para el tipo 
   * de parametros que eran. 
   * ------------------------------------------ */
   if (tipo_parametros == T_OUT && token->tipo == T_IN)
      ErrorSimboloInesperado(token);

   if (tipo_parametros == T_IN && token->tipo == T_IN)
      ErrorSimboloInesperado(token);

    return(token);
 }
   
   
/* ----------------------------------------------------------------------
 *  Funcion: ErrorSimboloInesperado. 
 *   Devuelve un error indicado que el simbolo analizado no era esperado
 * en la cadena de analisis. 
 *
 *    Parametros
 *        Entrada:
 *           token, token inesperado en la secuencia de tokens. 
 * ---------------------------------------------------------------------- */
static void ErrorSimboloInesperado(s_token * token) 
{
   /* --------------
    *  Indicar error de parser.
    * -------------------------------- */
/*
   fprintf(stderr,"%s \n",ERR_PARSER);
   fprintf(stderr,"%s",ERR_SIMBOLO_INESPERADO);
*/

   /* --------------
    *  Y el token inesperado. 
    * -------------------------------- */
   switch(token->tipo)
   {
      case T_TEXTO: 
		    fprintf(stderr,"%s %s ", ERR_CADENA,token->texto);
                    break; 
      case T_IN: 
		    fprintf(stderr,"%s ", ERR_T_IN);
                    break; 
      case T_OUT: 
		    fprintf(stderr,"%s ", ERR_T_OUT);
                    break; 
      case T_CONSTANTE: 
		    fprintf(stderr,"%s ", ERR_T_CONSTANTE);
                    break; 
      case T_IGUALDAD: 
		    fprintf(stderr,"%s ", ERR_T_IGUALDAD);
                    break; 
      default:
		    fprintf(stderr,"ERROR SIMBOLO,");
                    break; 
    }
    fprintf(stderr,"%s %d\n",ERR_EN_LINEA,token->linea);
    exit(1);
}
   
/* ----------------------------------------------------------------------
 *  Funcion: EsTipoParametro. 
 *   Indica si un determinado token es del tipo parametro o no. 
 * 
 *  Parametros
 *      Entrada:
 *          token, token del que se desea saber si representa a un
 *                 tipo de parametro o no.
 *      Retorna:
 *          TRUE, si lo es.
 *          FALSE, no lo es.
 * ---------------------------------------------------------------------- */
static int EsTipoParametro(s_token * token) 
{
   if ((token->tipo == T_CARACTER) ||
       (token->tipo == T_INT)  ||
       (token->tipo == T_LONG)  ||
       (token->tipo == T_FLOAT)  ||
       (token->tipo == T_DOUBLE)  ||
       (token->tipo == T_STRING))
     return TRUE;

   return FALSE;
}

/* ----------------------------------------------------------------------
 *  Funcion: ObtenerSiguienteToken. 
 *   Retorna el siguiente token en la lista de tokens y da un error si
 * no hay mas. 
 *
 *   Parametros
 *       Entrada:
 *           token, lista de tokens del que se desea el siguiente.
 *       Retorna:
 *          NULL, se acabo.
 *          token, el siguiente.
 * ---------------------------------------------------------------------- */
static s_token * ObtenerSiguienteToken(s_token * token) 
{
   if (token->siguiente == NULL)
   {
     fprintf(stderr,"%s \n", ERR_PARSER);
     fprintf(stderr,"%s \n",ERR_FIN_FICHERO);
     exit(1);
   }
   
   /* --------------------------------------
    *  Mirar si el valor de este token se corresponde con el de
    * una constante.
    * -------------------------------------------------------------- */
   if (token->siguiente->tipo == T_TEXTO)
     SustituyeConstante(token->siguiente);

   return(token->siguiente);
}

/* ----------------------------------------------------------------------
 *  Funcion: AnnadeConstante 
 *   Annade una constante a la lista de constante para su posterior  
 * sustitucion. 
 *
 *   Parametros
 *       Entrada:
 *            NombreConstante, nombre de la constante.
 *            ValorConstante , valor  de la constante.
 * ---------------------------------------------------------------------- */
static void  AnnadeConstate(char * NombreConstante,char * ValorConstante)
{
  static s_lconstantes * SiguienteConstante = NULL;

  s_lconstantes * aux;

  /* ---------------------------------
   *  Crea la estructura.
   * ----------------------------------- */
  aux = (s_lconstantes *) malloc(sizeof(s_lconstantes));
  if (aux == NULL)
  {
    fprintf(stderr," %s \n",ERR_SIN_MEMORIA);
    exit (1);
  }
  else 
  {
     if (((aux->Nombre = (char *)malloc(strlen(NombreConstante) +1)) == NULL) ||
        ((aux->Valor = (char *) malloc(strlen(ValorConstante) +1)) == NULL))
     {
       fprintf(stderr," %s \n",ERR_SIN_MEMORIA);
       exit (1);
     }
  }

  strcpy(aux->Nombre,NombreConstante); 
  strcpy(aux->Valor,ValorConstante); 

  /* ---------------------------------
   *  Enlazala en la lista de constantes. 
   * --------------------------------------------- */
  
  if (CabeceraConstantes == NULL)
   SiguienteConstante = CabeceraConstantes = aux;
  else
  {
   SiguienteConstante->siguiente = aux;
   SiguienteConstante = SiguienteConstante->siguiente;
  }

}

/* ----------------------------------------------------------------------
 *  Funcion: AnnadeIntermedio 
 *   Annade un nuevo elemento al lenguaje intermedio. 
 * 
 *  Parametros
 *     Entrada:
 *       token, token intermedio a unir en la lista de tokens intermedios. 
 *       valor, valor de este token.
 * ---------------------------------------------------------------------- */
static void  AnnadeIntermedio(int token, char * valor)
{
  static s_token * aux;
  static s_token * actual = NULL;

  /* ---------------------------------
   *  Crea la estructura.
   * ----------------------------------- */
  aux = (s_token *) malloc(sizeof(s_token));
  if (aux == NULL)
  {
    fprintf(stderr," %s \n",ERR_SIN_MEMORIA);
    exit (1);
  }

  aux->tipo = token;
  aux->texto = (char *)malloc(strlen(valor) + 1);
  if (aux->texto == NULL)
  {
    fprintf(stderr," %s \n",ERR_SIN_MEMORIA);
    exit (1);
  }

  strcpy(aux->texto,valor);

  if (actual == NULL)
    CodigoIntermedio = actual = aux;
  else
  {
    actual->siguiente = aux;
    aux->anterior = actual;
    actual = aux;
  }
} 

/* ----------------------------------------------------------------------
 *  Funcion: SustituyeConstante 
 *   Sustituye un texto si corresponde a una constante por su valor. 
 *  
 *   Parametros
 *       Entrada:
 *            token, token que si representa una constante sustituremos
 *                   por su valor.
 * ---------------------------------------------------------------------- */
static void  SustituyeConstante(s_token * token)
{
  s_lconstantes * aux;

  aux = CabeceraConstantes;

  /* ----------------------------------
   *   Mira a ver si es una constante de las que tenemos en la lista.
   * ---------------------------------------------------------------- */
  while (aux != NULL)
  {
    if (strcmp(aux->Nombre,token->texto) == 0)
    {
       /* ----------------------------------
        *   SI, sustituir el texto por el valor de constante. 
        * ------------------------------------------------------- */
       free(token->texto);
       if ((token->texto = malloc(strlen(aux->Valor))) == NULL)
       {
          fprintf(stderr," %s \n",ERR_SIN_MEMORIA);
          exit (1);
       }
       strcpy(token->texto,aux->Valor);
       return;
    }
    aux = aux->siguiente;
  }
}

