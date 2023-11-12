/*
 * snake.c
 *
 * Created: 23/12/2022 4:50:29 PM
 * Author : uturkmen16 & vturgut18
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include "new_font.h"
#include "small_font.h"

#define F_CPU 8000000L

#include <util/delay.h>

#define Data_Port PORTC
#define TotalPage 8


int foodx, foody;
uint8_t gameOverCounter = 0;
int snakex[100], snakey[100];
int snakelen = 20;
int score = 0;
char playerName[4] = {'A','A','A','A'};
uint8_t activePlayerNameChar = 0;
uint8_t currentScoreIndex = 0;
bool snakeMoved = false;
bool gameStarted = false;
bool increaseActivePlayerNameChar = false;
bool decreaseActivePlayerNameChar = false;
bool increasePlayerNameChar = false;
bool decreasePlayerNameChar = false;
bool Continue = false;
enum Direction { Left = 0, Up = 1, Right = 2, Down = 3 };
enum Direction direction;
enum Direction nextDirection;
char achievements[80];

void GLCD_Init();
void GLCD_Command(char Command);
void GLCD_ClearAll();
void GLCD_Data(char Data);
void paint(int* snakex, int* snakey, int snakelen);
void moveLeft(int* x, int* y, int snakelen);
void moveRight(int* x, int* y, int snakelen);
void moveUp(int* x, int* y, int snakelen);
void moveDown(int* x, int* y, int snakelen);
void checkOutOfSpace(int* x, int* y, int snakelen);
bool checkSnakeCollision();
bool checkFoodCollision();
void paintLetter(char v, char letter, char startPage, char startColumn);
void paintLetterUpper(uint8_t letter);
void paintLetterLower(uint8_t letter);
void paintLetterSmall(uint8_t v, char letter, uint8_t startPage, char startColumn);
void paintNumberSmall(uint8_t v, uint8_t number, uint8_t startPage, char startColumn);
void moveSnake(int* x, int* y, int snakelen, enum Direction dir);
void changeDirection();
void initalizeSnake();
void InitializeADC();
void InitializeTimer1andExtInt();
void paintGameOver();
void displayName();
void displayScore(char* currentPlayerName, uint8_t score, uint8_t scoreIndex);
void displayAchievements();
void readAchievementsEEPROM();
void writeAchievementsEEPROM();
void sortAchievements();
void checkAchievement();
void buzzEat();
void buzzGameOver();
uint8_t EEPROM_read(uint8_t address);
void EEPROM_write(uint8_t address, uint8_t value);

int main(void) {
  // Initialize uninitialized variables
  direction = Right;
  nextDirection = Right;
  
  // Initialize GLCD
  GLCD_Init();
  
  // Clear all GLCD display
  GLCD_ClearAll();
  
  // Initialize ADC Converter
  InitializeADC();
  
  // Initialize Timer
  InitializeTimer1andExtInt();
  
  _delay_ms(1000);
  
  // Initialize snake position
  initalizeSnake();
  
  

  PORTD |= (1 << PD2);
  
	
  while(!Continue){
	  GLCD_ClearAll();
	  displayName();
	  _delay_ms(100);
  }
  
  // Main game loop
  while (!checkSnakeCollision()) { //until game finishes
	 
	changeDirection(); //change direction if necessary
	moveSnake(snakex, snakey, snakelen, direction); //move the snake every loop
	checkOutOfSpace(snakex, snakey, snakelen);		//check if it is out of space and make it come at the opposite side of the screen
	
	if (checkFoodCollision()) { //if food is eaten
		buzzEat();				//ring the buzzer
		PORTD |= (1 << PD2);	//set the logic pin PD2 high so that the external interrupt occurs which takes the current TCNT1H and TCNT1L values to create the food. This was the best solution for creating the food appear at random places.
		score++;				
		snakelen++;
	}
	paint(snakex, snakey, snakelen);
	_delay_ms(50);
}
  
  for(int i = 0; i < 4; i++) {
	  GLCD_ClearAll();
	   _delay_ms(500);
	  paint(snakex, snakey, snakelen);
	  _delay_ms(500);
  }
  
  Continue = true;
  
  // Read achievements from EEPROM
  readAchievementsEEPROM();
  
  checkAchievement();
    
  sortAchievements();
    
  writeAchievementsEEPROM();
    
  while(1){
	  if(Continue) {
		  GLCD_ClearAll();
		  paintGameOver();
		  _delay_ms(500);
	  }
	  else {
		  GLCD_ClearAll();
		  displayAchievements();
		  _delay_ms(500);
	  }
	  
  }
}

void changeDirection() {
	direction = nextDirection;
}

void initalizeSnake() {
	  for (int i = 0; i < snakelen; i++) {
		  snakex[i] = snakelen - 1 - i;
		  snakey[i] = 15;
	  }
}

void moveSnake(int* x, int* y, int snakelen, enum Direction dir) {
  switch (dir) {
    case Left:
      moveLeft(x, y, snakelen);
      return;
    case Right:
      moveRight(x, y, snakelen);
      return;
    case Up:
      moveUp(x, y, snakelen);
      return;
    case Down:
      moveDown(x, y, snakelen);
      return;
  }
}

void moveRight(int* x, int* y, int snakelen) {
  for (int i = snakelen - 2; i >= 0; i--) {
    x[i + 1] = x[i];
    y[i + 1] = y[i];
  }
  x[0] = x[0] + 1;
}

void moveLeft(int* x, int* y, int snakelen) {
  for (int i = snakelen - 2; i >= 0; i--) {
    x[i + 1] = x[i];
    y[i + 1] = y[i];
  }
  x[0] = x[0] - 1;
}

void moveUp(int* x, int* y, int snakelen) {
  for (int i = snakelen - 2; i >= 0; i--) {
    x[i + 1] = x[i];
    y[i + 1] = y[i];
  }
  y[0] = y[0] - 1;
}

void moveDown(int* x, int* y, int snakelen) {
  for (int i = snakelen - 2; i >= 0; i--) {
    x[i + 1] = x[i];
    y[i + 1] = y[i];
  }
  y[0] = y[0] + 1;
}

bool checkSnakeCollision() { //check head of the snake to all other parts, if it does collide end the game.
  for (int i = 1; i < snakelen; i++) {
    if (snakex[0] == snakex[i] && snakey[0] == snakey[i]) return true;
  }
  return false;
}

bool checkFoodCollision() { //check if snake ate the food or not
  if (snakex[0] == foodx && snakey[0] == foody) return true;
  return false;
}

void checkOutOfSpace(int* x, int* y, int snakelen) { //check if snake moved out of bounds, if it does make it appear from the other side of the screen
  for (int i = 0; i < snakelen; i++) {
    x[i] = x[i] % 64;
    y[i] = y[i] % 32;

    if (x[i] < 0) x[i] += 64;
    if (y[i] < 0) y[i] += 32;
  }
}

void GLCD_Init() {
  DDRA = 0xFF;
  DDRD = 0xFF;
  DDRB = 0xFF;
  DDRC = 0xFF;

  _delay_ms(20);

  PORTA = 0x0C;
  PORTD = 0x80;
  PORTB = 0x02;

  PORTD |= (1 << 7);
  _delay_ms(20);
  GLCD_Command(0x3E); /* Display OFF */
  GLCD_Command(0x42); /* Set Y address (column=0) */
  GLCD_Command(0xB8); /* Set x address (page=0) */
  GLCD_Command(0xC0); /* Set z address (start line=0) */
  GLCD_Command(0x3F); // Display ON
}

