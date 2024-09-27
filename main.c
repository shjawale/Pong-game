/*        Author: shruti
 *  Partner(s) Name: 
 *        Lab Section:
 *        Assignment: Lab #  Exercise #
 *        Exercise Description: [optional - include for your own benefit]
 *
 *        I acknowledge all content contained herein, excluding template or example
 *        code, is my own original work.
 */
//#include "ledmatrixexample.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "timer.h"
#include "keypad.h"
#include "scheduler.h"
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char button;
int cnt;
int keycnt;

unsigned char posrow;
unsigned char poscol;
signed char velrow;
signed char velcol;

unsigned char ai;
unsigned char paddle1;
unsigned char paddle2;

unsigned char column;
unsigned char row;

unsigned char roundRobin;

unsigned long ballperiod_incr = 10000;
int N = 1;
unsigned long ballperiod = 100000;

int score1;
int score2;
int maxscore = 1;
int gamewait;

unsigned char x;
unsigned char spin;

static task  taskkeypad, taskscore, taskinput, taskball, taskpad1, taskpad2, taskop1, taskop2, taskob;

void reset(){
    cnt = 0;
    spin = 0;
    keycnt = 0;
    gamewait = 0;
    score1 = 0;
    score2 = 0;
    taskball.period = ballperiod * N;
    posrow = 0x03;
    poscol = 0x05;
    velrow = 0;
    velcol = -1;
    paddle1 = 3;
    paddle2 = 3;
    roundRobin = 1; //123
}

void replay(){
    cnt = 0;
    spin = 0;
    keycnt = 0;
    gamewait = 0;
    taskball.period = ballperiod * N;
    posrow = 0x03;
    poscol = 0x05;
    velrow = 0;
    velcol = -1;
    paddle1 = 3;
    paddle2 = 3;
    roundRobin = 1; //123
}

enum KeypadSM {koff, k1on, k2on};
int KeypadSM(int state) {
    // Transitions
    x = GetKeypadKey();
    switch (state) {
        case koff:
            if(x == '1'){//move up
                state = k1on;
                if(keycnt > -2){
                    keycnt -= 1;
                }
            }
            else if(x == '2'){//move down
                state = k2on;
                if(keycnt < 2){
                    keycnt += 1;
                }
            }
            break;
        case k1on:
            if(x == '\0'){
                state = koff;
            }
            break;
        case k2on:
            if(x == '\0'){
                state = koff;
            }
            break;
        default:
            state = koff;
            break;
    }
    // Actions
    switch (state) {
        case koff:
            break;
        case k1on:
            break;
        case k2on:
            break;
        default:
            break;
    }
    return state;
}

#define pat 64
#define shift 4 

enum Scoreboard {showscore};
int Scoreboard(int state) {
    // Transitions
    unsigned char slice = (gamewait >> shift) & 0x7F; //128
    unsigned char blank = slice & 0x40;
    unsigned char idx = slice & 0x03;

    if(gamewait > 0){ //gamewait not 0
        gamewait += 1;
        if(gamewait == 10000){
            gamewait = 0;
            reset();
            return state;
        }
    }
    switch (state) {
        case showscore:
            if(score1 == maxscore){
                if(gamewait == 0){
                    gamewait = 1;
                } else{
                    if(blank){
                        column = 0x00;
                        row = 0x00;
                    } else{
                        if(idx == 0){
                           column = 0x80;
                           row = 0x04;
                        }
                        else if(idx == 1){
                           column = 0x40;
                           row = 0x08;
                        }
                        else if(idx == 2){
                           column = 0x20;
                           row = 0x04;
                        }
                        else if(idx == 3){
                           column = 0x10;
                           row = 0x02;
                        }
                    }
                }
            }
            else if(score2 == maxscore){
                if(gamewait == 0){
                    gamewait = 1;
                } else{
                    if(blank){
                        column = 0x00;
                        row = 0x00;
                    } else{
                        if(idx == 0){
                           column = 0x08;
                           row = 0x04;
                        }
                        else if(idx == 1){
                           column = 0x04;
                           row = 0x08;
                        }
                        else if(idx == 2){
                           column = 0x02;
                           row = 0x04;
                        }
                        else if(idx == 3){
                           column = 0x01;
                           row = 0x02;
                        }
                    }
                }
            }
            break;
        default:
            state = showscore;
            break;
    }
    // Actions
    switch (state) {
        case showscore:
            PORTC = column;
            PORTD = ~row;
            break;
        default:
            break;
    }
    return state;
}


