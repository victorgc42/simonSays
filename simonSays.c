/* ----------------------------
 *        MINI-PROYECTO       *
 *          SIMON DICE        *
 * González Caballero, Víctor *
 *    Ingeniería Robótica     *
 * ---------------------------*/

#include <msp430.h>
#include <stdlib.h>
#include <math.h>

#define MAX 30
#define LEDS_ON 0
#define LEDS_OFF -1
#define GOAT 500
#define VETERAN 1000
#define NORMAL 1500
#define EASY 2000
#define GAME_OVER 5
#define LEVEL 6

unsigned int statusSimon, statusPlayer;
unsigned int count, score, level;
unsigned int simon[MAX], player[MAX];
unsigned int gameOver, pulsed;
float frecuency;

void SetupPorts();
void InitializeVariables();
void SelectLevel();
void UpdateSequence(int status);
int CompareStatus();
void InitializeVectors();
void DelaySec(int n);

int main(void)
{
	unsigned int i;

    WDTCTL = WDTPW | WDTHOLD;  /* Paramos el watchdog timer */

    SetupPorts();
    gameOver = 0;  level = 0;
    InitializeVariables();
    __bis_SR_register(GIE);

    while(1)
    {
    	while(level == 0)  /* Mientras no haya nivel seleccionado */
    		SelectLevel();

    	while(gameOver == 0)  /* Si el usuario no ha perdido la partida */
    	{
        	UpdateSequence(LEDS_ON);  /* Mostramos secuencia de inicio */
        	DelaySec(500);

    		statusSimon = (rand() % 4) + 1;  /* Generamos un numero aleatorio entre 1 y 4 */
			simon[count] = statusSimon;
			count++;
    		for(i = 0 ; i < count ; i++)
    		{
    			UpdateSequence(simon[i]);  /* Mostramos la sequencia generada */
				DelaySec(level);
				UpdateSequence(LEDS_OFF);  /* Intervalo entre sequencias */
				DelaySec(level);
    		}

    		UpdateSequence(LEDS_ON);  /* Mostramos secuencia de espera */
			for(i = 0 ; i < count ; i++)
			{
				P2IE |= (BIT3 + BIT4 + BIT5 + BIT6);
				pulsed = 0;
				while(pulsed == 0){ }  /* Esperamos la respuesta del usuario */
				player[i] = statusPlayer;
				UpdateSequence(player[i]);  /* Mostramos al usuario que boton ha pulsado */
				P2IE &= ~(BIT3 + BIT4 + BIT5 + BIT6);
				P2IFG &= ~(BIT3 + BIT4 + BIT5 + BIT6);
				DelaySec(500);  /* Intervalo para evitar rebotes */
				UpdateSequence(LEDS_OFF);  /* Intervalo entre secuencias */
			}
			DelaySec(1000);  /* Intervalo entre rondas */

			if(CompareStatus() == 1)  /* Si todas las respuestas son validas */
			{
				score++;
			}
			else
			{
				InitializeVectors();
				InitializeVariables();
				gameOver = 1;  /* Si falla alguna, se acaba la partida */
			}

			for(i = 0 ; i < count ; i++)
				player[i] = 0;  /* Reseteamos siempre las respuestas del usuario */
    	}

    	if(gameOver == 1)  /* Si el usuario pierde la partida */
    	{
    		 UpdateSequence(GAME_OVER);  /* Mostramos secuencia de GAME OVER */
    		 gameOver = 0;  /* Empezamos una nueva partida */
    	}
    }
}

void SetupPorts()
{
	/* Botones */
	P2DIR &=  ~(BIT3 + BIT4 + BIT5 + BIT6);
	P2REN |= (BIT3 + BIT4 + BIT5 + BIT6);
	P2OUT |= (BIT3 + BIT4 + BIT5 + BIT6);
	P2IES |= (BIT3 + BIT4 + BIT5 + BIT6);
	P2IFG &= ~( BIT3 + BIT4 + BIT5 + BIT6);
	P2IE |= (BIT3 + BIT4 + BIT5 + BIT6);

	/* LEDs */
	P3DIR |= (BIT2 + BIT3 + BIT4 + BIT5);
	P3OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);

	/* Piezo Speaker Module - TimerB0 */
	P7DIR |= BIT4;
	P7SEL |= BIT4;  // Selecciona TB0.2
	TB0CTL |= TASSEL_2 + MC_1 + TACLR;  // SMCLK 1MHz, Up Mode
	TB0CCTL2 = OUTMOD_3;  // Set/Reset
	TB0CCR2 = TB0CCR0;  // Colocamos la señal PWM a zero
}

#pragma vector = PORT2_VECTOR
__interrupt void Buttons_ISR(void)
{
	if(P2IFG & BIT3)
	{
		P2IE &= ~BIT3;
		if(level == 0)
			level = EASY;
		statusPlayer = 1;
		frecuency = 493.88;  // Si
		P2IFG &= ~BIT3;
	}
	if(P2IFG & BIT4)
	{
		P2IE &= ~BIT4;
		if(level == 0)
			level = NORMAL;
		statusPlayer = 2;
		frecuency = 415.30;  // Lab
		P2IFG &= ~BIT4;
	}
	if(P2IFG & BIT5)
	{
		P2IE &= ~BIT5;
		if(level == 0)
			level = VETERAN;
		statusPlayer = 3;
		frecuency = 349.23;  // Fa
		P2IFG &= ~BIT5;
	}
	if(P2IFG & BIT6)
	{
		P2IE &= ~BIT6;
		if(level == 0)
			level = GOAT;
		statusPlayer = 4;
		frecuency = 293.66;  // Re
		P2IFG &= ~BIT6;
	}

	pulsed = 1;

	TB0CCR0 = pow(10,6) / frecuency;  // SMCLK = 1MHz
	TB0CCR2 = TB0CCR0 / 2;  // Calcula CCR1 para DC = 50%
}