void GLCD_Change(bool screen) {
	//false = left screen, true = right screen

	PORTA = 0x0C;
	PORTD = 0x80;
	if(screen) PORTB = 0x01;
	else PORTB = 0x02;

	PORTD |= (1 << 7);
	GLCD_Command(0x3E); /* Display OFF */
	GLCD_Command(0x42); /* Set Y address (column=0) */
	GLCD_Command(0xB8); /* Set x address (page=0) */
	GLCD_Command(0xC0); /* Set z address (start line=0) */
	GLCD_Command(0x3F); // Display ON
}

void GLCD_Command(char Command) {
  Data_Port = Command;
  PORTA &= ~(1 << 2);  // Make RS LOW
  PORTA &= ~(1 << 3);  // Make R/W LOW
  PORTD |= (1 << 6);   // HIGH-LOW Enable
  _delay_us(5);
  PORTD &= ~(1 << 6);
  _delay_us(5);
}

void GLCD_ClearAll() /* GLCD all display clear function */
{
	for (int v = 0; v < 2; v++){
	  if(v == 0) GLCD_Change(false);
	  else if(v == 1) GLCD_Change(true);
	  
	  for (int i = 0; i < TotalPage; i++) {
		GLCD_Command((0xB8) + i); /* Increment page */
		for (int j = 0; j < 64; j++) {
		  GLCD_Data(0x00);
		}
	  }
	  GLCD_Command(0x40); /* Set Y address (column=0) */
	  GLCD_Command(0xB8); /* Set x address (page=0) */
	}
}

