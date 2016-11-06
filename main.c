#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>

#define P1AL	GPIO_Pin_0
#define P1AH	GPIO_Pin_1
#define P1BL	GPIO_Pin_2
#define P1BH	GPIO_Pin_3
#define P2AL	GPIO_Pin_4
#define P2AH	GPIO_Pin_5
#define P2BL	GPIO_Pin_6
#define P2BH	GPIO_Pin_7
#define PGPIO	GPIOA

//Define step mode, 8 steps provides less audible noise at half the rotation speed with higher resolution
//8 Step mode is also known as half step mode and 4 step mode is also known as full step mode
#define STEPS	8

GPIO_InitTypeDef G;

//Output defined types
typedef enum{
	HIGH, Z, LOW
} OutputTypes;

//Output phases
typedef enum{
	PH1A, PH1B, PH2A, PH2B
} PhaseTypes;

//Timekeeping
volatile uint32_t msec = 0;

void SysTick_Handler(void){
	msec++;
}

void Delay(uint32_t t){
	uint32_t mss = msec;
	while((msec-mss)<t);
}

//Set phase outputs
void SetPhase(PhaseTypes phase, OutputTypes out){
	uint8_t h, l;

	//First, reset phase output minimally to stop cross conduction
	switch(phase){
	case PH1A:
		GPIO_WriteBit(PGPIO, P1AH, 1);
		GPIO_WriteBit(PGPIO, P1AL, 0);
		break;
	case PH1B:
		GPIO_WriteBit(PGPIO, P1BH, 1);
		GPIO_WriteBit(PGPIO, P1BL, 0);
		break;
	case PH2A:
		GPIO_WriteBit(PGPIO, P2AH, 1);
		GPIO_WriteBit(PGPIO, P2AL, 0);
		break;
	case PH2B:
		GPIO_WriteBit(PGPIO, P2BH, 1);
		GPIO_WriteBit(PGPIO, P2BL, 0);
		break;
	}

	switch(out){
	case HIGH:
		h = 0;
		l = 0;
		break;
	case Z:
		h = 1;
		l = 0;
		break;
	case LOW:
		h = 1;
		l = 1;
		break;
	}

	switch(phase){
	case PH1A:
		GPIO_WriteBit(PGPIO, P1AH, h);
		GPIO_WriteBit(PGPIO, P1AL, l);
		break;
	case PH1B:
		GPIO_WriteBit(PGPIO, P1BH, h);
		GPIO_WriteBit(PGPIO, P1BL, l);
		break;
	case PH2A:
		GPIO_WriteBit(PGPIO, P2AH, h);
		GPIO_WriteBit(PGPIO, P2AL, l);
		break;
	case PH2B:
		GPIO_WriteBit(PGPIO, P2BH, h);
		GPIO_WriteBit(PGPIO, P2BL, l);
		break;
	}
}

int main(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	//Initialize output pins
	G.GPIO_Pin = P1AL|P1AH|P1BL|P1BH|P2AL|P2AH|P2BL|P2BH;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(PGPIO, &G);


#if STEPS == 8
	//						 +     +     +     Z     -     -     -     Z
	const OutputTypes a[] = {HIGH, HIGH, HIGH, HIGH, LOW,  LOW,  LOW,  HIGH};
	const OutputTypes b[] = {LOW,  LOW,  LOW,  HIGH, HIGH, HIGH, HIGH, HIGH};

	//						 +     Z     -     -     -     Z     +     +
	const OutputTypes c[] = {HIGH, HIGH, LOW,  LOW,  LOW,  HIGH, HIGH, HIGH};
	const OutputTypes d[] = {LOW,  HIGH, HIGH, HIGH, HIGH, HIGH, LOW,  LOW};
#else

	//						 +     -     -     +
	const OutputTypes a[] = {HIGH, LOW,  LOW,  HIGH};
	const OutputTypes b[] = {LOW,  HIGH, HIGH, LOW};

	//						 -	   -     +     +
	const OutputTypes c[] = {LOW,  LOW,  HIGH, HIGH};
	const OutputTypes d[] = {HIGH, HIGH, LOW,  LOW};
#endif

	SysTick_Config(SystemCoreClock/1000);

	//Time delay is inversely proportional to speed
	const uint32_t speed = 1;
	uint8_t pos = 0;

	while(1)
	{
		SetPhase(PH1A, a[pos]);
		SetPhase(PH1B, c[pos]);
		SetPhase(PH2A, b[pos]);
		SetPhase(PH2B, d[pos]);


		pos++;

#if STEPS == 8
		if(pos == 8) pos = 0;
#else
		if(pos == 4) pos = 0;
#endif

		//In 8 step mode, RPM = 60*1000/(2*(20*speed))
		//In 4 step mode, RPM = 60*1000/((20*speed))
		Delay(speed);
	}
}
