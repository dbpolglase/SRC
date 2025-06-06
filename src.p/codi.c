/* ----------------------------------------------------------
 *  
 *   Modulo de generacion de Codigo.
 * 
 *  Este modulo es el mas complejo, ya que para construir los
 * fuentes de un metodo hay que generar el codigo en varias
 * partes de acuerdo con el lenguaje a generar.
 *  Por ello primero vamos a crear a partir del lenguaje intermedio
 * una lista de clase y metodos de la siguiente forma.
 *
 *   ----------        -----------       -----------  
 *  | CLASE    | -->  | CLASE     | --> |  CLASE    | 
 *   ----------        -----------       -----------  
 *       |
 *       |
 *       \/
 *   ----------        -----------       -----------  
 *  | METODO   | -->  | METODO    | --> | METODO   | 
 *   ----------        -----------       -----------  
 *       |
 *       | ------------------------------
 *       \/                             \/
 *   -------------------        -------------------- 
 *  | PARAMETRO ENTRADA |      | PARAMETRO SALIDA    |
 *   -------------------        --------------------- 
 *       |                              |
 *       |                              | 
 *       \/                             \/
 *   -------------------        -------------------- 
 *  | PARAMETRO ENTRADA |      | PARAMETRO SALIDA    |
 *   -------------------        --------------------- 
 *
 * ----------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interfaz.h"
#include "src_gen.h"
#include "src_stri.h"
#include "codi.h"

extern s_token * CodigoIntermedio;

static char SUBCLASE[MAX_SUBCLASE] = "";    /* Subclase a Generar */
static char CLASE[MAX_CLASE] = "";          /* Clase a Generar */
static struct Clases * ListaClases = NULL;  /* Clases a Generar. */

/* -------------------------------------------
 * Genera la cabecera de los ficheros generados.
 * ------------------------------------------- */
static void GeneraCabecera(FILE *,FILE *);

/* -----------------------------------------------
 *  Genera las estructuras de una clase. 
 * ----------------------------------------------- */
static s_token * GeneraClase(s_token * aux);

/* ----------------------------------------------
 *  Genera las estructuras de un metodo.
 * ---------------------------------------------- */
static s_token * GeneraMetodo(s_token *);

/* ----------------------------------------------
 *  Enlaza un objeto con el siguiente objeto descrito.
 * --------------------------------------------------- */
static void EnlazaClases(struct Clases * aux);

/* ----------------------------------------------
 *  Enlaza un metodo al objeto al que pertenece.
 * --------------------------------------------------- */
static void EnlazaMetodo(struct S_EnlaceObjetos *);

/* ----------------------------------------------
 *  Enlaza un parametro en la lista de parametros de un metodo.
 * -------------------------------------------------------------- */
static void EnlazaParametro(struct S_Parametros ** ,struct S_Parametros * );

/* ----------------------------------------------
 *  Escribe el fichero con la descripcion del interfaz dado. 
 * -------------------------------------------------------------- */
static void EscribeInterfaz(FILE *);

/* ----------------------------------------------
 *  Escribe el fichero con el fuente necesario 
 * para enlazar la descripcion y el programa. 
 * --------------------------------------------------- */
static void EscribeVariablesFuente(FILE *);

/* ----------------------------------------------
 *  Escribe las variables correspondientes a los parametros.
 * ---------------------------------------------------------------- */
static void GeneraVariablesParametros(struct S_EnlaceObjetos * ,FILE * );

/* ----------------------------------------------
 *  Genera el codigo de enlace entre en el objeto-metodo y la 
 * funcion.
 * --------------------------------------------------- */
static void EscribeCodigoEnlace(struct Clases * ,FILE * );

/* -------------------------------------------------
 *  Genera el codigo de la funcion que hace el enlace
 * entre el mensaje y el metodo.
 * ------------------------------------------------- */
static void EscribeCodigoFuncionEnlace(struct Clases *,FILE *);

/* -------------------------------------------------
 *  Escribe el prototipo de las funciones de usuario,
 * para que los compiladores mas pjotas o el C++ no se
 * quejen.
 * ------------------------------------------------------- */
static void EscribePrototipos(struct Clases * clase,FILE * f_fuente);
/* --------------------------------------------------
 *  Recorrer el leguaje intermedio generado a partir de
 * la descripcion del interfaz y generar el codigo de interfaz y
 * fuente necesario.
 * ---------------------------------------------------------------- */
