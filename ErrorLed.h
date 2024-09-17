#ifndef ERRORLED_H_
#define ERRORLED_H_

#include "Util.h"

enum ERROR_E { E_NORMAL, E_ERROR};
int eled;                                 // pin number defined for this LED
enum ERROR_E emode;


// =====================================================================================
// StatusLed: Class for manage status led
// =====================================================================================
struct ErrorLed {  


    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // led           reference for the output pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    ErrorLed(int led) {
        eled = led;
        emode = E_NORMAL;
        pinMode(eled, OUTPUT);
        digitalWrite(eled, LOW);
}

    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // led           reference for the output pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    void setLedMode(enum ERROR_E mode) {
        LOG_D("mode=%d, emode=%d", mode, emode);

        if(mode != emode) {
            emode = mode;
            switch( emode ) {
                case E_NORMAL:
                    digitalWrite(eled, LOW);
                    break;
                case E_ERROR:
                    digitalWrite(eled, HIGH);
                    break;
            }
        }
  
    }
};

#endif /* ERRORLED_H_ */