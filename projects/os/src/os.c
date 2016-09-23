/*==================[inclusions]=============================================*/

#include "board.h"
#include "os.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

/** valor de retorno de excepción a cargar en el LR */
#define EXC_RETURN 0xFFFFFFF9

/** id de tarea inválida */
#define INVALID_TASK ((uint32_t)-1)

/** id de tarea idle */
#define IDLE_TASK (TASK_COUNT)

#define NULL_TASK -1

/** tamaño de stack de tarea idle */
#define STACK_IDLE_SIZE 256

/** How many priorities */
#define OSK_NRO_PRIORITIES 4

typedef struct {
    int *top;
    int *button;
    int queue[TASK_COUNT];
    int state;
}spriority;

/** estructura interna de control de tareas */
typedef struct taskControlBlock {
	uint32_t sp;
	const taskDefinition * tdef;
	taskState state;
	uint32_t waiting_time;
	uint32_t priority;
}taskControlBlock;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

__attribute__ ((weak)) void * idle_hook(void * p);

/*==================[internal data definition]===============================*/

/** indice a la tarea actual */
static uint32_t current_task = INVALID_TASK;

/** estructura interna de control de tareas */
static taskControlBlock task_control_list[TASK_COUNT];

/** contexto de la tarea idle */
uint8_t stack_idle[STACK_IDLE_SIZE];
static taskDefinition idle_tdef = {
		stack_idle, STACK_IDLE_SIZE, idle_hook, 0
};
static taskControlBlock idle_task_control;

/** Vector of struct priorities
 * OSK_NRO_PRIORITIES => array for the priority
 * TASK_COUNT => index the array of task
 *  */
static spriority osek_priority[OSK_NRO_PRIORITIES];

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/* si una tarea ejecuta return, vengo acá */
static void return_hook(void * returnValue)
{
	while(1);
}


/** Create queue for priority **/

void create_queue(spriority *squeue)
{
    squeue->top=&(squeue->queue[0]);
    squeue->button=&(squeue->queue[0]);
    squeue->state=0;
}

/** Add task to a queue **/
int add_task( spriority *squeue, int task)
{
    *(squeue->top)=task;
    squeue->state++;

    if( squeue->top == &(squeue->queue[TASK_COUNT-1])) {
        squeue->top = &(squeue->queue[0]);
    } else
        squeue->top++ ;
    return 0;
}

/** Remove task for the queue and return the next task **/
uint32_t remove_task(spriority *squeue )
{
	uint32_t osk_next_task;
    if( squeue->state == 0) {
        //printf("Queue is empty, I can't remove task");
    } else {
    	osk_next_task= *(squeue->button); // save next task.
        *(squeue->button)=NULL_TASK;
        squeue->state--;
        if(squeue->button == &(squeue->queue[TASK_COUNT-1])) {
            squeue->button=&(squeue->queue[0]);
        } else
            squeue->button++ ;
    }
    return osk_next_task;
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

void task_delay_update(void)
{
	uint32_t i;
	for (i=0; i<TASK_COUNT; i++) {
		if ( (task_control_list[i].state == TASK_STATE_WAITING) &&
				(task_control_list[i].waiting_time > 0)) {
			task_control_list[i].waiting_time--;
			if (task_control_list[i].waiting_time == 0) {
				task_control_list[i].state = TASK_STATE_READY; // Change new place
				add_task(&(osek_priority[task_control_list[i].priority]),i); //=> add to priority place
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

/* rutina de selección de próximo contexto a ejecutarse
 * acá definimos la política de scheduling de nuestro os
 */
int32_t getNextContext(int32_t current_context)
{
	uint32_t previous_task = current_task;
	uint32_t returned_stack;
	uint32_t i;
	static uint32_t pcount=0;

	/* guardo contexto actual si es necesario */
	if (current_task == IDLE_TASK) {
		idle_task_control.sp = current_context;
	}
	else if (current_task < TASK_COUNT) {
		task_control_list[current_task].sp = current_context;
	}

	pcount=0;
	for(i=0; i< OSK_NRO_PRIORITIES; i++){
		if (osek_priority[i].state > 0){
			current_task=remove_task(&(osek_priority[i]));
			break;
		}
		if ((OSK_NRO_PRIORITIES -1) == pcount){
			if(task_control_list[current_task].state == TASK_STATE_RUNNING){
				task_control_list[current_task].state = TASK_STATE_READY;
			}
		}
		pcount++;
	}

	/* si salí del while encontrando una tarea en estado READY... */
	if (task_control_list[current_task].state == TASK_STATE_READY) {
		task_control_list[current_task].state = TASK_STATE_RUNNING;
		returned_stack = task_control_list[current_task].sp;

		if (previous_task == IDLE_TASK) {
			idle_task_control.state = TASK_STATE_READY;
		}
		else if ((previous_task < TASK_COUNT)
				&& (task_control_list[previous_task].state == TASK_STATE_RUNNING)) {
			task_control_list[previous_task].state = TASK_STATE_READY;	// This not be necessary if all be ok
			add_task(&(osek_priority[task_control_list[previous_task].priority]),previous_task); // add task to queue
		}
	}
	else { /* si no hay tareas READY, ejecuto tarea idle */
		current_task = IDLE_TASK;
		idle_task_control.state = TASK_STATE_RUNNING;
		returned_stack = idle_task_control.sp;
	}
	return returned_stack;
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
	task_delay_update();
	schedule();
}

int start_os(void)
{
	uint32_t i;

	/* actualizo SystemCoreClock (CMSIS) */
	SystemCoreClockUpdate();

	/* inicializo contexto idle */
	idle_task_control.tdef = &idle_tdef;
	task_create(idle_task_control.tdef->stack,
			idle_task_control.tdef->stack_size,
			&(idle_task_control.sp),
			idle_task_control.tdef->entry_point,
			idle_task_control.tdef->parameter,
			&(idle_task_control.state));

	/*Init index priority vector pointing to priority array  */
	for(i=0; i< OSK_NRO_PRIORITIES; i++){
		create_queue(&(osek_priority[i]));
	}

	/* inicializo contextos iniciales de cada tarea */
	for (i=0; i<TASK_COUNT; i++) {
		task_control_list[i].tdef = task_list+i;

		task_create(task_control_list[i].tdef->stack,
				task_control_list[i].tdef->stack_size,
				&(task_control_list[i].sp),
				task_control_list[i].tdef->entry_point,
				task_control_list[i].tdef->parameter,
				&(task_control_list[i].state));

		add_task(&(osek_priority[task_list[i].priority]),i);
	}

	/* configuro PendSV con la prioridad más baja */
	NVIC_SetPriority(PendSV_IRQn, 255);
	SysTick_Config(SystemCoreClock / 1000);

	/* llamo al scheduler */
	schedule();

	return -1;
}

void * idle_hook(void * p)
{
	while (1) {
		__WFI();
	}
}





/*==================[end of file]============================================*/