void genera_codigo(FILE * f_prototipo,FILE * f_codigo)
{
   s_token * aux;

   GeneraCabecera(f_prototipo,f_codigo);   /* Pon Cabecera Ficheros */

   /* -------------------------------
    * Crea a partir del codigo intermedio una estructura que
    * se ajuste a la necesidades de la generacin del codigo.
    * tal como se explica al principio del fichero.
    * ------------------------------------------------------------- */
   aux = CodigoIntermedio;

   while(aux != NULL)
   {
     switch(aux->tipo)
     {
      case T_CLASE:  
		      aux = GeneraClase(aux);
		      break;
      case T_METODO:
		      aux = GeneraMetodo(aux);
		      break;
      default:
		      break;
     }
    if (aux != NULL) aux = aux->siguiente;
   }


   /* --------------------------
    *  Escribe el codigo del interfaz y el fuente, partiendo de esta
    * estructura.
    * -------------------------------------------------------------- */ 
   EscribeInterfaz(f_prototipo);
   EscribeVariablesFuente(f_codigo);
   EscribeCodigoEnlace(ListaClases,f_codigo);
   EscribePrototipos(ListaClases,f_codigo);
   EscribeCodigoFuncionEnlace(ListaClases,f_codigo);
}

/* --------------------------------------------------------
 *  Funcion: GeneraCabecera.
 *
 *    Escribe la cabecera que tienen que tener los ficheros
 * de interfaz y fuente.
 *
 *  Parametros
 *     Entrada:
 *          f_prototipo, fichero donde se va a escribir la 
 *                    descripcion del interfaz. 
 *          f_codigo, fichero donde se va a escribir el fuente.
 * --------------------------------------------------------------- */
static void GeneraCabecera(FILE * f_prototipo,FILE * f_codigo)
{
   fprintf(f_prototipo,"%s\n",ADVERTENCIA_SRC);

   fprintf(f_codigo,
     " %s \n#include <stdio.h>\n#include <stdarg.h>\n#include \"textos.h\"\n#include \"src.h\"\n#include \"recep.h\"\n#include \"emisor.h\"\n#include \"codifica.h\"\n\n",ADVERTENCIA_SRC);
}

/* ---------------------------------------
 *  Funcion: GeneraClase.
 *
 *    Genera una estructura para una clase.
 *
 *  Parametros
 *     Entrada:
 *          token, token del lenguaje intermedio donde comienza 
 *                 la descripcion de una clase. 
 *     Retorna:
 *          token, token del lenguaje intermedio donde acaba la 
 *                 descripcion de la clase.
 * --------------------------------------------------------------- */
static s_token * GeneraClase(s_token * aux)
{
  struct Clases * aux_clases;

  /* ---------------------------------------
   *  Crea la estructura clase.
   * ----------------------------------------- */
  strncpy(CLASE,aux->texto,MAX_CLASE);

  if ((aux_clases = (struct Clases *) malloc(sizeof(struct Clases))) == NULL)
  {
    fprintf(stderr,"%s\n",ERR_GENERACION);
    fprintf(stderr,"%s\n",ERR_SIN_MEMORIA);
    exit(1);
  }

  strcpy(aux_clases->Clase,"");
  if (strcmp(CLASE,"") != 0)
    strcat(aux_clases->Clase,CLASE);

  /* ---------------------------------------
   *  Preparados para que se puedan describir subclases. 
   * ---------------------------------------------------------- */
  if (strcmp(SUBCLASE,"") != 0)
  {
    strcat(aux_clases->Clase,".");
    strcat(aux_clases->Clase,SUBCLASE);
  }

  /* ---------------------------------------
   *  Y enlazala con el resto de ellas. 
   * ----------------------------------------- */
  aux_clases->metodos = NULL;
  aux_clases->siguiente = NULL;

  EnlazaClases(aux_clases);
  return (aux);
}

/* ---------------------------------------
 *  Funcion: EnlazaClases.
 *
 *  Enlaza la estructura de una clase con la lista de clases. 
 * 
 *  Parametros
 *      Entrada:
 *          aux, estructura que describe la clase a enlazar.
 * ---------------------------------------------------------------- */
static void EnlazaClases(struct Clases * aux)
{
  static struct Clases * actual;

   if (ListaClases == NULL)
    ListaClases = actual = aux;
   else
   {
     actual->siguiente = aux;
     if (ListaClases->siguiente == NULL)
       ListaClases->siguiente = actual;
     actual = aux;
   }
}

/* ---------------------------------------
 *  Funcion : GeneraMetodo.
 * 
 *     Crea una estructura para un metodo. 
 *
 *  Parametros
 *     Entrada:
 *          token, token del lenguaje intermedio donde comienza 
 *                 la descripcion de un metodo. 
 *     Retorna:
 *          token, token del lenguaje intermedio donde acaba la 
 *                 descripcion del metodo. 
 * ---------------------------------------------------------------- */
