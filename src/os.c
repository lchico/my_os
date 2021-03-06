/*==================[inclusions]=============================================*/

#include "board.h"
#include "os.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

/** valor de retorno de excepción a cargar en el LR */
#define EXC_RETURN 0xFFFFFFF9

<<<<<<< HEAD
#define INVALID_TASK ((uint32_t)-1)

=======
/** id de tarea inválida */
#define INVALID_TASK ((uint32_t)-1)

/** id de tarea idle */
#define IDLE_TASK (TASK_COUNT)

/** tamaño de stack de tarea idle */
#define STACK_IDLE_SIZE 256

/** estructura interna de control de tareas */
typedef struct taskControlBlock {
	uint32_t sp;
	const taskDefinition * tdef;
	taskState state;
	uint32_t waiting_time;
}taskControlBlock;

>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

<<<<<<< HEAD
=======
__attribute__ ((weak)) void * idle_hook(void * p);

>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
/*==================[internal data definition]===============================*/

/** indice a la tarea actual */
static uint32_t current_task = INVALID_TASK;

<<<<<<< HEAD
=======
/** estructura interna de control de tareas */
static taskControlBlock task_control_list[TASK_COUNT];

/** contexto de la tarea idle */
uint8_t stack_idle[STACK_IDLE_SIZE];
static taskDefinition idle_tdef = {
		stack_idle, STACK_IDLE_SIZE, idle_hook, 0
};
static taskControlBlock idle_task_control;

>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/* si una tarea ejecuta return, vengo acá */
static void return_hook(void * returnValue)
{
	while(1);
}

/* task_create sirve para crear un contexto inicial */
static void task_create(
		uint8_t * stack_frame, /* vector de pila (frame) */
		uint32_t stack_frame_size, /* el tamaño expresado en bytes */
		uint32_t * stack_pointer, /* donde guardar el puntero de pila */
		entry_point_t entry_point, /* punto de entrada de la tarea */
		void * parameter, /* parametro de la tarea */
		taskState * state)
{
	uint32_t * stack = (uint32_t *)stack_frame;

	/* inicializo el frame en cero */
	bzero(stack, stack_frame_size);

	/* último elemento del contexto inicial: xPSR
	 * necesita el bit 24 (T, modo Thumb) en 1
	 */
	stack[stack_frame_size/4 - 1] = 1<<24;

	/* anteúltimo elemento: PC (entry point) */
	stack[stack_frame_size/4 - 2] = (uint32_t)entry_point;

	/* penúltimo elemento: LR (return hook) */
	stack[stack_frame_size/4 - 3] = (uint32_t)return_hook;

	/* elemento -8: R0 (parámetro) */
	stack[stack_frame_size/4 - 8] = (uint32_t)parameter;

	stack[stack_frame_size/4 - 9] = EXC_RETURN;

	/* inicializo stack pointer inicial */
	*stack_pointer = (uint32_t)&(stack[stack_frame_size/4 - 17]);

	/* seteo estado inicial READY */
	*state = TASK_STATE_READY;
}

<<<<<<< HEAD
/*==================[external functions definition]==========================*/

=======
void task_delay_update(void)
{
	uint32_t i;
	for (i=0; i<TASK_COUNT; i++) {
		if ( (task_control_list[i].state == TASK_STATE_WAITING) &&
				(task_control_list[i].waiting_time > 0)) {
			task_control_list[i].waiting_time--;
			if (task_control_list[i].waiting_time == 0) {
				task_control_list[i].state = TASK_STATE_READY;
			}
		}
	}
}

/*==================[external functions definition]==========================*/

void delay(uint32_t milliseconds)
{
	if (current_task != INVALID_TASK) {
		task_control_list[current_task].state = TASK_STATE_WAITING;
		task_control_list[current_task].waiting_time = milliseconds;
		schedule();
	}
}

>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
/* rutina de selección de próximo contexto a ejecutarse
 * acá definimos la política de scheduling de nuestro os
 */