void readAchievementsEEPROM(){
	for(int i = 0; i < 80; i++) {
		//achievements[i] = eeprom_read_byte((uint8_t*)(i+10));
		achievements[i]=EEPROM_read(i+10);	
	}
}

void writeAchievementsEEPROM(){
	for(int i = 0; i < 80; i++) {
		//eeprom_write_byte((uint8_t*)(i+10),achievements[i]);
		EEPROM_write((i+10), achievements[i]);	
	}
}

void displayAchievements() {
	for(int i = 0; i < 16; i++) {
		displayScore(&achievements[i*5],achievements[i*5 + 4],i);
	}
}

void paint(int* snakex, int* snakey, int snakelen)
{
	for (int v = 0; v < 2; v++){
		if(v == 0) GLCD_Change(false);
		else if(v == 1) GLCD_Change(true);
		GLCD_Command(0x40); /* Set Y address (column=0) */
		GLCD_Command(0xB8); /* Set x address (page=0) */
		for (int i = 0; i < TotalPage; i++) {
			//Set page number between 0-7
			GLCD_Command(0xB8 + i);
			for (int j = 0; j < 32; j++) {
				int data = 0x00;
				// Paint snake
				for (int t = 0; t < snakelen; t++) {
					
					//For left screen
					if (v == 0) {
						if (snakex[t] / 32 == 0 && snakex[t] == j && snakey[t] / 4 == i) {
							switch (snakey[t] % 4) {
								case 0:
								data |= 0x03;
								break;
								case 1:
								data |= 0x0C;
								break;
								case 2:
								data |= 0x30;
								break;
								case 3:
								data |= 0xC0;
								break;
							}
						}
					}
					
					//For right screen
					else if (v == 1) {
						if (snakex[t] / 32 == 1 && snakex[t] % 32 == j && snakey[t] / 4 == i) {
							switch (snakey[t] % 4) {
								case 0:
								data |= 0x03;
								break;
								case 1:
								data |= 0x0C;
								break;
								case 2:
								data |= 0x30;
								break;
								case 3:
								data |= 0xC0;
								break;
							}
						}
					}
				}
				
				
				// Paint food left
				if (v == 0 && foodx / 32 == 0 && foodx == j && foody / 4 == i) {
					switch (foody % 4) {
						case 0:
						data |= 0x03;
						break;
						case 1:
						data |= 0x0C;
						break;
						case 2:
						data |= 0x30;
						break;
						case 3:
						data |= 0xC0;
						break;
					}
				}
				
				
				// Paint food right
				if (v == 1 && foodx / 32 == 1 && foodx % 32 == j && foody / 4 == i) {
					switch (foody % 4) {
						case 0:
						data |= 0x03;
						break;
						case 1:
						data |= 0x0C;
						break;
						case 2:
						data |= 0x30;
						break;
						case 3:
						data |= 0xC0;
						break;
					}
				}
				GLCD_Data(data);
				GLCD_Data(data);
			}
		}
		GLCD_Command(0x40); /* Set Y address (column=0) */
		GLCD_Command(0xB8); /* Set x address (page=0) */
	}
}


