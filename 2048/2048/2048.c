
#include <avr/io.h>
#include<stdlib.h>
#include<avr/interrupt.h>
#include<avr/eeprom.h>

volatile uint32_t MILLIS;
volatile uint8_t LED[6][8], PWM_BUFF[8], PWM_STATE, PWM_COUNT;

volatile uint8_t JOGO[4][4];
volatile uint32_t SCORE;
#define STACK_SIZE 40
volatile uint8_t STACK[STACK_SIZE][16],SI,SN,SF;
volatile uint32_t SCORE_STACK[STACK_SIZE];

#define LED11 (1<<2)
#define LED12 (1<<1)
#define LED13 (1<<6)
#define LED14 (1<<4)
#define LED21 (1<<3)
#define LED22 (1<<0)
#define LED23 (1<<7)
#define LED24 (1<<5)
#define GREEN_BOT (1<<2)
#define RED_BOT   (1<<3)
#define BLUE_BOT  (1<<4)
#define GREEN_TOP (1<<5)
#define RED_TOP   (1<<6)
#define BLUE_TOP  (1<<7)
#define N_BTN  (!(PINA&(1<<4)))
#define S_BTN  (!(PINA&(1<<0)))
#define W_BTN  (!(PINA&(1<<3)))
#define E_BTN  (!(PINA&(1<<6)))
#define NE_BTN (!(PINA&(1<<7)))
#define SE_BTN (!(PINA&(1<<1)))
#define NW_BTN (!(PINA&(1<<5)))
#define SW_BTN (!(PINA&(1<<2)))