static s_token * GeneraMetodo(s_token * token)
{
  struct S_EnlaceObjetos * aux;
  struct S_Parametros * aux_param = NULL;
  struct S_Parametros * p_entrada = NULL;
  struct S_Parametros * p_salida = NULL;
  int flag_entrada = 0;
 
  /* -------------------------------
   * Crea la estructura basica y rellenala.
   * -------------------------------------------- */
  if ((aux = (struct S_EnlaceObjetos *) malloc(sizeof(struct S_EnlaceObjetos))) == NULL) 
  {
    fprintf(stderr,"%s\n",ERR_GENERACION);
    fprintf(stderr,"%s\n",ERR_SIN_MEMORIA);
    exit(1);
  }

  strcpy(aux->Metodo,token->texto);
  aux->siguiente = NULL;
  token = token->siguiente;

  /* -------------------------------
   * Meter ahora los parametros, como listas dependientes de esta. 
   * ------------------------------------------------------------------- */
  while (token != NULL && token->tipo != T_CLASE && token->tipo != T_METODO)
  {
    switch(token->tipo)
    {
       case T_CARACTER:
       case T_INT:
       case T_STRING:
       case T_LONG:
       case T_FLOAT:
       case T_DOUBLE:
            /* ----------------------------------------
             *  Creo el parametro y lo incluyo en la lista de 
             * parametros de entrada o salida de acuerdo con lo
             * que corresponda.
             * ------------------------------------------------- */
	    if ((aux_param = (struct S_Parametros *) 
			    malloc(sizeof(struct S_Parametros))) == NULL)
	    {
	       fprintf(stderr,"%s\n",ERR_GENERACION);
	       fprintf(stderr,"%s\n",ERR_SIN_MEMORIA);
	       exit(1);
	    }
            aux_param->tipo = token->tipo;
	    aux_param->siguiente = NULL;
             /* -------------------------------
              *  Mirar la logitud si es un array.
              * -------------------------------------- */
            if (token->siguiente->tipo != T_TEXTO) 
	      aux_param->lon = 1;
            else
            {
               token = token->siguiente;
	       aux_param->lon = atoi(token->texto);
            }
            if (flag_entrada == 0)
	      EnlazaParametro(&p_entrada,aux_param);
	    else
	      EnlazaParametro(&p_salida,aux_param);
	    break;

     case T_OUT:
	   flag_entrada = 1;
	   break;
     case T_VERSION:
 	   aux->version = atoi(token->texto);
	   break;
    }
    token = token->siguiente;
  }
  /* -----------------------------------------
   *  Enlaza el metodo a la lista de metodos de la clase, y devuelve 
   * un token que permita continuar con el trabajo de reconocimiento.
   * --------------------------------------------------------------- */
  aux->p_entrada = p_entrada;
  aux->p_salida = p_salida;
  EnlazaMetodo(aux);

  if (token != NULL)
   return(token->anterior);
  else
   return NULL;
}

/* ----------------------------------------------
 *  Enlaza el metodo en la ultima clase que tenemos descrita que
 * es donde logicamente debe ir. 
 * ---------------------------------------------------------------- */
static void  EnlazaMetodo(struct S_EnlaceObjetos * metodo)
{
  struct Clases * aux_clases;
  struct S_EnlaceObjetos * aux_metodo;

   aux_clases = ListaClases;

   while(aux_clases->siguiente != NULL)
     aux_clases = aux_clases->siguiente;

   if (aux_clases->metodos == NULL)
     aux_clases->metodos = metodo;
   else
   {
     aux_metodo = aux_clases->metodos;
     while(aux_metodo->siguiente != NULL)
      aux_metodo = aux_metodo->siguiente;
     aux_metodo->siguiente = metodo;
   }
}

/* ----------------------------------------------
 * Funcion: EnlazaParametro.
 * 
 *  Enlaza un parametro en la lista de parametros de un metodo. 
 *
 *  Parametros
 *     Entrada:
 *          parametro, parametro a enlazar.
 *     Entrada/Salida:
 *          p_lista, lista de parametros de entrada o salida del
 *                   metodo.
 * ---------------------------------------------------------------- */
static void  EnlazaParametro(struct S_Parametros ** p_lista,
		             struct S_Parametros * parametro)
{
  struct S_Parametros * aux;

  aux = *p_lista;

  if (aux == NULL)
   *p_lista = parametro;
  else
  {
    /* ---------------------------------
     *  Colocarlo al final, el orden es importante.
     * ------------------------------------------------- */
    while (aux->siguiente != NULL)
      aux = aux->siguiente;
    aux->siguiente = parametro;
  }
}

/* ----------------------------------------------
 *  Funcion: EscribeInterfaz.
 *
 *  Escribe el codigo del interfaz asociado a la descripcion
 * de los servicios remotos. 
 *
 *  Parametros
 *     Entrada:
 *          f_prototipo, fichero donde vamos a escribir el 
 *                 prototipado de la descripcion de metodos dada.
 * ---------------------------------------------------------------- */