void GLCD_Data(char Data) /* GLCD data function */
{
  Data_Port = Data;    /* Copy data on data pin */
  PORTA |= (1 << 2);   // RS to HIGH
  PORTA &= ~(1 << 3);  // RW to LOW
  PORTD |= (1 << 6);   // Enable
  _delay_us(5);
  PORTD &= ~(1 << 6);  // Disable
  _delay_us(5);
}

ISR(ADC_vect) {
  int ADCOut = ADCL | (ADCH << 8); //read ADC value is saved, the result gives a result between 0 and 1024 because ADCH is 2 bits and ADCL is 8 bits which makes a 10 bit resolution. 

  switch (ADMUX) {					//since in ADC initialize first channel selected was 5, we will stick to it in switch and make sure we put it on the top to not miss the first reading
    case 0xC5:						//check if it is channel 5, the 5th channel is reading the analog value of x axis of joystick
      if (ADCOut > 674) {			//if the reading is higher then a threshold of 674, which was put to cancel out the noise because if we put 512 which is right between 1024 and 0 snake sometimes moves arbitrary. 
									//We also didn't want player to regret midway his choice of direction so we left a little error margin for the player in case they want to change to another direction before it is too late.  
        // RIGHT
        if (direction != Left && gameStarted) {  //checks if game started and direction is not left since it is prohibited to turn 180 degree in this game. 
          nextDirection = Right;                 //we used a flag because we found a bug which is while the next square is not displayed in the game, the code could easily enter two interrupts of ADC where let's assume the 
											     //direction is left and we first choose up, then right. The new direction becomes right and then game executes it in while loop and snake overlaps with itself and the game does not finish.
												 //Therefore we are changing the flag and we are changing the direction in the while loop. This is something we both learned from COMP 302 course software programming where we 
												 //programmed a game as well. We all used game view controller pattern for necessity. That pattern was about asynchronously reading the input from the player and synchronously reflecting it to the game. 
												 //If an input is read, a flag is activated or deactivated to later execute it at the right time and clearing the flag for further uses.
        }
		else if(!gameStarted && activePlayerNameChar < 3) {		//if game has started go to one next char at the right in the first section of the game 
			increaseActivePlayerNameChar = true;
			decreaseActivePlayerNameChar = false;
			_delay_ms(10);
		}
        _delay_us(5);
      } else if (ADCOut < 350) {		//check if ADCout is lower than 350 which indicates that the joystick movement is left.
        // LEFT
        if (direction != Right && gameStarted) {
          nextDirection = Left;
        }
		else if(!gameStarted && activePlayerNameChar > 0) {
			increaseActivePlayerNameChar = false;
			decreaseActivePlayerNameChar = true;
			_delay_ms(10);
		}
        _delay_us(5);
      }
      ADMUX = 0xC4;                    //connect the 4th channel to ADC. This is very important because we want to read 3 different ADC conversions at the same time. But we can only choose 1 channel from the ADMUX register and we chose it 
									   //to be the 4th channel in ADC Initialization. Now we change the ADMUX to the desired channel and continue to check for the case of this channel. Select the 4th channel
      break;
    case 0xC4:						   //Check if it is the 4th channel. Since will be always selected in the previous case, all of the cases will be repeatedly executed. Therefore we will be able to read all the ADC conversions at one interrupt. 

      if (ADCOut > 700) {			   
        // UP
        if (direction != Down && gameStarted) {  //if game started and direction is not down change the direction up
          nextDirection = Up;
        }
		else if(!gameStarted) {  //if game not started choose the left character to be changed in the first section for the name
			increasePlayerNameChar = false;
			decreasePlayerNameChar = true;
			_delay_ms(1);
		}
        _delay_us(5);
      } else if (ADCOut < 350) {
        // DOWN
        if (direction != Up && gameStarted) {
          nextDirection = Down;
        }
		else if(!gameStarted) {
			increasePlayerNameChar = true;
			decreasePlayerNameChar = false;
			_delay_ms(1);
		}
        _delay_us(5);
      }
      ADMUX = 0xC7;  //connect the ADC to channel 7. 
      break;
	case 0xC7:		//check if ADMUX is the 7th channel. 
		if(ADCOut < 50) {  
			//For the x y values at the center, 512 is assigned to them which corresponds to 2.5V which is middle of 0 and 5V. So the center is 2.5V, down or left is 0V, up or right is 5V. 
			//For push button, the value is 5V at initial position and when it is pressed it becomes zero. It works like a switch. I didn't have to use ADC converter for this purpose but I did anyways. 
			Continue = !Continue;
			gameStarted = true;
		}
		ADMUX = 0xC5; //return back to the first channel. 
		break;
    default:
      // Default code
      break;
  }
  ADCSRA |= 1 << ADSC; //start ADC conversion again so that it repeatedly enters the interrupt. 
}

