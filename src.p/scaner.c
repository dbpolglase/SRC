/* -----------------------------------------------------------------
 *
 *   Modulo que realiza el scanner de la descripcion de la  
 * facilidades de mensajes. 
 *
 * ---------------------------------------------------------------- */
/* --------------------------------------
 *  Modulos que utiliza de alguna forma. 
 * -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "src_gen.h"
#include "src_stri.h"
#include "scaner.h"

/* -----------------------------------------------
 *  Definicion de constantes utilizadas por el modulo. 
 * ------------------------------------------------------- */
#define MAX_CADENA MAX_TEXTO

/* -----------------------------------------------
 *  Cadenas de texto cuyo significado es un token 
 * interesante.
 * ------------------------------------------------------- */
#define S_CONST  	"CONST"
#define S_ENTRADA  	"IN"
#define S_ENTERO   	"INT"
#define S_SALIDA   	"OUT"
#define S_STRING   	"CHAR[]"
#define S_CARACTER 	"CHAR"
#define S_LONG     	"LONG"
#define S_FLOAT     	"FLOAT"
#define S_DOUBLE     	"DOUBLE"
#define S_CLASE   	"CLASE"
#define S_P_ARRAY       "<"
#define S_FIN_ARRAY     ">"
#define S_P_CLASE       "{"
#define S_FIN_CLASE     "}"
#define S_FIN_METODO    ";"
#define S_IGUALDAD      "="
#define S_COMENTARIO    "//"
#define S_P_PARAMETROS  "::"
#define S_S_PARAMETROS  ","

/* --------------------------------------
 *  Variables externas que se requieren.
 * -------------------------------------- */
extern s_token * CabeceraListaTokens;

/* ------------------------------------------------------------
 *  Funciones privadas que utiliza.
 * ------------------------------------------------------------ */
/* ------------------------------------------------------------
 *  Indica si los caracteres dados confirman un separador. 
 * ------------------------------------------------------------ */
static int EsSeparador(char , char,int * );
/* ------------------------------------------------------------
 *  Indica si un determinado caracter ha de ser tenido en
 * cuenta o no. 
 * ------------------------------------------------------------ */
static int EsCaracterRepresentativo(char );
/* ------------------------------------------------------------
 *  Analiza un cadena con vistas a identificar el token que 
 * se trata. 
 * ------------------------------------------------------------ */
static int AnalizaCadena(char * );
/* ------------------------------------------------------------
 *  Crea un elemento token. 
 * ------------------------------------------------------------ */
static s_token * ConstruyeToken(int ,char * , int );
/* ------------------------------------------------------------
 *  Annade un token a la lista de tokens. 
 * ------------------------------------------------------------ */
static void AnnadeToken(s_token *);
  
/* --------------------------------------------------------------
 *  Funcion : Scanner. 
 *
 *  Recorre el Fichero de Descripcion identificando los tokens. 
 *
 * -------------------------------------------------------------- */
void Scaner(FILE * f_descripcion)
{
  int anterior, actual;
  char cadena[MAX_CADENA];
  int  i = 0;
  int  representativos;
  int  tipo;
  int  linea = 1;
  s_token * token;

  if ((anterior = fgetc(f_descripcion)) == EOF)
  {
    fprintf(stderr," %s\n %s \n",ERR_SCANER,FICH_DESC_VACIO);
    exit (1);
  }

  /* ------------------------------------------------------------
   *  Mientras haya caracteres que leer en el fichero de descripcion.
   * ------------------------------------------------------------------ */
  while ((actual = fgetc(f_descripcion)) != EOF)
  {
     /* -----------------------------------------
      *  Si hemos encontrado un separador 
      * ----------------------------------------- */
     if (EsSeparador(anterior,actual,&representativos))
     {
       /* --------------------------------
        *  Estudiar la cadena y ver que token representa.
        * --------------------------------------------------- */
        if (i != 0)
        {
          cadena[i++] = '\0';
          tipo = AnalizaCadena(cadena); 
          token = ConstruyeToken(tipo,cadena,linea);
          AnnadeToken(token); 
          i = 0;
        }
       /* -------------------------------
        *  Estudiar ahora el separador. 
        * --------------------------------------------------------- */
        cadena[0] = anterior;
        if (representativos == 1)
          cadena[1] = '\0';
        else
        {
          cadena[1] = actual;
          cadena[2] = '\0';
        }

        tipo = AnalizaCadena(cadena); 
       /* -------------------------------
        *  El separador era un comentario pasar a la siguiente linea.
        * --------------------------------------------------------- */
        if (tipo == T_COMENTARIO)
        {
          while ((anterior != '\n') &&
             (anterior = fgetc(f_descripcion)) != EOF)
           ;
          linea ++;

        }
        else
        {
         /* -------------------------------
          *  Genera el token del separador. 
          * ----------------------------------- */
          if (representativos > 0)
          {
            token = ConstruyeToken(tipo,cadena,linea);
            AnnadeToken(token); 
          }
        }

       /* -------------------------------
        *  Consume de acuerdo al tamano del operador. 
        * ------------------------------------------- */
        if (representativos == 2)
        {
           if ((anterior = fgetc(f_descripcion)) == EOF)
             return;
        }
        else
         anterior = actual;

      }
      else
      {
       /* -------------------------
        *  Si no hay separador annadir el caracter a la cadena que
        * luego se analizara siempre que no sea un blanco, tabulador
        * o espacio en blanco.
        * ---------------------------------------------------------- */
       if (i < MAX_CADENA) 
       {
          if (EsCaracterRepresentativo(anterior))
          {
            cadena[i] = anterior;
            i++;
          }
          if (anterior == '\n') linea ++;
          anterior = actual;
       }
       else
       {
         fprintf(stderr," %s \n %s %d \n",ERR_SCANER,TOKEN_MUY_GRANDE,linea);
         exit(1);
       }
     }
  }
  /* ---------------------------
   *  Mira el resto de cadena que queda.
   * --------------------------------------- */
  cadena[i] = anterior;
  cadena[i++] = '\0';
  tipo = AnalizaCadena(cadena); 
  token = ConstruyeToken(tipo,cadena,linea);
  /* -------------------------------
   *  El token era un comentario pasar a la siguiente linea.
   * --------------------------------------------------------- */
   if (token != NULL && token->tipo == T_COMENTARIO)
     return;

   AnnadeToken(token); 
}
   