static void EscribeInterfaz(FILE *f_prototipo)
{
  struct Clases * aux;
  struct S_EnlaceObjetos * metodos;
  struct S_Parametros * parametros;
  char var;

 /* ------------------------------------
  *   Recorremos las clases.
  * ------------------------------------- */
  aux = ListaClases;
  while (aux != NULL)
  {
   /* ------------------------------------
    *   Y para cada metodo dentro de estas. 
    * ------------------------------------- */
    metodos = aux->metodos;
    while (metodos != NULL)
    {
      /* ------------------------------------
       *   Escribimos el prototipo del metodo. 
       * ----------------------------------------------- */
      fprintf(f_prototipo,"#define ");
      fprintf(f_prototipo,"%s_",aux->Clase);
      fprintf(f_prototipo,"%s(Objeto",metodos->Metodo);

      var = 'A';
      /* ------------------------------------
       *   Con sus parametros de entrada. 
       * ----------------------------------------------- */
      parametros = metodos->p_entrada;
      while (parametros != NULL)
      {
        fprintf(f_prototipo,",");
        fprintf(f_prototipo,"%c1",var);
        var = (char)((int)var + 1);
        parametros = parametros->siguiente;
      }
      /* ------------------------------------
       *   Y sus parametros de entrada. 
       * ----------------------------------------------- */
      parametros = metodos->p_salida;
      while (parametros != NULL)
      {
        fprintf(f_prototipo,",");
        fprintf(f_prototipo,"%c2",var);
        var = (char)((int)var + 1);
        parametros = parametros->siguiente;
      }
      fprintf(f_prototipo,") \\\n");

      /* ------------------------------------
       *   Como el ejecutar una Operacion o Procedimiento. 
       * ---------------------------------------------------- */
      if (metodos->p_salida != NULL)
        fprintf(f_prototipo,"Emisor_PeticionProcedimiento(Objeto,");
      else
        fprintf(f_prototipo,"Emisor_PeticionOperacion(Objeto,");

      if (strcmp(aux->Clase,"" ) == 0)
        fprintf(f_prototipo,"\"\",");
      else
        fprintf(f_prototipo,"\"%s\",",aux->Clase);

      fprintf(f_prototipo,"\"%s\",",metodos->Metodo);
      fprintf(f_prototipo,"%d",metodos->version);

      /* ------------------------------------
       *   Con estos parametros de entrada. 
       * ----------------------------------------------- */
      var = 'A';
      parametros = metodos->p_entrada;
      while (parametros != NULL)
      {
        switch(parametros->tipo)
        {
           case T_CARACTER:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"c\",1,");
                   else
  		      fprintf(f_prototipo,",\"C\",%d,",parametros->lon);
  		   break;
	    case T_INT:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"i\",1,");
                   else
  		      fprintf(f_prototipo,",\"I\",%d,",parametros->lon);
  		   break;
            case T_STRING:
  		   fprintf(f_prototipo,",\"s\",1,");
		   break;
            case T_LONG:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"l\",1,");
                   else
  		      fprintf(f_prototipo,",\"L\",%d,",parametros->lon);
  		   break;
            case T_FLOAT:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"f\",1,");
                   else
  		      fprintf(f_prototipo,",\"F\",%d,",parametros->lon);
  		   break;
            case T_DOUBLE:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"d\",1,");
                   else
  		      fprintf(f_prototipo,",\"D\",%d,",parametros->lon);
  		   break;
         }
        fprintf(f_prototipo,"%c1",var);
        var = (char)((int)var + 1);
        parametros = parametros->siguiente;
      }
      /* ------------------------------------
       *   Y estos parametros de salida. 
       * ----------------------------------------------- */
      fprintf(f_prototipo,",\"\"");
      parametros = metodos->p_salida;
      while (parametros != NULL)
      {
        switch(parametros->tipo)
        {
           case T_CARACTER:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"c\",1,");
                   else
  		      fprintf(f_prototipo,",\"C\",%d,",parametros->lon);
		   break;
            case T_INT:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"i\",1,");
                   else
  		      fprintf(f_prototipo,",\"I\",%d,",parametros->lon);
  		   break;
            case T_STRING:
  		   fprintf(f_prototipo,",\"s\",1,");
  		   break;
            case T_LONG:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"l\",1,");
                   else
  		      fprintf(f_prototipo,",\"L\",%d,",parametros->lon);
  		   break;
            case T_FLOAT:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"f\",1,");
                   else
  		      fprintf(f_prototipo,",\"F\",%d,",parametros->lon);
  		   break;
            case T_DOUBLE:
                   if (parametros->lon == 1)
  		      fprintf(f_prototipo,",\"d\",1,");
                   else
  		      fprintf(f_prototipo,",\"D\",%d,",parametros->lon);
  		   break;
         }
        fprintf(f_prototipo,"%c2",var);
        var = (char)((int)var + 1);
	parametros = parametros->siguiente;
      }
      fprintf(f_prototipo,",\"\"");
      fprintf(f_prototipo,")\n");
      metodos = metodos->siguiente;
   }
   aux = aux->siguiente;
  }
}

