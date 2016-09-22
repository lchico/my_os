#ifndef _OS_H_
#define _OS_H_

/*==================[inclusions]=============================================*/

#include "board.h"
<<<<<<< HEAD
=======
#include "os_config.h"
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/** prototipo de las tareas o hilos de ejecucion o threads */
typedef void* (*entry_point_t)(void *);

/** posibles estados de una tarea */
typedef enum taskState {
	TASK_STATE_ERROR,
	TASK_STATE_READY,
	TASK_STATE_RUNNING,
	TASK_STATE_WAITING,
	TASK_STATE_ZOMBIE,
	TASK_STATE_TERMINATED
}taskState;

<<<<<<< HEAD
/** estructura de control y estado de tareas */
typedef struct taskControlBlock {
	uint32_t sp;
=======
/** estructura de definicón de tareas */
typedef struct taskDefinition {
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712
	uint8_t * stack;
	uint32_t stack_size;
	entry_point_t entry_point;
	void  * parameter;
<<<<<<< HEAD
	taskState state;
}taskControlBlock;

/*==================[external data declaration]==============================*/

extern taskControlBlock task_list[];
extern const size_t task_count;
=======
}taskDefinition;

/*==================[external data declaration]==============================*/

extern const taskDefinition task_list[TASK_COUNT];
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712

/*==================[external functions declaration]=========================*/

int start_os(void);
void schedule(void);
<<<<<<< HEAD
=======
void delay(uint32_t milliseconds);
>>>>>>> 24b17501cb18bf112c4810e46a2bdd8485dfc712

/*==================[end of file]============================================*/
#endif /* #ifndef _OS_H_ */