void InitializeADC() {
  DDRA = 0x00;
  DDRA |= 0b01001111;
  ADCSRA |= 1 << ADPS2 | 1<< ADPS1 | 1<< ADPS0;								   
  //division for xtal frequency and clock frequency. This prescalar specifies the speed and accuracy for ADC conversion. 
  //It is recommended that ADC conversion frequency needs to be between 50 kHz and 200kHz according to our research. With lower prescalar the frequency will be higher. The higher frequency means fast 
  //conversion but less accurate compared to low frequency conversion which is low speed high accuracy. 8.000.000/128=62.5 kHz. Since our program doesn't rely on very accurate results, 
  //since the only important thing is to distinguish between up down left right, we didn't need a high ADC frequency.
  
  ADMUX |= 1 << REFS0 | 1 << REFS1 | 1 << 0 | 1 << 2;  //adjusting ADMUX so that internal reference voltage is used and ADC channel 5 is connected.
  ADCSRA |= 1 << ADIE;								   //ADC control and status register: enabling interrupt register and then enabling ADC
  ADCSRA |= 1 << ADEN;

  sei();												//enabling interrupts

  ADCSRA |= 1 << ADSC;									//starting ADC conversion
}

void InitializeTimer1andExtInt(){
        DDRD=0b11111111;
        GICR = 1<<INT0;        // Enable INT0
        MCUCR = 1<<ISC01 | 1<<ISC00;  // Trigger INT0 on rising edge D2
        TCNT1=0;                      //adjusting timer1 counter value
        TCCR1B= (1<<0);				  //adjusting the timer controller b register
}

void paintGameOver() { //for dipslaying the game over animation with the motion of going up and down
	buzzGameOver();
	gameOverCounter %= 4;
	
	if(gameOverCounter == 0) {
		paintLetter(0,'G',3,7);
		paintLetter(0,'A',3,20);
		paintLetter(0,'M',3,33);
		paintLetter(0,'E',3,46);
		paintLetter(1,'O',3,7);
		paintLetter(1,'V',3,20);
		paintLetter(1,'E',3,33);
		paintLetter(1,'R',3,46);
	}
	
	else if(gameOverCounter == 1) {
		paintLetter(0,'G',2,7);
		paintLetter(0,'A',4,20);
		paintLetter(0,'M',2,33);
		paintLetter(0,'E',4,46);
		paintLetter(1,'O',2,7);
		paintLetter(1,'V',4,20);
		paintLetter(1,'E',2,33);
		paintLetter(1,'R',4,46);
	}
	
	else if(gameOverCounter == 2) {
		paintLetter(0,'G',3,7);
		paintLetter(0,'A',3,20);
		paintLetter(0,'M',3,33);
		paintLetter(0,'E',3,46);
		paintLetter(1,'O',3,7);
		paintLetter(1,'V',3,20);
		paintLetter(1,'E',3,33);
		paintLetter(1,'R',3,46);
	}
	
	else if(gameOverCounter == 3) {
		paintLetter(0,'G',4,7);
		paintLetter(0,'A',2,20);
		paintLetter(0,'M',4,33);
		paintLetter(0,'E',2,46);
		paintLetter(1,'O',4,7);
		paintLetter(1,'V',2,20);
		paintLetter(1,'E',4,33);
		paintLetter(1,'R',2,46);
	}
	
	gameOverCounter++;
}