enum Pad2Dir {off, b1on, b2on};
int Pad2Dir(int state) {
    // Transitions
    button = (~PINA) & 0x07;
    switch (state) {
        case off:
            if(button == 1){//move up
                state = b1on;
                if(cnt > -2){
                    cnt -= 1;
                }
            }
            else if(button == 2){//move down
                state = b2on;
                if(cnt < 2){
                    cnt += 1;
                }
            }
            else if(button == 4){//move down
                spin = 1;
            }
            break;
        case b1on:
            if(button == 0){
                state = off;
            }
            break;
        case b2on:
            if(button == 0){
                state = off;
            }
            break;
        default:
            state = off;
            break;
    }
    // Actions
    switch (state) {
        case off:
            break;
        case b1on:
            break;
        case b2on:
            break;
        default:
            break;
    }
    return state;
}

enum Ball {move};
int Ball(int state) {
    if(gamewait > 0){
        return state;
    }
    // Transitions
    switch (state) {
        case move:
            break;
        default:
            state = move;
            break;
    }
    // Actions
    switch (state) {
      case move:
        if(posrow == 1){
            if(velrow == -1){
                velrow = -velrow; //case where bounces
            }
        }
        else if(posrow == 5){
            if(velrow == 1){
                velrow = -velrow; //case where bounces
            }
        }

        if(poscol == 1){ 
            if(velcol == -1){
                if(posrow == (paddle1 - 1)){
                    taskball.period = (ballperiod - ballperiod_incr * 7) * N;
                    velrow = -1;   
                } else if(posrow == paddle1){
                    taskball.period = (ballperiod - ballperiod_incr * 7) * N;
                    velrow = -velrow;
                } else if(posrow == (paddle1 + 1)){
                    taskball.period = (ballperiod - ballperiod_incr * 7) * N;
                    velrow = 1;
                } else if((posrow < (paddle1 - 1)) || (posrow > (paddle1 + 1))){
                    velcol = 0;
                    velrow = 0;
                    score2 += 1;
                }
                velcol = -velcol; //case where bounces
            }
        }
        else if(poscol == 8){
            if(velcol == 1){
                if(posrow == (paddle2 - 1)){
                    taskball.period = (ballperiod - ballperiod_incr * 7) * N;
                    velrow = -1;
                } else if(posrow == paddle2){
                    taskball.period = (ballperiod - ballperiod_incr * 7) * N;
                    velrow = -velrow;
                } else if(posrow == (paddle2 + 1)){
                    taskball.period = (ballperiod - ballperiod_incr * 7) * N;
                    velrow = 1;
                } else if((posrow < (paddle2 - 1)) || (posrow > (paddle2 + 1))){
                    velcol = 0;
                    velrow = 0;
                    score1 += 1;
                }
                velcol = -velcol; //case where bounces
            }
        }
    
        else if(poscol == 3){
            if((spin == 1) && (velcol < 0)){
                if(velrow == -1){
                    velrow = 0;
                }
                else if(velrow == 0){
                    velrow = 1;
                }
                else if(velrow == 1){
                    velrow = 0;
                }
            }
            spin = 0;
        }

        if((velcol == 0) && (velrow == 0)){
            replay();
        } else{
            taskball.period += ballperiod_incr * N;
            poscol += velcol;
            posrow += velrow;
        }
        break;
      default:
        break;
    }
    return state;
}

