/* ---------------------------------------------------------
 *  Modulo de control de TIME-OUTs de las llamadas 
 * remotas a servicios.
 * --------------------------------------------------------- */
#include <time.h>
#include <tiempo.h>

#define DEFECTO_TIMEOUT 5   /* 5 sg. */
static int segundos_timeout = DEFECTO_TIMEOUT;
static time_t t_peticion;
static int t_resta;

/* ------------------------
 * Funcion: PutTimeoutService.
 *
 * Indica el nro. de segundos que deben trascurrir entre la 
 * solicitud de un servicio y la no llegada de la respuesta para
 * que se considere TIMEOUT.
 * 
 * Parametros:
 *  Entrada:
 *    segundos, nro. de segundos que deben transcurrir.
 *  Retorna:
 *   -1, si el timeout es menor que 0.
 * -------------------------------------------------------------------- */
int PutTimeoutService(int segundos)
{
  if (segundos < 0)
   return -1;
  segundos_timeout = segundos;
  return 0;
}

/* ------------------------
 * Funcion: GetTimeoutServicio.
 *
 * Obtener el nro. de segundos que deben trascurrir entre la 
 * solicitud de un servicio y la no llegada de la respuesta para
 * que se considere TIMEOUT.
 * 
 * Parametros:
 *  Salida:
 *    segundos, nro. de segundos que deben transcurrir.
 * -------------------------------------------------------------------- */
int GetTimeoutService(int * segundos)
{
  *segundos = segundos_timeout;
  return 0;
}

/* ------------------------
 * Funcion: Get_TimeoutServicioPendiente.
 *
 * Obtener el nro. de segundos que restan para que se cumpla el plazo 
 * de espera de respuesta. 
 * 
 * Parametros:
 *  Salida:
 *    segundos, nro. de segundos que restan.
 * -------------------------------------------------------------------- */
int Get_TimeoutServicioPendiente()
{
  return t_resta;
}

/* ------------------------
 * Funcion: Put_TimeoutServicioPendiente.
 *
 * Indica que se recalcule el tiempo de espera que hay en este momento. 
 * 
 * Parametros:
 *  Entrada:
 *    segundos, nro. de segundos que restan.
 *  Retorna:
 *    nro. de segundos que restan. 
 * -------------------------------------------------------------------- */
int Put_TimeoutServicioPendiente(void)
{
   t_resta = segundos_timeout - (time(0) - t_peticion);
   return t_resta;
}

/* ------------------------
 * Funcion: Reset_TimeoutServicioPendiente.
 *
 * Pone el tiempo de espera que resta al time out del servicio.
 * 
 * Parametros:
 * -------------------------------------------------------------------- */
int Reset_TimeoutServicioPendiente(void)
{
   t_peticion = time(0);
   t_resta = segundos_timeout; 
   return t_resta;
}