/* ----------------------------------------------
 *  Funcion: EscribeVariablesFuente.
 *
 *  Escribe el codigo del fuente asociado a la descripcion
 * de los servicios remotos para las variables que vamos
 * a necesitar.
 *
 *   Parametros
 *     Entrada:
 *           f_fuente, fichero fuente a generar.
 * ---------------------------------------------------------------- */
static void EscribeVariablesFuente(FILE * f_fuente)
{
  struct Clases * aux;
  struct S_EnlaceObjetos * metodos;
  struct S_Parametros * parametros;
  char var;

  /* ----------------------------------------
   *    Para cada clase.
   * ------------------------------------------- */
  aux = ListaClases;
  while (aux != NULL)
  {
    /* ----------------------------------------
     *    Y para cada metodo de esta 
     * ------------------------------------------- */
    metodos = aux->metodos;
    while (metodos != NULL)
    {
     /* ----------------------------------------
      *    Crea las variables que vamos a necesitar para los parametros. 
      * --------------------------------------------------------------  */
      GeneraVariablesParametros(metodos,f_fuente);
     /* ----------------------------------------
      *    Y las estructuras que los van a contener. 
      *    a los de entrada.
      * ---------------------------------------------- */
      parametros = metodos->p_entrada;
      if (parametros != NULL)
      {
        fprintf(f_fuente,"static s_parametros ");
        fprintf(f_fuente,"%s_",aux->Clase);
        fprintf(f_fuente,"%s_pentrada[] = {",metodos->Metodo);

        var = 'A';
        fprintf(f_fuente,"{");
        while (parametros != NULL)
        {
	  if (var != 'A')
           fprintf(f_fuente,",{");

          var = (char)((int)var + 1);
          switch(parametros->tipo)
          {
             case T_CARACTER:
                   if (parametros->lon == 1)
  	  	     fprintf(f_fuente,"\"c\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"C\",%d,NULL}",parametros->lon);
	  	   break;
              case T_INT:
                   if (parametros->lon == 1)
    		     fprintf(f_fuente,"\"i\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"I\",%d,NULL}",parametros->lon);
    		   break;
              case T_STRING:
    		   fprintf(f_fuente,"\"s\",1,NULL}");
		   break;
              case T_LONG:
                   if (parametros->lon == 1)
  		     fprintf(f_fuente,"\"l\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"L\",%d,NULL}",parametros->lon);
    		   break;
              case T_FLOAT:
                   if (parametros->lon == 1)
  		     fprintf(f_fuente,"\"f\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"F\",%d,NULL}",parametros->lon);
    		   break;
              case T_DOUBLE:
                   if (parametros->lon == 1)
    		     fprintf(f_fuente,"\"d\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"D\",%d,NULL}",parametros->lon);
    		   break;
            }
    
          parametros = parametros->siguiente;
        }
        fprintf(f_fuente,",{\"\",1,NULL}};\n");
      }
     /* ----------------------------------------
      *    y a los de salida.
      * ---------------------------------------------- */
      parametros = metodos->p_salida;
      if (parametros != NULL)
      {
        fprintf(f_fuente,"static s_parametros ");
        fprintf(f_fuente,"%s_",aux->Clase);
        fprintf(f_fuente,"%s_psalida[] = {",metodos->Metodo);
        fprintf(f_fuente,"{");
        var = 'A';
	while (parametros != NULL)
        {
	  if (var != 'A')
           fprintf(f_fuente,",{");

          var = (char)((int)var + 1);

          switch(parametros->tipo)
          {
             case T_CARACTER:
                   if (parametros->lon == 1)
  	  	     fprintf(f_fuente,"\"c\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"C\",%d,NULL}",parametros->lon);
	  	   break;
              case T_INT:
                   if (parametros->lon == 1)
    		     fprintf(f_fuente,"\"i\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"I\",%d,NULL}",parametros->lon);
    		   break;
              case T_STRING:
    		   fprintf(f_fuente,"\"s\",1,NULL}");
    		   break;
              case T_LONG:
                   if (parametros->lon == 1)
  		     fprintf(f_fuente,"\"l\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"L\",%d,NULL}",parametros->lon);
    		   break;
              case T_FLOAT:
                   if (parametros->lon == 1)
  		     fprintf(f_fuente,"\"f\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"F\",%d,NULL}",parametros->lon);
    		   break;
              case T_DOUBLE:
                   if (parametros->lon == 1)
  		     fprintf(f_fuente,"\"d\",1,NULL}");
                   else
  	  	     fprintf(f_fuente,"\"D\",%d,NULL}",parametros->lon);
    		   break;
            }
          parametros = parametros->siguiente;
        }
        fprintf(f_fuente,",{\"\",1,NULL}};\n");
      }
      metodos = metodos->siguiente;
    }
  aux = aux->siguiente;
  }
}


