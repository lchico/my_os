/*==================[inclusions]=============================================*/

#include "os.h"
#include "board.h"

/*==================[macros and definitions]=================================*/

/** tamaño de pila para los threads */
#define STACK_SIZE 512



/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void * tarea1(void * param);
static void * tarea2(void * param);
static void * tarea3(void * param);
static void * tarea4(void * param);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/* pilas de cada tarea */
uint8_t stack1[STACK_SIZE];
uint8_t stack2[STACK_SIZE];
uint8_t stack4[STACK_SIZE];
uint8_t stack3[STACK_SIZE];


const taskDefinition task_list[TASK_COUNT] = {
		{stack1, STACK_SIZE, tarea1, (void *)0xAAAAAAAA , 1 },
		{stack2, STACK_SIZE, tarea2, (void *)0xBBBBBBBB , 2 },
		{stack3, STACK_SIZE, tarea3, (void *)0xCCCCCCCC , 3 },
		{stack4, STACK_SIZE, tarea4, (void *)0xDDDDDDDD , 3 }
};

/*==================[internal functions definition]==========================*/

static void * tarea1(void * param)
{
	int i,j;
	float t=1.1;
	while (t) {
		t*=1.1;
		Board_LED_Toggle(0);
//		for(i=0;i<1000000;i++) j=i;
		delay(1000);
	}
	return (void *)0; /* a dónde va? */
}

static void * tarea2(void * param)
{
	int j=4,i;
	float t=1;
	while (t) {
		t*=1.1;
		Board_LED_Toggle(3);
//		for(i=0;i<1000000;i++) j=i;
		t*=1.1;
		delay(1000);
	}
	return (void *)4; /* a dónde va? */
}

static void * tarea3(void * param)
{
	int i,j;
	float t=1.1;
	while (t) {
		t*=1.1;
		Board_LED_Toggle(4);
		for(i=0;i<1000000;i++) j=i;
		delay(1000);
	}
	return (void *)0; /* a dónde va? */
}

static void * tarea4(void * param)
{
	int j=4,i;
	float t=1;
	while (t) {
		t*=1.1;
		Board_LED_Toggle(5);
		for(i=0;i<1000000;i++) j=i;
		//delay(1000);
	}
	return (void *)4; /* a dónde va? */
}
/*==================[external functions definition]==========================*/

int main(void)
{
	/* Inicialización del MCU */
	Board_Init();

	/* Inicio OS */
	start_os();

	/* no deberíamos volver acá */
	while(1);
}

/*==================[end of file]============================================*/