/* --------------------------------------------------------------
 *  Funcion : EsSeparador. 
 *
 *  Indica si los caracteres leidos son separadores de tokens o no. 
 * y cuantos de estos son representativos del separador, uno o dos.
 *
 *   Parametros
 *      Entrada:
 *             anterior,   caracter leido con anterioridad.
 *             actual,     caracter leido actualmente. 
 *      Salida:
 *             representativos, cuantos caracteres consume el separador.
 *      Retorna:
 *             1, hay separacion de tokens.
 *             0, no hay separacion de tokens, seguimos con el mismo.
 *    
 * ---------------------------------------------------------------- */
static int EsSeparador(char anterior, char actual,int * representativos)
{
  /* --------------------------
   *  Separador simple.
   * --------------------------------- */
  if ((anterior == ',') ||                   
      (anterior == '{') ||
      (anterior == '}') ||
      (anterior == ';') ||
      (anterior == '=') ||
      (anterior == '<') ||
      (anterior == '>')) 
  {
     *representativos = 1;
     return 1;
  }
  /* --------------------------
   *  Separador doble. 
   * --------------------------------- */
  if ((anterior == '/' && actual == '/') ||   
      (anterior == ':' && actual == ':')) 
  {
     *representativos = 2;
     return 1;
  }
  /* --------------------------
   *  Separador nulo. 
   * --------------------------------- */
  if (anterior == ' ')
  {
    *representativos = 0;
     return 1;
  }

return 0;
}

/* --------------------------------------------------------------
 *  Funcion : AnalizaCadena
 *
 *  Analiza la cadena con vistas a reconocer el token de que se 
 * trata.
 * 
 *    Parametros
 *       Entrada: 
 *            cadena, cadena de texto entre dos separadores y que
 *                    constituye por tanto un token.
 *       Retorna:
 *            tipo, token que representa esta cadena.
 * ---------------------------------------------------------------- */
