#ifndef __RECEP
#define __RECEP
/* --------------------------------------------------------------------
 *  MODULO: receptor.
 *
 *  Interfaz del modulo de recepcion de peticiones de mensajes.
 *  Como ademas hay un fuente generado, tenemos que exportar algunas
 * estructuras y funciones que deberian ser privadas.
 * --------------------------------------------------------------------- */

typedef struct{
		char tipo[2];
		unsigned int  lon;
		void * valor;
		}s_parametros;

typedef struct {
		char metodo[100];
		int  version;
		s_parametros * entrada;
		s_parametros * salida;
		}s_metodos;

struct s_PETICIONES{
 		   struct sockaddr_in origen;
                   char tipo;
		   int  buzon;
		   char * mensaje;
		   char * recibido;
		   int  tamano;
		   struct s_PETICIONES * siguiente;
		   };

typedef struct s_PETICIONES s_peticiones;
		   
int Receptor_Despachar(int buzon, struct sockaddr_in origen, 
		       char * peticion, int longitud);
int Receptor_BuscaMetodo(char * clase, char * metodo, s_metodos METODOS[]);
int Receptor_Actual(s_peticiones ** actual);
int Receptor_AtiendePendientes();
int Receptor_LiberaActual();
void LlamaAFuncion(char * Clase, char * Metodo, int version, char * Mensaje,int logitud); 
#endif /* __RECEP */