/* ----------------------------------------------
 *  Funcion: GeneraVariablesParametros.
 *
 *  Genera las variables que vamos a necesitar, si realmente
 * son necesarias.
 * ---------------------------------------------------------------- */
static void GeneraVariablesParametros(struct S_EnlaceObjetos * metodos,
				      FILE * f_fuente)
{
  static int llevamos = 0;
  int tenemos = 0;
  static char v_llevamos = 'A';
  struct S_Parametros * aux;

  /* -----------------------------------------
   *  Calcula el numero de variables que requiere este metodo.
   * ----------------------------------------------------------- */
  aux = metodos->p_entrada;
  while (aux != NULL)
  {
    tenemos ++;
    aux = aux->siguiente;
  }
  aux = metodos->p_salida;
  while (aux != NULL)
  {
    tenemos ++;
    aux = aux->siguiente;
  }

  /* -----------------------------------------
   *  Si son mas de las que ya hemos descrito, generar una mas. 
   * ----------------------------------------------------------- */
  while (tenemos > llevamos)
  {
     fprintf(f_fuente,"static s_parametros * %c;\n",v_llevamos);
     v_llevamos = (char)((int)v_llevamos + 1);
     llevamos ++;
  }
}


/* ----------------------------------------------
 *  Funcion: EscribeCodigoEnlace.
 *
 *  Escribe el codigo de la estructura de enlace entre 
 * un metodo remoto y la funcion que lo atiende. 
 *
 *  Parametros
 *      Entrada:
 *         clase, descripcion de las clases para cuyos metodos
 *                hay que generar el codigo de enlace.
 *         f_fuente, fichero donde vamos a escribir este fuente.
 *
 * ---------------------------------------------------------------- */
static void EscribeCodigoEnlace(struct Clases * clases,FILE * f_fuente)
{
  struct Clases * aux;
  struct S_EnlaceObjetos * metodos;
  int primero = 1;

   aux = clases;

   /* -----------------------------------------------------
    *  Vamos a generar:
    *
    *   METODOS[] = {{"clase","metodo",parametros_entrada,para_salida},
    *                {"clase","metodo",parametros_entrada,para_salida},
    *               };
    * -------------------------------------------------------------- */
   fprintf(f_fuente,"\n\nstatic s_metodos METODOS[] = {\n"); 
   while (aux != NULL)
   {
     metodos = aux->metodos;
     while (metodos != NULL)
     {
        if (!primero)
          fprintf(f_fuente,",");
        else
          primero = 0;
        fprintf(f_fuente,"{\"%s_",aux->Clase);
	fprintf(f_fuente,"%s\",",metodos->Metodo);
        fprintf(f_fuente,"%d,",metodos->version);
        if (metodos->p_entrada == NULL)
          fprintf(f_fuente,"(s_parametros *)NULL,");
        else
        {
          fprintf(f_fuente,"(s_parametros *)&%s_",aux->Clase);
          fprintf(f_fuente,"%s_pentrada,",metodos->Metodo);
        }
        if (metodos->p_salida == NULL)
          fprintf(f_fuente,"(s_parametros *)NULL}");
        else
        {
          fprintf(f_fuente,"(s_parametros *)&%s_",aux->Clase);
          fprintf(f_fuente,"%s_psalida}\n",metodos->Metodo);
        }
       
        metodos = metodos->siguiente;
     }
     aux = aux->siguiente;
   }
   fprintf(f_fuente,"\n,{\"\",0,(s_parametros *)NULL,(s_parametros *)NULL}};\n");
}

/* ----------------------------------------------
 *  Funcion: EscribeCodigoFuncionEnlace
 *
 *     Escribe el codigo de la funcion que realiza el  
 *  trabajo de llamada al procedimiento a partir de un mensaje. 
 *
 *    Parametros
 *      Entrada:
 *          clase, lista de clase cuyo codigo de fusion voy a escribir.
 *          f_fuente, fichero fuente donde lo voy a escribir.
 * ---------------------------------------------------------------- */