void displayName() {
	//gameOverCounter is used again since displayName() and gameOver() won't be used simultaneously
	if(decreaseActivePlayerNameChar) {
		activePlayerNameChar--;
		decreaseActivePlayerNameChar = false;
	}
	else if(increaseActivePlayerNameChar) {
		activePlayerNameChar++;
		increaseActivePlayerNameChar = false;
	}
	
	if(increasePlayerNameChar) {
		playerName[activePlayerNameChar]++;
		if(playerName[activePlayerNameChar] > 'Z') {
			playerName[activePlayerNameChar] = 'A';
		}
		increasePlayerNameChar = false;
	}
	else if(decreasePlayerNameChar) {
		playerName[activePlayerNameChar]--;
		if(playerName[activePlayerNameChar] < 'A') {
			playerName[activePlayerNameChar] = 'Z';
		}
		decreasePlayerNameChar = false;
	}	
	
	gameOverCounter %= 2;
	
	if(gameOverCounter == 0) {
		paintLetter(0,playerName[0],3,17);
		paintLetter(0,playerName[1],3,45);
		paintLetter(1,playerName[2],3,8);
		paintLetter(1,playerName[3],3,35);
	}
	
	else if(gameOverCounter == 1) {
		if(activePlayerNameChar != 0) paintLetter(0,playerName[0],3,17);
		if(activePlayerNameChar != 1) paintLetter(0,playerName[1],3,45);
		if(activePlayerNameChar != 2) paintLetter(1,playerName[2],3,8);
		if(activePlayerNameChar != 3) paintLetter(1,playerName[3],3,35);
	}	
	
	gameOverCounter++;
}

void displayScore(char* currentPlayerName, uint8_t score, uint8_t scoreIndex) { //displaying the score at last which consists of multiple GLCD functions as you can see below. 
	uint8_t v = scoreIndex / 8;
	uint8_t page = scoreIndex % 8;
	paintLetterSmall(v,currentPlayerName[0],page,0);
	paintLetterSmall(v,currentPlayerName[1],page,7);
	paintLetterSmall(v,currentPlayerName[2],page,14);
	paintLetterSmall(v,currentPlayerName[3],page,21);
	uint8_t tmp = score;
	paintNumberSmall(v,(tmp % 10),page,49);
	tmp /= 10;
	paintNumberSmall(v,(tmp % 10),page,42);
	tmp /= 10;
	paintNumberSmall(v,(tmp % 10),page,35);
}

void paintLetterSmall(uint8_t v, char letter, uint8_t startPage, char startColumn) { //painting small letter for GLCD
		if(v == 0) GLCD_Change(false);
		else if(v == 1) GLCD_Change(true);
		GLCD_Command(0x40 + startColumn);
		GLCD_Command(0xB8 + startPage);
		uint8_t letterindex = letter - 65;
		for(int i = 0; i < small_font_letter_size; i++) {
			GLCD_Data(pgm_read_byte(&(small_Font[small_font_letter_start_index + letterindex*small_font_letter_size + i])));
		}	
}

void paintNumberSmall(uint8_t v, uint8_t number, uint8_t startPage, char startColumn) { //painting small number for GLCD
	if(v == 0) GLCD_Change(false);
	else if(v == 1) GLCD_Change(true);
	GLCD_Command(0x40 + startColumn);
	GLCD_Command(0xB8 + startPage);
	for(int i = 0; i < small_font_number_size; i++) {
		GLCD_Data(pgm_read_byte(&(small_Font[small_font_number_start_index + number*small_font_number_size + i])));
	}
}

void paintLetter(char v, char letter, char startPage, char startColumn){ //painting letter for GLCD
	
	if(v == 0) GLCD_Change(false);
	else if(v == 1) GLCD_Change(true);
	GLCD_Command(0x40 + startColumn);
	GLCD_Command(0xB8 + startPage);
	uint8_t letterindex = letter - 65;
	paintLetterUpper(letterindex);
	
	GLCD_Command(0x40 + startColumn);
	GLCD_Command(0xB8 + startPage + 1);
	paintLetterLower(letterindex);
}

void deleteLetter(char v, char startPage, char startColumn){ //delete letter for GLCD
	
	if(v == 0) GLCD_Change(false);
	else if(v == 1) GLCD_Change(true);
	GLCD_Command(0x40 + startColumn);
	GLCD_Command(0xB8 + startPage);
	for (int i = 0; i < new_font_letter_size / 2; i++) GLCD_Data(0x00);
}

