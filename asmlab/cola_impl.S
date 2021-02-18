#include "cola_asm.h"

.global cola_crear
.global cola_destruir
.global cola_encolar
.global cola_desencolar
.global cola_esta_vacia
.global cola_ver_primero


cola_crear:                /* Se recomienda usar calloc() */
	movq	$1, %rdi
	movq	$SIZEOF_COLA, %rsi
	call	calloc
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
	pushq	%rbp
	pushq	%r12
	movq	%rdi, %rbp
	movq	%rsi, %r12
	movq	$1, %rdi
	movq	$SIZEOF_NODO, %rsi
	call	calloc
	pushq	%rbx
	movq	%rax, %rbx
	movq	%r12, OFFSET_DATA(%rbx)      /* nodo->data = valor */
	movq	%rbp, %rdi
	call	cola_esta_vacia
	testb	%al, %al
	jne	.esta_vacia
	movq	OFFSET_TAIL(%rbp), %rdx
	movq	%rbx, OFFSET_NEXT(%rdx)      /* cola->tail->next = nodo */
	movq	%rbx, OFFSET_TAIL(%rbp)      /* cola->tail = nodo */
	jmp	.devolver_bool
.esta_vacia:
	movq	%rbx, OFFSET_HEAD(%rbp)      /* cola->head = temp */
	movq	%rbx, OFFSET_TAIL(%rbp)      /* cola->tail = nodo */
.devolver_bool:
	cmpq	$0, %rbx      /* Devuelve falso si el nodo es NULL */
	setne	%al
	popq	%rbx
	popq	%r12
	popq	%rbp
	ret


cola_desencolar:           /* Se permite dos saltos condicionales + un cmov */
	movq	%rdi, %rcx
	call	cola_esta_vacia
	testb	%al, %al
	jne	.vacia
	pushq	%rbx
	movq	OFFSET_HEAD(%rcx), %rdi 	/* nodo */
	movq	OFFSET_DATA(%rdi), %rbx	/* data */
	movq	OFFSET_NEXT(%rdi), %rax	/* next -> actual */
 	movq	%rax, OFFSET_HEAD(%rcx)
	call	free			/* free nodo */
	movq	%rbx, %rax		/* devuelve data */
	popq	%rbx
	ret
.vacia:
	movq	$0, %rax
	ret


cola_destruir:             /* Sin restricciones */
	pushq	%rbx
	pushq	%rbp
	pushq	%r12
	movq	%rdi, %r12
	movq	OFFSET_HEAD(%rdi), %rbx
.loop:
	cmpq	$0, %rbx
	je	.salirLoop
	movq	OFFSET_NEXT(%rbx), %rbp
	movq	%rbx, %rdi
	call	free
	movq	%rbp, %rbx
	cmpq	$0, %rbx
	jne	.loop
.salirLoop:
	movq	%r12, %rdi
	call	free
	popq	%r12
	popq	%rbp
	popq	%rbx
	ret

