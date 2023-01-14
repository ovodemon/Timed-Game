/**********************************************************************/
// ENGR-2350 Lab2-Timed Game
// Name: Yijia Zhou
// RIN: 661995479,
/**********************************************************************/

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIOInit();
void TimerInit();
void Game_starting_logic();
void check_pb();
void check_bmp_press();
void get_bmp_num();
void Timer_ISR();
void Timer_reset();
void Timer_delay(uint16_t ms);
void RGB_display(uint8_t len);
void RGB_array(uint8_t num);
void RGB_reset();
void RGB_quick_control();
void RGB_advanced_control();
void RGB_control();
void BiLed_green();
void BiLed_red();
void BiLed_off();
void over_time();
void incorrect();
void you_win();

// Add global variables here, as needed.
Timer_A_UpModeConfig Timer;
uint8_t array[10] = {0,1,2,3,4,5,4,3,2,1};
uint8_t round = 0;
uint16_t timer_resets = 0;
uint8_t pb;
uint8_t bmp;
uint8_t bmp_num;
uint8_t overtime = 0;
uint8_t correct = 5;


int main(void) /* Main Function */
{
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();
    // Place initialization code (or run-once) code here
    GPIOInit();
    RGB_reset();

    while(1){
        // Place code that runs continuously in here
        correct = 5;
        overtime = 0;
        round = 1;
        printf("Press Pushbutton to START!\r\n");
        Game_starting_logic();
        printf("Game Start\r\n");
        TimerInit();
        while(round < 11){
            BiLed_red();
            Timer_reset();
            while (timer_resets < 8){  // wait 1 second
            }

            // RGB_display logic
            RGB_display(round);

            // Press logic
            BiLed_green();
            printf("Press BMP\r\n");
            uint8_t a = 0;
            while (1){
                __delay_cycles(240e3);
                if (a > (round-1)){
                    break;
                }
                overtime = 0;
                correct = 5;
                check_pb();
                while (pb){
                    Timer_A_stopTimer(TIMER_A0_BASE);
                    RGB_control();
                    check_pb();
                }
                Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
                if (timer_resets >= 24){
                    overtime = 1;
                    break;
                }
                get_bmp_num();
                if (bmp){
                    printf("bmp = %u", bmp_num);
                }
                RGB_control();
                check_pb();
                if (bmp && pb){
                    check_pb();
                    while (pb){
                        Timer_A_stopTimer(TIMER_A0_BASE);
                        RGB_control();
                        check_pb();
                        bmp = 0;
                    }
                    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
                }
                if (bmp && !pb){
                    if (bmp_num == array[a]){
                        correct = 1;
                        bmp = 0;
                        Timer_reset();
                        a ++;
                        continue;
                    }else {
                        correct = 0;
                        break;
                    }
                }
            }
            if (overtime){
                break;
            }
            if (correct == 0){
                break;
            }
            round ++;
        }
        if (overtime){
            over_time();
        }else if (correct == 0){
            incorrect();
        }else {
            you_win();
        }
    }
}

// Add function declarations here as needed
void GPIOInit(){
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN0|GPIO_PIN1|GPIO_PIN2); // RGB LED
    GPIO_setAsInputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN2|GPIO_PIN3|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7); // BMP
    GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN5); // Pushbutton
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN6|GPIO_PIN7); // BiLed
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN3);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN6);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6|GPIO_PIN7);
}

void TimerInit(){
    Timer.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    Timer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64; // Divider 64
    Timer.timerPeriod = 46874; // Period = 125ms
    Timer.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A0_BASE, &Timer);
    // Registering the ISR...
    Timer_A_registerInterrupt(TIMER_A0_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Timer_ISR);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

void Game_starting_logic(){
    uint8_t pb = GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5);
    while (1){
        RGB_control();
        pb = GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5);
        if (pb == 1){
            break;
        }
    }
}

void check_pb(){
    pb = GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5);
}

void check_bmp_press(){
    bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0) ||
            !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2) ||
            !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3) ||
            !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5) ||
            !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6) ||
            !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
}

void get_bmp_num(){
    check_bmp_press();
    if (bmp){
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)){
            bmp_num = 0;
        }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2)){
            bmp_num = 1;
        }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)){
            bmp_num = 2;
        }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)){
            bmp_num = 3;
        }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6)){
            bmp_num = 4;
        }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)){
            bmp_num = 5;
        }
    }
}