static void EscribeCodigoFuncionEnlace(struct Clases * clase,FILE * f_fuente)
{
  struct Clases * aux;
  struct S_EnlaceObjetos * metodos;
  struct S_Parametros * parametros;
  int i = 0;
  int primero = 1;
  char var;
  char cadena[] = "\nvoid LlamaAFuncion(char * Clase, \
char * Metodo, int version, char * Mensaje,int logitud) \n\
{\n\n\
int i;\n\n\
  i=Receptor_BuscaMetodo(Clase,Metodo,METODOS);\n\
  if (i < 0) \n\
  {\n\
     Emisor_ErrorMetodo(Clase,Metodo,version,TEXTO_ERR_SIN_METODO);\n\
     return;\n\
  }\n\
  if (METODOS[i].version != version)\n \
  {\n\
    Emisor_ErrorMetodo(Clase,Metodo,version,TEXTO_ERR_VERSION);\n\
    return;\n\
  }\n\
  if (Decodifica_ParametrosMetodo(METODOS[i].entrada,Mensaje) < 0)\n \
  {\n\
    Emisor_ErrorMetodo(Clase,Metodo,version,TEXTO_ERR_DESAPLANA);\n\
    return;\n\
  }\n\n\
  switch(i)\n\
  {\n";

    fprintf(f_fuente,"%s",cadena);
    /* -----------------------------------------------
     * Una vez escrita la parte fija de busqueda dentro de la estructura
     * intermedia la que representa el metodo a ejecutar, crear las
     * llamadas al mismo con las variables descritar anteriormente
     * en la generacion de variables.
     *   
     * ---------------------------------------------------------------- */
    aux = clase;
    while (aux != NULL)
    {
       metodos = aux->metodos;
       while(metodos != NULL)
       {
          /* ------------------------------------------------
           *    A = par_entrada[0];
           *    B = par_entrada[1];
           *    C = par_salida[0];
           *    case Clase.Metodo:
           *             ClaseMetodo(A,B,&C);
           * ------------------------------------------------------- */
          var = 'A';
          fprintf(f_fuente,"case %d: \n",i);
          /* -----------------------------------------------
           *  Asigno las variables de entrada.
           * ---------------------------------------------- */
          parametros = metodos->p_entrada;
          while(parametros != NULL)
	  {
            if (var == 'A')
             fprintf(f_fuente,"   %c = METODOS[i].entrada;\n",var);
            else
             fprintf(f_fuente,"   %c = %c; \n   %c++;\n",
                                      var,(char)((int)var - 1),var);
            var = (char)((int)var +1);
            
            parametros = parametros->siguiente;
          }

          /* -----------------------------------------------
           *  Y de salida.
           * ---------------------------------------------- */
          parametros = metodos->p_salida;
          while(parametros != NULL)
          {
            if (primero)
             fprintf(f_fuente,"   %c = METODOS[i].salida;\n",var);
            else
             fprintf(f_fuente,"   %c = %c; \n   %c++;\n",
                                    var,(char)((int)var - 1),var);
            var = (char)((int)var +1);
            primero = 0;
            
	    parametros = parametros->siguiente;
          }
          primero = 1;

          fprintf(f_fuente,"  %s_%s(",aux->Clase,metodos->Metodo);
          var = 'A';
          /* -----------------------------------------------
           *  Y las utilizo en la llamada a la funcion 
           * ---------------------------------------------- */
          parametros = metodos->p_entrada;
          while(parametros != NULL)
          {
             if (!primero)
	       fprintf(f_fuente,",");

             /* --------------------------------------
              *  Poner el cast.
              * ------------------------------------------- */
             switch(parametros->tipo)
             {
                case T_CARACTER:
		     if (parametros->lon == 1)
     	  	       fprintf(f_fuente,"*(char *)(%c->valor)",var);
                     else
     	  	       fprintf(f_fuente,"(char *)(%c->valor)",var);
   	  	     break;
		case T_INT:
		     if (parametros->lon == 1)
  	  	       fprintf(f_fuente,"*(int *)(%c->valor)",var);
                     else
  	  	       fprintf(f_fuente,"(int *)(%c->valor)",var);
    		     break;
                case T_STRING:
  	  	     fprintf(f_fuente,"(char *)(%c->valor)",var);
    		     break;
                case T_LONG:
		     if (parametros->lon == 1)
  	  	       fprintf(f_fuente,"*(long *)(%c->valor)",var);
                     else
  	  	       fprintf(f_fuente,"(long *)(%c->valor)",var);
    		     break;
                case T_FLOAT:
		     if (parametros->lon == 1)
  	  	       fprintf(f_fuente,"*(float *)(%c->valor)",var);
                     else
  	  	       fprintf(f_fuente,"(float *)(%c->valor)",var);
    		     break;
		case T_DOUBLE:
		     if (parametros->lon == 1)
  	  	       fprintf(f_fuente,"*(double *)(%c->valor)",var);
                     else
  	  	       fprintf(f_fuente,"(double *)(%c->valor)",var);
    		     break;
             }


	     primero = 0;
	     parametros = parametros->siguiente;
	     var = (char)((int)var + 1);
	  }
	  if (metodos->p_salida != NULL)
	  {
          /* -----------------------------------------------
           *  Tambien en los parametros de salida con & 
           * ---------------------------------------------- */
	    parametros = metodos->p_salida;
	    while(parametros != NULL)
	    {
	       if (!primero)
                 fprintf(f_fuente,",");

             /* --------------------------------------
              *  Poner el cast.
              * ------------------------------------------- */
             switch(parametros->tipo)
             {
                case T_CARACTER:
     	  	     fprintf(f_fuente,"(char *)(%c->valor)",var);
   	  	     break;
                case T_INT:
  	  	     fprintf(f_fuente,"(int *)(%c->valor)",var);
    		     break;
                case T_STRING:
  	  	     fprintf(f_fuente,"(char **)(&%c->valor)",var);
    		     break;
                case T_LONG:
  	  	     fprintf(f_fuente,"(long *)(%c->valor)",var);
                     break;
                case T_FLOAT:
  	  	     fprintf(f_fuente,"(float *)(%c->valor)",var);
                     break;
                case T_DOUBLE:
  	  	     fprintf(f_fuente,"(double *)(%c->valor)",var);
                     break;
              }

               primero = 0;
               parametros = parametros->siguiente;
    	       var = (char)((int)var + 1);
            }
            fprintf(f_fuente,");\n");
            fprintf(f_fuente,
   /* -----------------------------------------------
    *  Si tiene parametros de salida, mandarlos a origen. 
    * --------------------------------------------------------- */
    "Emisor_RespuestaProcedimiento(Clase,Metodo,version,METODOS[i].salida);\n");
	  fprintf(f_fuente,"\nbreak;\n");
         }
         else
	   fprintf(f_fuente,");\nbreak;\n");
         i++;
         primero = 1;
         metodos = metodos->siguiente;
        }
     aux = aux->siguiente;
    }
    fprintf(f_fuente,"\n}\n}\n");
}

