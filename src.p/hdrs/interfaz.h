/* <MESA:01:@(#):MwillyfrSB0g:pds:1.1:021028195841:willy:1 35 825:MESA> */
/* 
 *  Descripcion de los parametros.
 */
struct S_Parametros{
		      int tipo;
		      int lon;
		      struct S_Parametros * siguiente;
		    };

/*
 *  Descripcion de los metodos.
 */

struct S_EnlaceObjetos{
	    char Metodo[100];
	    int  version;
	    struct S_Parametros * p_entrada;   /* Parametros de Entrada */
	    struct S_Parametros * p_salida;   /* Parametros de Salida */
	    struct S_EnlaceObjetos * siguiente;
	  };

/*
 *  Descripcion de los objetos.
 */

struct Clases{
		char Clase[100];
		struct S_EnlaceObjetos * metodos;
		struct Clases * siguiente;
	       };