static int AnalizaCadena(char * cadena)
{
   int lon,i;
   char aux_cadena[MAX_CADENA];
   int tipo = T_TEXTO;     /* De momento es una cadena de texto */

   lon = strlen(cadena);

   /* --------------
    * Para una correcta comparacion lo pasamos a mayusculas.
    * ------------------------------------------------------- */
    for (i =0; i <= lon; i++)
       aux_cadena[i] = toupper(cadena[i]);

   /* ----------------------------------
    * Con vistas a reducir la busqueda nos basaremos en que conocemos
    * la longitud de los token en caracteres.
    * ----------------------------------------------------------------- */
   switch(lon)
   {
     case 1:  if (strcmp(aux_cadena,S_IGUALDAD) == 0)
              {
                tipo = T_IGUALDAD; 
                break;
              }
	      if (strcmp(aux_cadena,S_FIN_METODO) == 0)
              {
                tipo = T_FIN_METODO; 
                break;
              }
	      if (strcmp(aux_cadena,S_FIN_CLASE) == 0)
              {
                tipo = T_FIN_CLASE; 
                break;
              }
	      if (strcmp(aux_cadena,S_P_CLASE) == 0)
              {
                tipo = T_P_CLASE; 
                break;
              }
	      if (strcmp(aux_cadena,S_S_PARAMETROS) == 0)
              {
                tipo = T_S_PARAMETROS; 
                break;
              }
	      if (strcmp(aux_cadena,S_P_ARRAY) == 0)
              {
                tipo = T_P_ARRAY; 
                break;
              }
	      if (strcmp(aux_cadena,S_FIN_ARRAY) == 0)
              {
                tipo = T_FIN_ARRAY; 
                break;
              }
              break;

     case 2:  
	      if (strcmp(aux_cadena,S_COMENTARIO) == 0)
              {
                tipo = T_COMENTARIO; 
                break;
              }
	      if (strcmp(aux_cadena,S_ENTRADA) == 0)
              {
                tipo = T_IN; 
                break;
              }
	      if (strcmp(aux_cadena,S_P_PARAMETROS) == 0)
              {
                tipo = T_P_PARAMETROS; 
                break;
              }
              break;

     case 3:  if (strcmp(aux_cadena,S_ENTERO) == 0)
              {
                tipo = T_INT; 
                break;
              }
              if (strcmp(aux_cadena,S_SALIDA) == 0)
              {
                tipo = T_OUT; 
                break;
              }
              break;
     
     case 4:  if (strcmp(aux_cadena,S_CARACTER) == 0)
              {
                tipo = T_CARACTER; 
                break;
              }
              if (strcmp(aux_cadena,S_LONG) == 0)
              {
                tipo = T_LONG; 
                break;
              }
              break;

     case 5:
              if (strcmp(aux_cadena,S_CONST) == 0)
              {
                tipo = T_CONSTANTE; 
                break;
              }
	      if (strcmp(aux_cadena,S_CLASE) == 0)
              {
                tipo = T_CLASE; 
                break;
              }
	      if (strcmp(aux_cadena,S_FLOAT) == 0)
              {
                tipo = T_FLOAT; 
                break;
              }
               break;

     case 6:  if (strcmp(aux_cadena,S_STRING) == 0)
              {
                tipo = T_STRING; 
                break;
              }
	      if (strcmp(aux_cadena,S_DOUBLE) == 0)
              {
                tipo = T_DOUBLE; 
                break;
              }
               break;
     default:
               tipo = T_TEXTO;
               break;
   }
   return tipo;
}

/* --------------------------------------------------------------
 *  Funcion : ConstruyeToken
 *
 *  Con los datos que ya conocemos de caracteres tipo, linea, etc.
 *  construimos el token propiamente dicho.
 *
 *  Parametros
 *      Entrada: 
 *           tipo, tipo del token.
 *           cadena, texto asociado al mismo.
 *           linea, linea en el fichero donde se ha encontrado.
 *       Retorna:
 *          NULL, fallo la construccion.
 *          token, el token construido.
 * ---------------------------------------------------------------- */
static s_token * ConstruyeToken(int tipo,char * cadena, int linea)
{
  s_token * token;

   /* -----------------------------------
    *  Pedimos la memoria necesaria para su construccion y la rellenamos.
    * -------------------------------------------------------------------- */
   token = (s_token *) malloc(sizeof(s_token));
   if (token == NULL)
   {
     fprintf(stderr,"%s\n",ERR_SIN_MEMORIA);
     exit (1);
   }
   token->tipo = tipo;
   token->linea = linea;
   token->siguiente = NULL;
   token->anterior = NULL;
   /* ------------------------------------
    * Si el token es de texto lo importante es la cadena que lo
    * representa.
    * ------------------------------------------------------------ */
   if (token->tipo == T_TEXTO)
   {
     if (strcmp(cadena,"") == 0)
        return NULL;
     token->texto = (char *)malloc(strlen(cadena) + 1);
     if (token->texto == NULL)
     {
       fprintf(stderr,"%s\n",ERR_SIN_MEMORIA);
       exit (1);
      }
     strcpy(token->texto,cadena);
   }
   else
     token->texto = NULL;
 
   return(token);
}

/* --------------------------------------------------------------
 *  Funcion : EsCaracterRepresentativo. 
 *
 *  Indica si hay que ignorar o no un determinado caracter. 
 *
 *    Parametros
 *       Entrada:
 *           caracter, caracter a estudiar.
 *       Retorna:
 *           0, no es representativo.
 *           1, si es representativo.
 * ---------------------------------------------------------------- */
static int EsCaracterRepresentativo(char caracter)
{
   /* ----------------------------
    *  Ignoramos blancos, saltos de linea y tabuladores.
    * ------------------------------------------------------- */
   if ((caracter == ' ') || (caracter == '\n') || (caracter == '\t')) 
      return 0;

   return 1;
}

/* --------------------------------------------------------------
 *  Funcion : AnnadeToken. 
 *
 *  Anade un token a la lista de tokens encontrados ya en el 
 * fichero de descripcion de servicios.
 *  
 *   Parametros:
 *       Entrada:
 *            token, token a unir a la lista.
 * ---------------------------------------------------------------- */
static void AnnadeToken(s_token * token)
{
  static s_token * actual = NULL;

  if (token == NULL)
    return;

  /* --------------------------------------
   *   El actual es el ultimo token enlazado, este se pone 
   * a continuacion.
   * ------------------------------------------------------------------- */
  if (actual == NULL)
   CabeceraListaTokens = actual = token;
  else
  {
   actual->siguiente = token;
   token->anterior = actual;
   actual = token;
  }
}