enum Paddle1 {movepad1};
int Paddle1(int state) {
    // Transitions
    if(gamewait > 0){
        return state;
    }
    switch (state) {
        case movepad1:
            break;
        default:
            state = movepad1;
            break;
    }
    // Actions
    switch (state) {
      case movepad1:
        ai = rand() % 2;
        if(0){
            if(ai == 0){ //don't move
                paddle1 += 0; 
            }
            else if(ai == 1){ //move in some direction
                ai = rand() % 2;
                if(ai == 0){ //move down
                    if((paddle1 == 2) || (paddle1 == 3)){
                        paddle1 += 1;
                    }
                }
                else if(ai == 1){ //move up
                    if((paddle1 == 3) || (paddle1 == 4)){
                        paddle1 -= 1;
                    }
                }
            }
        }
        else{
            if(keycnt > 0){ //move up
                if((paddle1 >= 3)){
                    paddle1 -= 1;
                    keycnt += 1;
                }
            }
            else if(keycnt < 0){ //move down
                if((paddle1 <= 3)){
                    paddle1 += 1;
                    keycnt -= 1;
                }
            }
        }
        break;
      default:
        break;
    }
    return state;
}

enum Paddle2 {movepad2};
int Paddle2(int state) {
    if(gamewait > 0){
        return state;
    }
    // Transitions
    switch (state) {
        case movepad2:
            break;
        default:
            state = movepad2;
            break;
    }
    // Actions
    switch (state) {
      case movepad2:
        if(0){
            ai = rand() % 2;
            if(ai == 0){ //don't move
                paddle2 += 0;
            }
            else if(ai == 1){ //move in some direction
                ai = rand() % 2;
                if(ai == 0){ //move down
                    if((paddle2 == 2) || (paddle2 == 3)){
                        paddle2 += 1;
                    }
                }
                else if(ai == 1){ //move up
                    if((paddle2 == 3) || (paddle2 == 4)){
                        paddle2 -= 1;
                    }
                }
            }
        }
        else{
            if((cnt < 0)){ //move up
                if((paddle2 >= 3)){
                    paddle2 -= 1;
                    cnt += 1;
                }
            }
            else if((cnt > 0)){ //move down
                if((paddle2 <= 3)){
                    paddle2 += 1;
                    cnt -= 1;
                }
            }
        }

        break;
      default:
        break;
    }
    return state;
}

enum OutputBall {outball};
int OutputBall(int state) {
    if(gamewait > 0){
        return state;
    }
    if(roundRobin == 4){
        roundRobin = 1;
        return state;
    } 
    if(roundRobin != 1){
        return state;
    }
    // Transitions
    switch (state) {
        case outball:
            column = 0x80 >> (poscol - 1);
            row = 0x01 << (posrow - 1);
            break;
        default:
            state = outball;
            break;
    }

    // Actions
    switch (state) {
      case outball:
         break;
      default:
        break;
    }
    
    roundRobin = 5;
    PORTC = column;    // column to display
    PORTD = ~row;   // Row(s) displaying pattern    
    return state;
}

enum OutputPad1 {outpad1};
int OutputPad1(int state) {
    if(gamewait > 0){
        return state;
    }
    if(roundRobin == 5){
        roundRobin = 2;
        return state;
    }
    if(roundRobin != 2){
        return state;
    }

    // Transitions
    switch (state) {
        case outpad1:
            column = 0x80;
            row = 0x07 << (paddle1 - 2);
            break;
        default:
            state = outpad1;
            break;
    }

    // Actions
    switch (state) {
      case outpad1:
         break;
      default:
        break;
    }

    roundRobin = 6;
    PORTC = column;    // column to display
    PORTD = ~row;   // Row(s) displaying pattern
    return state;
}

enum OutputPad2 {outpad2};
int OutputPad2(int state) {
    if(gamewait > 0){
        return state;
    }
    if(roundRobin == 6){
        roundRobin = 3;
        return state;
    } 
    if(roundRobin != 3){
        return state;
    }

    // Transitions
    switch (state) {
        case outpad2:
            column = 0x01;
            row = 0x07 << (paddle2 - 2);
            break;
        default:
            state = outpad2;
            break;
    }

    // Actions
    switch (state) {
      case outpad2:
         break;
      default:
        break;
    }

    roundRobin = 4;
    PORTC = column;    // column to display
    PORTD = ~row;   // Row(s) displaying pattern
    return state;
}