void ioInit(){
	DDRD  = 0b11111100;
	PORTD = 0b00000000;
	DDRC  = 0b11111111;
	PORTC = 0b00000000;
	DDRB  = 0b00000000;
	PORTB = 0b00001111;
	DDRA  = 0b00000000;
	PORTA = 0b11111111;
	SFIOR &= ~(1<<PUD);
}
void clearGame(){
	uint8_t count, *jogo;
	for(count=0,jogo=(uint8_t*)JOGO;count<4*4;count++,jogo++)*jogo = 0;
	SI = 0;
	SF = 0;
	SN = 0;
}
void clearScreen(){
	uint8_t count, *led;
	for(count=0,led=(uint8_t*)LED;count<4*4*3;count++,led++)*led = 0;
}
void delay_ms(uint32_t ms){
	uint32_t fim = ms+MILLIS;
	while(fim>MILLIS);
}
void timerInit(){
	TCCR0  = 0b00001010;//without output, CTC clk/1 
	OCR0 = 40; //20us interruptions
	TIMSK |= 0b00000010;//compare interrupt enabled
	sei();
	clearScreen();
	PWM_STATE = 0;
	PWM_COUNT = 0;
	MILLIS = 0;
	for(uint8_t a=0;a<8;a++)PWM_BUFF[a] = LED[PWM_STATE][a];
	if(PWM_BUFF[0])PORTC|=LED11;
	if(PWM_BUFF[1])PORTC|=LED12;
	if(PWM_BUFF[2])PORTC|=LED13;
	if(PWM_BUFF[3])PORTC|=LED14;
	if(PWM_BUFF[4])PORTC|=LED21;
	if(PWM_BUFF[5])PORTC|=LED22;
	if(PWM_BUFF[6])PORTC|=LED23;
	if(PWM_BUFF[7])PORTC|=LED24;
	PORTD = RED_TOP;
}
ISR(TIMER0_COMP_vect){
	const uint8_t STATES[6]={RED_TOP,GREEN_TOP,BLUE_TOP,RED_BOT,GREEN_BOT,BLUE_BOT};
	uint8_t a;
	PWM_COUNT++;
	if(PWM_COUNT>=50){
		PORTC = 0;
		PORTD = 0;
		PWM_COUNT = 0;
		PWM_STATE++;
		if(PWM_STATE>=6)PWM_STATE = 0;
		for(a=0;a<8;a++)PWM_BUFF[a] = LED[PWM_STATE][a];
		//PORTD = 0;
		a = 0;
		if(PWM_BUFF[0])a|=LED11;
		if(PWM_BUFF[1])a|=LED12;
		if(PWM_BUFF[2])a|=LED13;
		if(PWM_BUFF[3])a|=LED14;
		if(PWM_BUFF[4])a|=LED21;
		if(PWM_BUFF[5])a|=LED22;
		if(PWM_BUFF[6])a|=LED23;
		if(PWM_BUFF[7])a|=LED24;
		PORTD = STATES[PWM_STATE];
		PORTC = a;
		MILLIS++;
	}else{
		if(PWM_BUFF[0]==PWM_COUNT)PORTC&=~LED11;
		if(PWM_BUFF[1]==PWM_COUNT)PORTC&=~LED12;
		if(PWM_BUFF[2]==PWM_COUNT)PORTC&=~LED13;
		if(PWM_BUFF[3]==PWM_COUNT)PORTC&=~LED14;
		if(PWM_BUFF[4]==PWM_COUNT)PORTC&=~LED21;
		if(PWM_BUFF[5]==PWM_COUNT)PORTC&=~LED22;
		if(PWM_BUFF[6]==PWM_COUNT)PORTC&=~LED23;
		if(PWM_BUFF[7]==PWM_COUNT)PORTC&=~LED24;
	}
}
void setR(uint8_t x, uint8_t y, uint8_t v){
	if(x>3)x=3;
	if(y>3)y=3;
	if(v>100)v=100;
	uint8_t a = 0;
	if(y>1){a+=3;y-=2;}
	x+=y*4;
	LED[a][x] = v>>1;
}
void setG(uint8_t x, uint8_t y, uint8_t v){
	if(x>3)x=3;
	if(y>3)y=3;
	if(v>100)v=100;
	uint8_t a = 1;
	if(y>1){a+=3;y-=2;}
	x+=y*4;
	LED[a][x] = v>>1;
}
void setB(uint8_t x, uint8_t y, uint8_t v){
	if(x>3)x=3;
	if(y>3)y=3;
	if(v>100)v=100;
	uint8_t a = 2;
	if(y>1){a+=3;y-=2;}
	x+=y*4;
	LED[a][x] = v>>1;
}
void setLed(uint8_t x,uint8_t y,uint8_t score){
	uint8_t r=0, g=0, b=0;
	uint16_t q = 500 - MILLIS%500;
	uint16_t p = MILLIS%1000;
	if(p>500)p = 1000-p;
	switch(score){
		case 0: 
			break;
		case 1:
			r = 40;
			break;
		case 2:
			b = 40;
			break;
		case 3:
			g = 40;
			break;
		case 4: 
			r = 20;
			g = 20;
			break;
		case 5:
			r = 20;
			b = 20;
			break;
		case 6:
			g = 20;
			b = 20;
			break;
		case 7:
			r = 15;
			g = 15;
			b = 15;
			break;
		case 8:
			r = p/10;
			break;
		case 9:
			b = p/10;
			break;
		case 10:
			g = p/10;
			break;
		case 11:
			r = p/20;
			g = p/20;
			break;
		case 12:
			r = p/20;
			b = p/20;
			break;
		case 13:
			g = p/20;
			b = p/20;
			break;
		case 14:
			r = p/30;
			g = p/30;
			b = p/30;
			break;
		case 15:
			r = q/5;
			break;
		case 16:
			b = q/5;
			break;
		case 17:
			g = q/5;
			break;
		default://mesmo sendo impossível com uma grade de 4x4 ter mais que 2^17, é interessante deixar espaço pros bugs
			r = 100;
			g = 100;
			b = 100;	
			break;
	}
	if(x>3)x=3;
	if(y>3)y=3;
	uint8_t a = 0;
	if(y>1){a+=3;y-=2;}
	x+=y*4;
	//Correção em 04/08 pela substituição de um LED que tem pinagem ao contrário
	// LED[1][3]  -> x = 5  a = 3
	if(a==3 && x==5){
		LED[a][x] = r>>1;
		LED[++a][x] = b>>1;
		LED[++a][x] = g>>1;
	}else{
		LED[a][x] = r>>1;
		LED[++a][x] = g>>1;
		LED[++a][x] = b>>1;
	}	
}
void renderFrame(){
	for(uint8_t x=0;x<4;x++){
		for(uint8_t y=0;y<4;y++){
			setLed(x,y,JOGO[x][y]);
		}
	}
}
void save(){
	uint8_t *j, *s, a;
	SF = (SN+1)%STACK_SIZE;
	if(SF==SI)SI=(SI+1)%STACK_SIZE;
	j = (uint8_t *)JOGO;
	s = (uint8_t *)STACK[SF];
	for(a=0;a<16;a++,j++,s++)*s = *j;
	SCORE_STACK[SF] = SCORE;
	SN = SF;
}
void initializeStack(){
	uint8_t *j, *s, a;
	SN = 0;
	SI = 0;
	SF = 0;
	j = (uint8_t *)JOGO;
	s = (uint8_t *)STACK[0];
	for(a=0;a<16;a++,j++,s++)*s = *j;
	SCORE_STACK[0] = SCORE;
}
void undo(){
	uint8_t *j, *s, a;
	if(SN!=SI){
		SN = (SN+STACK_SIZE-1)%STACK_SIZE;
		j = (uint8_t *)JOGO;
		s = (uint8_t *)STACK[SN];
		for(a=0;a<16;a++,j++,s++)*j = *s;
	}
	SCORE = SCORE_STACK[SN];
}
void redo(){
	uint8_t *j, *s, a;
	if(SN!=SF){
		SN = (SN+1)%STACK_SIZE;
		j = (uint8_t *)JOGO;
		s = (uint8_t *)STACK[SN];
		for(a=0;a<16;a++,j++,s++)*j = *s;
	}
	SCORE = SCORE_STACK[SN];
}
uint8_t efetuarJogada(uint8_t dir){
	//dir:      0
	//       3     1
	//          2
	//como não é possível fazer duas colisões de uma vez só no mesmo quadrado, 
	//temos que discernir aonde já ocorreu uma colisão (nessa jogada)
	uint8_t colisoes[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
	int8_t lasso, x, y, moved = 0;
	for(lasso=0;lasso<3;lasso++){//lasso para a animação de "push"
		switch(dir){
			case 0:
				for(y=1;y<4;y++){
					for(x=0;x<4;x++){
						if(JOGO[x][y]){
							if(JOGO[x][y-1]==0){
								JOGO[x][y-1] = JOGO[x][y];
								colisoes[x][y-1] = colisoes[x][y];
								JOGO[x][y] = 0;
								colisoes[x][y] = 0;
								moved++;
							}else if(JOGO[x][y-1]==JOGO[x][y] && !colisoes[x][y-1] && !colisoes[x][y]){
								JOGO[x][y-1]++;
								SCORE += 2<<JOGO[x][y-1];
								JOGO[x][y] = 0;
								colisoes[x][y-1] = 1;
								moved++;
							}
						}
					}
				}
				break;
			case 1:
				for(x=2;x>=0;x--){
					for(y=0;y<4;y++){
						if(JOGO[x][y]){
							if(JOGO[x+1][y]==0){
								JOGO[x+1][y] = JOGO[x][y];
								colisoes[x+1][y] = colisoes[x][y];
								JOGO[x][y] = 0;
								colisoes[x][y] = 0;
								moved++;
								}else if(JOGO[x+1][y]==JOGO[x][y] && !colisoes[x+1][y] && !colisoes[x][y]){
								JOGO[x+1][y]++;
								SCORE += 2<<JOGO[x+1][y];
								JOGO[x][y] = 0;
								colisoes[x+1][y] = 1;
								moved++;
							}
						}
					}
				}
				break;
			case 2:
				for(y=2;y>=0;y--){
					for(x=0;x<4;x++){
						if(JOGO[x][y]){
							if(JOGO[x][y+1]==0){
								JOGO[x][y+1] = JOGO[x][y];
								colisoes[x][y+1] = colisoes[x][y];
								JOGO[x][y] = 0;
								colisoes[x][y] = 0;
								moved++;
							}else if(JOGO[x][y+1]==JOGO[x][y] && !colisoes[x][y+1] && !colisoes[x][y]){
								JOGO[x][y+1]++;
								SCORE += 2<<JOGO[x][y+1];
								JOGO[x][y] = 0;
								colisoes[x][y+1] = 1;
								moved++;
							}
						}
					}
				}
				break;
			case 3:
			for(x=1;x<4;x++){
				for(y=0;y<4;y++){
					if(JOGO[x][y]){
						if(JOGO[x-1][y]==0){
							JOGO[x-1][y] = JOGO[x][y];
							colisoes[x-1][y] = colisoes[x][y];
							JOGO[x][y] = 0;
							colisoes[x][y] = 0;
							moved++;
						}else if(JOGO[x-1][y]==JOGO[x][y] && !colisoes[x-1][y] && !colisoes[x][y]){
							JOGO[x-1][y]++;
							SCORE += 2<<JOGO[x-1][y];
							JOGO[x][y] = 0;
							colisoes[x-1][y] = 1;
							moved++;
						}
					}
				}
			}
			break;
		}
		renderFrame();
		delay_ms(80);
	}
	return moved;
}
void putRandomNumber(){
	uint8_t x, y, z;
	do{
		x = rand()/ (RAND_MAX / 4 + 1);
		y = rand()/ (RAND_MAX / 4 + 1);
	}while(JOGO[x][y]!=0);	
	z = rand()/ (RAND_MAX / 10 + 1);
	if(z<9)JOGO[x][y] = 1;
	else   JOGO[x][y] = 2;
}
void saveToROM(){
	eeprom_write_block(JOGO,0,16);
	eeprom_write_dword((uint32_t*)16,SCORE);
}
void readFromROM(){
	uint8_t *a, b, erro = 0;
	for(a=0;a<(uint8_t*)16 && !(erro&2);a++){
		b = eeprom_read_byte(a);
		if(b>0) erro |= 1;
		if(b>17) erro |= 2;
	}
	if(erro==1){
		eeprom_read_block(JOGO,0,16);
		SCORE = eeprom_read_dword((uint32_t*)16);
		initializeStack();
	}
}
uint8_t isGameOver(){
	uint8_t x, y, over = 1;
	for(x=0;x<4 && over;x++){
		for(y=0;y<4 && over;y++){
			if(JOGO[x][y]==0)over = 0;
			if(x>0)if(JOGO[x-1][y]==0 ||JOGO[x-1][y]==JOGO[x][y])over = 0;
			if(y>0)if(JOGO[x][y-1]==0 ||JOGO[x][y-1]==JOGO[x][y])over = 0;
		}
	}
	return over;
}
void gameOverAnimation(){
	uint8_t a, b, k;
	renderFrame();
	do{
		k = 0;
		for(a=0;a<6;a++){
			for(b=0;b<8;b++){
				if(LED[a][b]){
					LED[a][b]--;
					k = 1;
				}
			}
		}
		delay_ms(100);
	}while(k==1);		
}
int main(void){
	uint8_t b;
	uint32_t a;
	ioInit();
	timerInit();
	clearGame();
	for(int z=1;z<=3;z++){
		for(int x=0;x<4;x++){
			for(int y=0;y<4;y++){
				setLed(x,y,z);
			}
		}
		delay_ms(200);
	}
	clearScreen();
	clearGame();
	
	JOGO[1][0] = 1;
	JOGO[3][2] = 2;
	JOGO[0][2] = 3;
	while(!NE_BTN){
		b = JOGO[0][1];
		JOGO[0][1] = JOGO[0][2];
		JOGO[0][2] = JOGO[1][3];
		JOGO[1][3] = JOGO[2][3];
		JOGO[2][3] = JOGO[3][2];
		JOGO[3][2] = JOGO[3][1];
		JOGO[3][1] = JOGO[2][0];
		JOGO[2][0] = JOGO[1][0];
		JOGO[1][0] = b;
		renderFrame();
		delay_ms(100);
	}
	srand((MILLIS<<10)|PWM_COUNT);
	clearGame();
	SCORE = 0;
	putRandomNumber();
	putRandomNumber();
	/*
	JOGO[0][1] = 2;
	JOGO[0][2] = 3;
	JOGO[0][3] = 4;
	JOGO[1][0] = 5;
	JOGO[1][1] = 6;
	JOGO[1][2] = 7;
	JOGO[1][3] = 8;
	JOGO[2][0] = 9;
	JOGO[2][1] = 10;
	JOGO[2][2] = 11;
	JOGO[2][3] = 12;
	JOGO[3][0] = 13;
	JOGO[3][1] = 14;
	JOGO[3][2] = 15;
	JOGO[3][3] = 16;
	*/
	
	initializeStack();
	renderFrame();
	while(NE_BTN);
	delay_ms(50);
	while(1){
		renderFrame();
		delay_ms(100);
		if(N_BTN){
			if(efetuarJogada(0)){
				putRandomNumber();
				if(isGameOver())gameOverAnimation();
				save();
			}
			while(N_BTN);
		}
		if(E_BTN){
			if(efetuarJogada(1)){
				putRandomNumber();
				if(isGameOver())gameOverAnimation();
				save();
			}
			while(E_BTN);
		}
		if(S_BTN){
			if(efetuarJogada(2)){
				putRandomNumber();
				if(isGameOver())gameOverAnimation();
				save();
			}
			while(S_BTN);
		}
		if(W_BTN){
			if(efetuarJogada(3)){
				putRandomNumber();
				if(isGameOver())gameOverAnimation();
				save();
			}				
			while(W_BTN);
		}
		if(NE_BTN){
			clearGame();
			putRandomNumber();
			putRandomNumber();
			initializeStack();
			renderFrame();
			while(NE_BTN && !SE_BTN);
			if(SE_BTN){
				readFromROM();
				renderFrame();
				while(NE_BTN || SE_BTN);
			}
		}
		if(SW_BTN){
			undo();
			renderFrame();
			delay_ms(50);
			while(SW_BTN);
			delay_ms(50);
		}
		if(NW_BTN){
			redo();
			renderFrame();
			delay_ms(50);
			while(SW_BTN);
			delay_ms(50);
		}
		if(SE_BTN){
			a = MILLIS+2000;
			delay_ms(50);
			while(SE_BTN && (a>MILLIS));
			if(a<=MILLIS){
				volatile uint8_t *p = LED[0];
				saveToROM();
				for(b=0;b<4*4*3;b++, p++)	*p = 10;
				delay_ms(100);
				renderFrame();
				while(SE_BTN);				
			}
		}
	}
}
