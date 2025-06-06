/* <MESA:01:@(#):MwillyfrSB20:pds:1.1:021028195848:willy:1 35 53048:MESA> */
/* --------------------------------------------------------------
 *   Modulo general del analizador y constructor del generador
 * de codigo para SRC.
 * -------------------------------------------------------------- */
#define TRUE  1
#define FALSE 0


/* -------------------------------------------------------------
 *  Estructura que describe los tokens.
 * ------------------------------------------------------------- */
struct Token{
               int tipo;                  /* token encontrado */
               int linea;                 /* linea en que se encuentra */
               char * texto;              /* texto asociado al token.*/
               struct Token * siguiente;  /* siguiente token. */
               struct Token * anterior;   /* token anterior. */
            };

typedef struct Token s_token;

/* -------------------------------------------------------------
 *  Tokens a reconocer 
 * ------------------------------------------------------------- */
#define T_TEXTO        0      /* abc... */
#define T_CLASE        1      /* clase */
#define T_IN	       3      /* in */
#define T_OUT          4      /* out */
#define T_IGUALDAD     5      /* = */
#define T_COMENTARIO   6      /* // */
#define T_CARACTER     7      /* char */
#define T_INT          8      /* int */
#define T_STRING       9      /* char[] */
#define T_LONG         10     /* long */
#define T_CONSTANTE    11     /* constat */
#define T_FIN_CLASE    12     /* } */
#define T_FIN_METODO   13     /* ; */
#define T_P_PARAMETROS 14     /* :: */
#define T_S_PARAMETROS 15     /* , */
#define T_P_CLASE      16     /* { */
#define T_METODO       17     /*    ------ Auxiliar */
#define T_VERSION      18     /*    ------ Auxiliar */
#define T_FLOAT        19     /* float */
#define T_DOUBLE       20     /* double */
#define T_P_ARRAY      21     /* < */
#define T_FIN_ARRAY    22     /* > */


/* -----------------------------------
 *  TAMANNOS MAXIMOS.
 * ------------------------------------- */
/* Maximo texto que se puede digerir del fichero de descripcion. */
#define MAX_TEXTO 100 

#define MAX_SUBCLASE MAX_TEXTO 
#define MAX_CLASE    MAX_TEXTO

/* -----------------------------------
 *  TIPOS DE MENSAJES A TRADUCIR. 
 * ------------------------------------- */
#define SINCRONO  0
#define ASINCRONO 1