void InitializeVariables()
{
	count = 0;  score = 0;  pulsed = 0;
}

void SelectLevel()
{
	P2IE |= (BIT3 + BIT4 + BIT5 + BIT6);
	pulsed = 0;
	while(pulsed == 0){ UpdateSequence(LEVEL); }  /* Mostramos secuencia de nivel */
	P2IE &= ~(BIT3 + BIT4 + BIT5 + BIT6);
	P2IFG &= ~(BIT3 + BIT4 + BIT5 + BIT6);
	DelaySec(100);  /* Intervalo para evitar rebotes */
}

void UpdateSequence(int sequence)
{
	switch(sequence)
    {
		case LEDS_ON:
			P3OUT |= (BIT2 + BIT3 + BIT4 + BIT5);
			break;
    	case 1:
    		P3OUT |= BIT2;
    		P3OUT &= ~(BIT3 + BIT4 + BIT5);
    		frecuency = 493.88;  // Si
    		TB0CCR0 = pow(10,6) / frecuency;  // SMCLK = 1MHz
    		TB0CCR2 = TB0CCR0 / 2;  // Calcula CCR1 para DC = 50%
    		break;
    	case 2:
    		P3OUT |= BIT3;
    		P3OUT &= ~(BIT2 + BIT4 + BIT5);
    		frecuency = 415.30;  // Lab
    		TB0CCR0 = pow(10,6) / frecuency;  // SMCLK = 1MHz
    		TB0CCR2 = TB0CCR0 / 2;  // Calcula CCR1 para DC = 50%
    		break;
    	case 3:
    		P3OUT |= BIT4;
    		P3OUT &= ~(BIT2 + BIT3 + BIT5);
    		frecuency = 349.23;  // Fa
    		TB0CCR0 = pow(10,6) / frecuency;  // SMCLK = 1MHz
    		TB0CCR2 = TB0CCR0 / 2;  // Calcula CCR1 para DC = 50%
    		break;
    	case 4:
    		P3OUT |= BIT5;
    		P3OUT &= ~(BIT2 + BIT3 + BIT4);
    		frecuency = 293.66;  // Re
    		TB0CCR0 = pow(10,6) / frecuency;  // SMCLK = 1MHz
    		TB0CCR2 = TB0CCR0 / 2;  // Calcula CCR1 para DC = 50%
    		break;
    	case GAME_OVER:
    		P3OUT |= BIT2;  P3OUT &= ~(BIT3 + BIT4 + BIT5);  DelaySec(100);
    		frecuency = 493.88;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Si
    		P3OUT |= BIT3;  P3OUT &= ~(BIT2 + BIT4 + BIT5);  DelaySec(100);
    		frecuency = 415.30;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Lab
    		P3OUT |= BIT4;  P3OUT &= ~(BIT2 + BIT3 + BIT5);  DelaySec(100);
    		frecuency = 349.23;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Fa
    	    P3OUT |= BIT5;  P3OUT &= ~(BIT2 + BIT3 + BIT4);  DelaySec(100);
    	    frecuency = 293.66;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Re
    		P3OUT |= BIT4;  P3OUT &= ~(BIT2 + BIT3 + BIT5);  DelaySec(100);
    		frecuency = 349.23;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Fa
    		P3OUT |= BIT3;  P3OUT &= ~(BIT2 + BIT4 + BIT5);  DelaySec(100);
    		frecuency = 415.30;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Lab
    		P3OUT |= BIT2;  P3OUT &= ~(BIT3 + BIT4 + BIT5);  DelaySec(100);
    		frecuency = 493.88;  TB0CCR0 = pow(10,6) / frecuency;  TB0CCR2 = TB0CCR0 / 2;  // Si
    		P3OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);  		 DelaySec(200);
    		break;
    	case LEVEL:
    		P3OUT |= (BIT2 + BIT3 + BIT4 + BIT5);   DelaySec(1000);
    		P3OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);  DelaySec(1000);
    		break;
    	default:
    		P3OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);
    }
}

int CompareStatus()
{
	unsigned int i, compare = 1;

	for(i = 0 ; i < count ; i++)
	{
		if(player[i] != simon[i])
			compare = 0;
	}

	return compare;
}

void InitializeVectors()
{
	unsigned int i;

	for(i = 0 ; i < MAX ; i++)
	{
		simon[i] = 0;
		player[i] = 0;
	}
}

void DelaySec(int n)  /* Para contar 1 ms @32 KHz, CCR0 = 10^-3 * 32*10^3 = 32 ciclos */
{
	// TimerA0
	TA0CTL |= TASSEL_1 + MC_1 + TACLR;  /* ACLK = 32 KHz, Up Mode */
	TA0CCR0 = n * 32;               /* n * 1 mseg = n mseg */
	TA0CCTL0 = CCIE;
	TA0CCTL0 &= ~CCIFG;
	__bis_SR_register(LPM0_bits);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_ISR(void)
{
	TA0CCTL0 &= ~CCIFG;
	TB0CCR2 = TB0CCR0;  /* Colocamos la señal PWM a zero */
	__bic_SR_register_on_exit(LPM0_bits);
}

