	.syntax unified
	.global PendSV_Handler

	/* declaro acceso a la rutina de seleccion de proximo contexto */
	.extern getNextContext

	.text
	.thumb_func
PendSV_Handler:
	/* guardamos el resto del contexto */
	push {r4-r11,lr}

	/* llamo a getNextContext pasándole como parámetro al
	contexto actual, esto equivale a hacer:
	nextContext = getNextContext(actualContext);

	Por convención (AAPCS), paso el parámetro en R0 y devuelvo
	el valor en R0.
	*/


	tst lr,0x10
	it eq
	vpusheq {s16-s31}
	push {lr}

	mrs r0, msp /* r0 = msp */
	bl getNextContext /* r0 = siguiente puntero de pila
	uso la instrucción bl (branch with link) */

	tst lr,0x10
	it eq
	vpopeq {s16-s31}
//	push {lr}

	msr msp, r0 /* msp = r0 */

	pop {r4-r11,lr}

	bx lr /* retorno de interrupción,
deberá recuperar el contexto armado en task_create */
