#ifndef STATUSLED_H_
#define STATUSLED_H_

enum STATUS_E { INITIALIZING, NORMAL, ERROR, AP , OFF};

long ledH = 2500000;
long ledL = 2500000;
int sled;                                 // pin number defined for this LED


// -----------------------------------------------------------------------------------
// ISR to Fire when Timer is triggered
// -----------------------------------------------------------------------------------
void ICACHE_RAM_ATTR onTime() {
  if(digitalRead(sled)) {
	  timer1_write(ledL);
    digitalWrite(sled, LOW);
  } else {
	  timer1_write(ledH);
    digitalWrite(sled, HIGH);
  }
}


// =====================================================================================
// StatusLed: Class for manage status led
// =====================================================================================
struct StatusLed {  


    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // led           reference for the output pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    StatusLed(int led) {
        sled = led;


        pinMode(led, OUTPUT);
        //Initialize Ticker every 0.5s
        timer1_attachInterrupt(onTime); // Add ISR Function
        timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);  /*  Dividers:
                                                                TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
                                                                TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)
                                                                TIM_DIV256 = 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
                                                             Reloads:
                                                                TIM_SINGLE	0 //on interrupt routine you need to write a new value to start the timer again
                                                                TIM_LOOP	1 //on interrupt the counter will start with the same value again
                                                            */
        // Arm the Timer for our 0.5s Interval
        timer1_write(ledH); // 2500000 / 5 ticks per us from TIM_DIV16 == 500,000 us interval 
    }

    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // led           reference for the output pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    void setLedMode(enum STATUS_E mode) {
  
        switch( mode ) {
            case AP:
                ledH = 2500000;
                ledL = 2500000;
                break;
            case NORMAL:
                ledH = 25000000;
                ledL = 250000;
                break;
            case ERROR:
                ledH = 250000;
                ledL = 25000000;
                break;
            case INITIALIZING:
                ledH = 500000;
                ledL = 500000;
                break;
        }
    }
};

#endif /* STATUSLED_H_ */