/* ----------------------------------------------
 *  Funcion: EscribePrototipos
 *
 *      Escribe el prototipo de las funciones de forma que 
 *  pueda funcionar como C.
 *
 * ---------------------------------------------------------------- */
static void EscribePrototipos(struct Clases * clase,FILE * f_fuente)
{
  struct Clases * aux;
  struct S_EnlaceObjetos * metodos;
  struct S_Parametros * parametros;
   aux = clase;

   fprintf(f_fuente,"\n");
   /* ---------------------------------
    *  Pa cada clase.
    * ----------------------------------- */
   while ( aux != NULL)
   {
     metodos = aux->metodos;
     /* ---------------------------------
      *  Pa cada metodo.
      * ----------------------------------- */
      while (metodos != NULL)
      {
          /* ----------------------------
           *  Cabecera funcion.
	   * ------------------------------ */ 
           fprintf(f_fuente,"void %s_%s(",aux->Clase,metodos->Metodo);
          /* ----------------------------
           *  Parametros de Entrada.
           * ----------------------------- */
           parametros = metodos->p_entrada;
           while(parametros != NULL)
           {
              switch(parametros->tipo)
              {
       		case T_CARACTER:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"char ");
                            else
                             fprintf(f_fuente,"char[%d]",parametros->lon);
                            break;
                        }
       		case T_INT:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"int ");
                            else
                             fprintf(f_fuente,"int[%d]",parametros->lon);
                            break;
                        }
       		case T_STRING:
			{
                             fprintf(f_fuente,"char * ");
                             break;
                        }
       		case T_LONG:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"long ");
                            else
                             fprintf(f_fuente,"long[%d]",parametros->lon);
                            break;
                        }
       		case T_FLOAT:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"float ");
                            else
                             fprintf(f_fuente,"float[%d]",parametros->lon);
                            break;
                        }
       		case T_DOUBLE:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"double ");
                            else
                             fprintf(f_fuente,"double[%d]",parametros->lon);
                            break;
                        }
              }
             if (parametros->siguiente != NULL || (metodos->p_salida != NULL))
                fprintf(f_fuente,",");
             parametros = parametros->siguiente;
           }
           parametros = metodos->p_salida;
           while(parametros != NULL)
           {
              switch(parametros->tipo)
              {
       		case T_CARACTER:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"char *");
                            else
                             fprintf(f_fuente,"char[%d]",parametros->lon);
                            break;
                        }
       		case T_INT:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"int * ");
                            else
                             fprintf(f_fuente,"int[%d]",parametros->lon);
                            break;
                        }
       		case T_STRING:
			{
                             fprintf(f_fuente,"char ** ");
                             break;
                        }
       		case T_LONG:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"long * ");
                            else
                             fprintf(f_fuente,"long[%d]",parametros->lon);
                            break;
                        }
       		case T_FLOAT:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"float *");
                            else
                             fprintf(f_fuente,"float[%d]",parametros->lon);
                            break;
                        }
       		case T_DOUBLE:
			{
                            if (parametros->lon == 1)
                             fprintf(f_fuente,"double * ");
                            else
                             fprintf(f_fuente,"double[%d]",parametros->lon);
                            break;
                        }
              }
             if (parametros->siguiente != NULL)
                fprintf(f_fuente,",");
             parametros = parametros->siguiente;
           }
           fprintf(f_fuente,");\n");
         
         
        metodos = metodos->siguiente;
      }
      aux = aux->siguiente;
    }
}