int32_t getNextContext(int32_t current_context)
{
<<<<<<< HEAD
	if (current_task == INVALID_TASK) {
		current_task = 0;
	}
	else if(current_task == 0) {
		task_list[current_task].sp = current_context;
		current_task = 1;
	}
	else if (current_task == 1) {
		task_list[current_task].sp = current_context;
		current_task = 0;
	}
	else {
		while(1); /* error!!! */
	}
	return task_list[current_task].sp;
=======
	uint32_t previous_task = current_task;
	uint32_t returned_stack;
	taskState state;

	/* guardo contexto actual si es necesario */
	if (current_task == IDLE_TASK) {
		idle_task_control.sp = current_context;
	}
	else if (current_task < TASK_COUNT) {
		task_control_list[current_task].sp = current_context;
	}

	/* decido cuál va a ser el contexto siguiente a ejecutar */
	do {
		current_task++;
		if (current_task > IDLE_TASK) {
			current_task = 0;
		}
		if (current_task == IDLE_TASK) {
			state = idle_task_control.state;
		}
		else {
			state = task_control_list[current_task].state;
		}
	} while ((state != TASK_STATE_READY)
			&& (previous_task != current_task)
			&& (previous_task != INVALID_TASK));

	/* si salí del while encontrando una tarea en estado READY... */
	if (task_control_list[current_task].state == TASK_STATE_READY) {
		task_control_list[current_task].state = TASK_STATE_RUNNING;
		returned_stack = task_control_list[current_task].sp;

		if (previous_task == IDLE_TASK) {
			idle_task_control.state = TASK_STATE_READY;
		}
		else if ((previous_task < TASK_COUNT)
				&& (task_control_list[previous_task].state == TASK_STATE_RUNNING)) {
			task_control_list[previous_task].state = TASK_STATE_READY;
		}
	}
	else { /* si no hay tareas READY, ejecuto tarea idle */
		current_task = IDLE_TASK;
		idle_task_control.state = TASK_STATE_RUNNING;
		returned_stack = idle_task_control.sp;
	}
	return returned_stack;
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
}

void schedule(void)
{
	/* activo PendSV para llevar a cabo el cambio de contexto */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/* Instruction Synchronization Barrier: aseguramos que se
	 * ejecuten todas las instrucciones en  el pipeline
	 */
	__ISB();
	/* Data Synchronization Barrier: aseguramos que se
	 * completen todos los accesos a memoria
	 */
	__DSB();
}

void SysTick_Handler(void)
{
<<<<<<< HEAD
=======
	task_delay_update();
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
	schedule();
}

int start_os(void)
{
	uint32_t i;

	/* actualizo SystemCoreClock (CMSIS) */
	SystemCoreClockUpdate();

<<<<<<< HEAD
	/* inicializo contextos iniciales de cada tarea */
	for (i=0; i<task_count; i++) {
		task_create(task_list[i].stack, task_list[i].stack_size,
				&(task_list[i].sp), task_list[i].entry_point,
				task_list[i].parameter, &(task_list[i].state));
=======
	/* inicializo contexto idle */
	idle_task_control.tdef = &idle_tdef;
	task_create(idle_task_control.tdef->stack,
			idle_task_control.tdef->stack_size,
			&(idle_task_control.sp),
			idle_task_control.tdef->entry_point,
			idle_task_control.tdef->parameter,
			&(idle_task_control.state));

	/* inicializo contextos iniciales de cada tarea */
	for (i=0; i<TASK_COUNT; i++) {
		task_control_list[i].tdef = task_list+i;

		task_create(task_control_list[i].tdef->stack,
				task_control_list[i].tdef->stack_size,
				&(task_control_list[i].sp),
				task_control_list[i].tdef->entry_point,
				task_control_list[i].tdef->parameter,
				&(task_control_list[i].state));
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
	}

	/* configuro PendSV con la prioridad más baja */
	NVIC_SetPriority(PendSV_IRQn, 255);
	SysTick_Config(SystemCoreClock / 1000);

	/* llamo al scheduler */
	schedule();

	return -1;
}

<<<<<<< HEAD
=======
void * idle_hook(void * p)
{
	while (1) {
		__WFI();
	}
}

>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
/*==================[end of file]============================================*/