void paintLetterUpper(uint8_t letter) { //print uppercase letter for GLCD
	for(int i = 0; i < new_font_letter_size / 2; i++) {
		GLCD_Data(pgm_read_byte(&(new_Font[new_font_letter_start_index + letter*new_font_letter_size + i])));
	}
}

void paintLetterLower(uint8_t letter) { //print lowercase letter for GLCD
	for(int i = new_font_letter_size / 2; i < new_font_letter_size; i++) {
		GLCD_Data(pgm_read_byte(&(new_Font[new_font_letter_start_index + letter*new_font_letter_size + i])));
	}
}

void sortAchievements() { //after checking the achievements and if the current player needs to be in the scoreboard, putting him in the bottommost rank, this function sorts the whole scoreboard.
	
    for (int i = 0; i < 16; i++) {
	    
	    // iterates the array elements from index 1
	    for (int j = i + 1; j < 16; j++) {
			
		    if (achievements[5*i + 4] < achievements[5*j + 4]) {  //sort all of the achievemnts in an decreasing order.
			    for(int k = 0; k < 5; k++) {
					uint8_t temp = achievements[5*i + k];
					achievements[5*i + k] = achievements[5*j + k];
					achievements[5*j + k] = temp;
				}
		    }
	    }
    }	
}

void checkAchievement(){
	if(score >= achievements[79]) { //check if the current score surpasses the least score on scoreboard
		for(int i = 0; i < 4; i++){		
			achievements[i + 75] = playerName[i]; //if it does write the new name and new score to the least score in the scoreboard. This function is for to only saving the score to the scoreboard. It will be sorted correctly later.
		}
		achievements[79] = score;
	}
}

void buzzEat(){
	DDRD = 0xff;           // Configure PORTC as output

	for(int i=0; i<40; i++){
		PORTD = 0xff;        // Turn ON the Buzzer connected to PORTC
		_delay_ms(1);      // Wait for some time
		PORTD = 0x00;        // Turn OFF the Buzzer connected to PORTC
		_delay_ms(1);      // Wait for some time
	}
	PORTD=0;
}

void buzzGameOver(){
	DDRD = 0xff;           // Configure PORTC as output

	for(int i=0; i<10; i++){
		PORTD = 0xff;        // Turn ON the Buzzer connected to PORTC
		_delay_ms(3);      // Wait for some time
		PORTD = 0x00;        // Turn OFF the Buzzer connected to PORTC
		_delay_ms(3);      // Wait for some time
	}
	PORTD=0;
}

ISR(INT0_vect)
{

    foodx=TCNT1H%32;
    foody=TCNT1L%32;        

	int i=-1;
	while(i<snakelen){
		i++;
		if(snakex[i]==foodx && snakey[i]==foody){  //check if food overlaps with any of snake's locations
			foody++;								//if it does increment y and execute the whole while loop from the beginning
			i=-1;
		}
	}
    PORTD &= ~(1 << PD2);							//clearing the PD2 in order to deactivate INT0 interrupt.
    _delay_us(5);                                  // Software debouncing control delay 

}


uint8_t EEPROM_read(uint8_t address){        //function for reading from EEPROM
	uint8_t result;
	while(EECR&(1<<EEWE));					//wait for last writing operation to be finished
	EEAR=address;							//writing the address value into the EEPROM address register
	EECR|=(1<<EERE);						//writing 1 to EEPROM reading enable bit
	result=EEDR;							
	return result;							//returning the value that is written into EEPROM Data Register
	}
	
void EEPROM_write(uint8_t address, uint8_t value){	//function for writing to EEPROM
	while(EECR&(1<<EEWE));							//wait till last writing operation to be finished just in case
	EEAR=address;									//write the address value into the EEPROM address register
	EEDR=value;										//write the data value into EEPROM data register
	EECR|=(1<<EEMWE);								//enabling by writing 1 to EEPROM Master Writing Enable bit
	EECR|=(1<<EEWE);								//enabling by writing 1 to EEPROM Writing Enable bit
}

