#ifndef __TIEMPO
#define __TIEMPO
/* ---------------------------------------------------------
 *  Interfaz del Modulo de control de TIME-OUTs.
 * --------------------------------------------------------- */
/* ------------------------
 * Indicar y obtener el tiempo especificado de espera para un servicio.
 * -------------------------------------------------------------------- */
int PutTimeoutService(int segundos);
int GetTimeoutService(int * segundos);
/* ------------------------
 * Indicar y obtener el tiempo especificado de espera para un servicio
 * que resta para que cumpla.
 * -------------------------------------------------------------------- */
int Put_TimeoutServicioPendiente(void);
int Get_TimeoutServicioPendiente(void);
int Reset_TimeoutServicioPendiente(void);
#endif /* __TIEMPO */
