#include "cola_asm.h"

.global cola_crear
.global cola_destruir
.global cola_encolar
.global cola_desencolar
.global cola_esta_vacia
.global cola_ver_primero


cola_crear:                /* Se recomienda usar calloc() */
	movq	$32, %rdi
	call	malloc
  ret


cola_esta_vacia:           /* Se permite un conditional move (cmovXX/setXX) */
  cmpq $0, OFFSET_HEAD(%rdi)
  sete %al
  ret


cola_ver_primero:          /* Se permite un salto condicional */
  xorq %rax, %rax
  movq OFFSET_HEAD(%rdi), %rcx
  cmpq %rax, %rcx          
  je .salir
  movq OFFSET_DATA(%rcx), %rax
.salir:
  ret


cola_encolar:              /* Se permite un conditional move */
  ret


cola_desencolar:           /* Se permite dos saltos condicionales + un cmov */
	movq	%rdi, %rcx
	call	cola_esta_vacia
	testb	%al, %al
	jne	.vacia
	pushq	%rbp
	movq	(%rcx), %rdi 	/* nodo */
	movq	(%rdi), %rbp	/* data */
	movq	8(%rdi), %rax	/* next -> actual */
 	movq	%rax, (%rcx)
	call	free			    /* free nodo */
	movq	%rbp, %rax		/* devuelve data */
	popq	%rbp
	ret
.vacia:
	movq	$0, %rax
	ret



cola_destruir:             /* Sin restricciones */
	movq %rdi,%rcx
.loop:
	call cola_esta_vacia
	testb %al,%al
	jne .salirLoop
	movq %rcx,%rdi
	call cola_desencolar
	testb %al,%al
	jne .loop
.salirLoop:
	call free
	ret
