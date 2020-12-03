#include "cola_asm.h"

.global cola_crear
.global cola_destruir
.global cola_encolar
.global cola_desencolar
.global cola_esta_vacia
.global cola_ver_primero


cola_crear:                /* Se recomienda usar calloc() */
  ret


cola_esta_vacia:           /* Se permite un conditional move (cmovXX/setXX) */
  ret


cola_ver_primero:          /* Se permite un salto condicional */
  ret


cola_encolar:              /* Se permite un conditional move */
  ret


cola_desencolar:           /* Se permite dos saltos condicionales + un cmov */
  ret


cola_destruir:             /* Sin restricciones */
  ret