int main(void) {
    // Insert DDR and PORT initializations 
    DDRA = 0x00;  PORTA = 0x07;
    DDRB = 0xF0;  PORTB = 0x0F; //PC7...PC4 outputs init 0s, PC3...PC0 inputs init 1s
    DDRC = 0xFF;  PORTC = 0x00;
    DDRD = 0xFF;  PORTD = 0xFF;

    //Declare an array of tasks
    task *tasks[] = { &taskkeypad, &taskinput, &taskball, &taskpad1, &taskpad2, &taskob, &taskop1, &taskop2, &taskscore };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    // Task 1 (KeypadSM)
    taskkeypad.state = 0;//Task initial state.
    taskkeypad.period = 100;//Task Period.
    taskkeypad.elapsedTime = taskkeypad.period;//Task current elapsed time.
    taskkeypad.TickFct = &KeypadSM;//Function pointer for the tick.

    // Task (Score)
    taskscore.state = showscore;//Task initial state.
    taskscore.period = 100;//Task Period.
    taskscore.elapsedTime = 0;//Task current elapsed time.
    taskscore.TickFct = &Scoreboard;//Function pointer for the tick.

    // Task (Pad2Dir)
    taskinput.state = off;//Task initial state.
    taskinput.period = 100;//Task Period.
    taskinput.elapsedTime = 0;//Task current elapsed time.
    taskinput.TickFct = &Pad2Dir;//Function pointer for the tick.

    // Task (Ball)
    taskball.state = move;//Task initial state.
    taskball.period = ballperiod * N;//Task Period.
    taskball.elapsedTime = 0;//Task current elapsed time.
    taskball.TickFct = &Ball;//Function pointer for the tick.

    // Task (Paddle1)
    taskpad1.state = movepad1;//Task initial state.
    taskpad1.period = 130000 * N;//Task Period.
    taskpad1.elapsedTime = 0;//Task current elapsed time.
    taskpad1.TickFct = &Paddle1;//Function pointer for the tick.

    // Task (Paddle2)
    taskpad2.state = movepad2;//Task initial state.
    taskpad2.period = 130000 * N;//Task Period.
    taskpad2.elapsedTime = 0;//Task current elapsed time.
    taskpad2.TickFct = &Paddle2;//Function pointer for the tick.

    // Task (OutputBall)
    taskob.state = outball;//Task initial state.
    taskob.period = 100;//Task Period.
    taskob.elapsedTime = 0;//Task current elapsed time.
    taskob.TickFct = &OutputBall;//Function pointer for the tick.

    // Task (OutputPad1)
    taskop1.state = outpad1;//Task initial state.
    taskop1.period = 100;//Task Period.
    taskop1.elapsedTime = 0;//Task current elapsed time.
    taskop1.TickFct = &OutputPad1;//Function pointer for the tick.

    // Task (OutputPad2)
    taskop2.state = outpad2;//Task initial state.
    taskop2.period = 100;//Task Period.
    taskop2.elapsedTime = 0;//Task current elapsed time.
    taskop2.TickFct = &OutputPad2;//Function pointer for the tick.


    // Set the timer and turn it on
    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for ( i = 1; i < numTasks; i++ ) {
        GCD = findGCD(GCD,tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
    srand(time(0));
    reset();

    // Insert your solution below 
    while (1) {
        for ( i = 0; i < numTasks; i++ ) { // Scheduler code
            if ( tasks[i]->elapsedTime == tasks[i]->period ) { // Task is ready to tick
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); // Set next state
                tasks[i]->elapsedTime = 0; // Reset the elapsed time for next tick.
            }
            tasks[i]->elapsedTime += GCD;
        }

//        while(!TimerFlag); //Wait 1 sec
//        TimerFlag = 0;
    }
    return 0;
}
