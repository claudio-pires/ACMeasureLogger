#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <LiquidCrystal_I2C.h>
#include "Util.h"

#define BACKLIGHT_TIMER 10000
#define DISPLAY_COLS    16
#define DISPLAY_ROWS    2
#define DISPLAY_ADDRESS 0x27

// =====================================================================================
// Display: Class for manage LCD
// =====================================================================================
struct Display {  

    unsigned long backlightTimer;
    bool status;
    LiquidCrystal_I2C *_display; 

    // -----------------------------------------------------------------------------------
    // constructor() method
    // -----------------------------------------------------------------------------------
    Display(LiquidCrystal_I2C *_lcd) {
        this->_display = _lcd;
    }

     // -----------------------------------------------------------------------------------
    // constructor() method
    // -----------------------------------------------------------------------------------   
    void init() {
        _display->begin(DISPLAY_COLS, DISPLAY_ROWS);
        _display->init();
        _display->backlight();  
        status = true;

    }



    // -----------------------------------------------------------------------------------
    // constructor() method
    // -----------------------------------------------------------------------------------
    void Show(uint8_t lcd_col,uint8_t lcd_row, char* texto) {

        _display->setCursor(lcd_col, lcd_row); 
        _display->print(texto);
   }


    // -----------------------------------------------------------------------------------
    // constructor() method
    // -----------------------------------------------------------------------------------
    void Show(char* linea1, char* linea2) {
        _display->setCursor(0, 0); 
        _display->print(linea1);
        _display->setCursor(0, 1); 
        _display->print(linea2);
   }


     // -----------------------------------------------------------------------------------
    // constructor() method
    // -----------------------------------------------------------------------------------   
    void Refresh(unsigned long t) {
        //LOG_D("Llamando a Refresh con status=%d, t=%d y backlightTimer=%d", status, t, backlightTimer);
        if(status && (t > backlightTimer)) {
            _display->noBacklight();  
            status = false;
        }
    }

     // -----------------------------------------------------------------------------------
    // constructor() method
    // -----------------------------------------------------------------------------------   
    void setBacklight(unsigned long t) {
        if(!status) {
            _display->backlight();  
            status = true;
        }
        backlightTimer = t + BACKLIGHT_TIMER;
    }
};

#endif /* DISPLAY_H_ */