void Timer_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);   // acknowledge the interrupt
    timer_resets ++;
}

void Timer_reset(){
    timer_resets = 0;
    Timer_A_clearTimer(TIMER_A0_BASE);
}

void Timer_delay(uint16_t ms){
    uint16_t count = ms / 125;
    Timer_reset();
    while (timer_resets < count){
    }
}

void RGB_display(uint8_t len){
    uint8_t ctr = 0;
    timer_resets = 0;
    for(ctr=0;ctr<len;ctr++){
        while (timer_resets < 4){
            RGB_array(array[ctr]);
        }
        RGB_reset();
        timer_resets = 0;
        while (timer_resets < 4){
        }
        timer_resets = 0;
    }
}

void RGB_array(uint8_t num){
    if (num == 0){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // RGB Red
    }else if (num == 1){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1|GPIO_PIN0); // RGB Yellow
    }else if (num == 2){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);        // RGB Blue
    }else if (num == 3){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);        // RGB Green
    }else if (num == 4){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN1);// RGB Cyan
    }else if (num == 5){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN0);// RGB Purple
    }
}

void RGB_reset(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
}

void RGB_quick_control(){
    if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)){ // BMP0
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // RGB Red
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2)){ // BMP1
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1|GPIO_PIN0); // RGB Yellow
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)){ // BMP2
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);        // RGB Blue
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)){ // BMP3
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);        // RGB Green
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6)){ // BMP 4
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN1);// RGB Cyan
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)){ // BMP 5
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN0);// RGB Purple
    }
}

void RGB_advanced_control(){
    check_bmp_press();
    while (bmp){
        __delay_cycles(240e3);
        RGB_quick_control();
        check_pb();
        while (pb){
            Timer_A_stopTimer(TIMER_A0_BASE);
            check_bmp_press();
            while (bmp){
                RGB_quick_control();
                check_bmp_press();
                check_pb();
                if (!pb){
                    break;
                }
            }
            check_pb();
        }Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
        get_bmp_num();
        if (timer_resets >= 24){
            overtime = 1;
            break;
        }
    }RGB_reset();
}

void RGB_control(){
    uint8_t bmp;
    if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)){ // BMP0
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // RGB Red
        bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
        while (bmp){
            __delay_cycles(240e3);
            bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
        }
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2)){ // BMP1
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1|GPIO_PIN0); // RGB Yellow
        bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2);
        while (bmp){
            __delay_cycles(240e3);
            bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2);
        }
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1|GPIO_PIN0);
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)){ // BMP2
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);        // RGB Blue
        bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3);
        while (bmp){
            __delay_cycles(240e3);
            bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3);
        }
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)){ // BMP3
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);        // RGB Green
        bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5);
        while (bmp){
            __delay_cycles(240e3);
            bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5);
        }
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6)){ // BMP 4
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN1);// RGB Cyan
        bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6);
        while (bmp){
            __delay_cycles(240e3);
            bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6);
        }
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1|GPIO_PIN2);
    }else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)){ // BMP 5
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN0);// RGB Purple
        bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
        while (bmp){
            __delay_cycles(240e3);
            bmp = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
        }
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2|GPIO_PIN0);
    }
}

void BiLed_green(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN7);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN6);
}

void BiLed_red(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN7);
}

void BiLed_off(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6|GPIO_PIN7);

}

void over_time(){
    printf("Over Time!\r\n");
    check_pb();
    while (!pb){
        check_pb();
        BiLed_red();
        check_pb();
        Timer_delay(250);
        check_pb();
        BiLed_off();
        check_pb();
        Timer_delay(250);
        check_pb();
    }
}

void incorrect(){
    printf("Incorrect!\r\n");
    check_pb();
    while (!pb){
        check_pb();
        BiLed_red();
        check_pb();
        Timer_delay(250);
        check_pb();
        BiLed_off();
        check_pb();
        Timer_delay(250);
        check_pb();
    }
}

void you_win(){
    printf("You Win!\r\n");
    check_pb();
    while (!pb){
        check_pb();
        BiLed_green();
        check_pb();
        Timer_delay(250);
        check_pb();
        BiLed_off();
        check_pb();
        Timer_delay(250);
        check_pb();
    }
}
// Add interrupt functions last so they are easy to find
