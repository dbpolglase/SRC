#ifndef __TEXTOS
#define __TEXTOS
/* -----------------------------------------------------------------
 *   Definiciones de textos. 
 * ----------------------------------------------------------------- */
#define TEXTO_ERR_DESCONOCIDO     "RECIBIDO DE TIPO DESCONOCIDO"
#define TEXTO_ERR_PEDAZO_FUERA_SECUENCIA   \
                                  "RECIBIDO UN MENSAJE FUERA DE SECUENCIA"
#define TEXTO_ERR_CORREOS_COGER   "AL LEER UN PAQUETE UDP"
#define TEXTO_ERR_MALLOC          "AL PEDIR MEMORIA"
#define TEXTO_ERR_CODIFICA        "EN LA CODIFICACION" 
#define TEXTO_ERR_DECODIFICA      "EN LA DECODIFICACION" 
#define TEXTO_ERR_CORREOS_SELECT  "PROBLEMAS CON EL SELECT " 
#define TEXTO_ERR_CORREOS_MANDAR  "ESCRIBIR UN PAQUETE UDP"
#define TEXTO_ERR_BIND_SOCKET     "REGISTRAR EL SOCKET" 
#define TEXTO_ERR_DIR_MULTICAST   "ERROR EN DIRECCION MULTICAST"
#define TEXTO_ERR_CREAR_SOCKET    "CREAR EL SOCKET" 
#define TEXTO_ERR_DIRECCION       "OBTENER LA DIRECCION UDP (SERVICIO)"
#define TEXTO_ERR_RESPUESTA       "RESPUESTA NO ESPERADA"
#define TEXTO_ERR_TIMEOUT         "TIMEOUT EN PETICION SERVICIO " 
#define TEXTO_ERR_SIN_METODO      "NO METODO"
#define TEXTO_ERR_VERSION         "ERR VERSION"
#define TEXTO_ERR_DESAPLANA       "ERR PARAMETROS" 
#define TEXTO_ERR_ACK_TIMEOUT     "TIMEOUT EN LA ESPERA DE ACK "
#define TEXTO_ERR_BUZON           "NO EXISTE BUZON PARA EL DESCRIPTOR DADO "

/* ----------------------------------------------
 *  Definicion de errores.
 * ------------------------------------------------ */
#define ERR_DESCONOCIDO               0 
#define ERR_MALLOC                    1 
#define ERR_CODIFICA                  2 
#define ERR_DECODIFICA                3 
#define ERR_PEDAZO_FUERA_SECUENCIA    4
#define ERR_CORREOS_COGER             5 
#define ERR_CORREOS_SELECT            6
#define ERR_CORREOS_MANDAR            7
#define ERR_BIND_SOCKET               8
#define ERR_DIR_MULTICAST             9
#define ERR_CREAR_SOCKET             10
#define ERR_DIRECCION                11
#define ERR_RESPUESTA                12
#define ERR_TIMEOUT                  13
#define ERR_SIN_METODO               14
#define ERR_VERSION                  15
#define ERR_DESAPLANA                16
#define ERR_ACK_TIMEOUT              17
#define ERR_BUZON                    18

/* ----------------------------------------------
 *  Definicion de niveles.
 * ------------------------------------------------ */
#define ERROR_FISICO      1 
#define ERROR_ENLACE      2 
#define ERROR_RED         3 
#define ERROR_TRANSPORTE  4 
#define ERROR_SESION      5 
#endif /* __TEXTOS*/
