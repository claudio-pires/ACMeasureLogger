#ifndef CONTROLBUTTON_H_
#define CONTROLBUTTON_H_

#include "Util.h"

enum CONTROL { DOUBLE_CLICK, PRESSED, C_NORMAL, NONE };
enum CONTROL controlButton = NONE; // Estado inicial del boton de control es NONE

int buttonState;
int buttonState_prev;
unsigned long button_time = 0;
unsigned long button_time_prev = 0;
unsigned long tdif = 0;
bool flag1 = false;
bool flag2 = false;
int cpin;

void IRAM_ATTR isr() {
    button_time = millis();
    buttonState = digitalRead(cpin);
    tdif = button_time - button_time_prev;
    LOG_D("ControlButton generando ISR con button_time=%d, buttonState=%s, tdif=%d", button_time, (buttonState == HIGH?"HIGH":"LOW"), tdif);

    if((buttonState != buttonState_prev) && tdif > 10) {
        if(buttonState == LOW) {
            if (tdif < 150) {
                flag2 = true;
            } else {
                flag1 = false;
                flag2 = false;
            }
            Serial.print("DOWN @ ");
        } else {
            if(tdif > 1200) {
                Serial.print("LONG PRESSED ");
                controlButton = PRESSED;
                flag1 = false;
                flag2 = false;
                if(tdif > 10000) {
                    Serial.println("RESET");
                    ESP.reset();
                }
            } else {
                if (tdif < 150) {
                    if (flag1 && flag2) {
                        Serial.print("DOUBLE CLIC PRESSED ");
                        controlButton = DOUBLE_CLICK;
                        flag1 = false;
                        flag2 = false;
                    } else {
                        flag1 = true;
                    }
                } else {
                    Serial.print("NORMAL PRESSED ");
                    controlButton = C_NORMAL;
                    flag1 = false;
                    flag2 = false;
                }
            }
            Serial.print("UP @ ");
        }
        Serial.println(tdif);
        button_time_prev = button_time;
    } else {
        LOG_D("Se descarta por tiempo bajo o rebote")
    }
    buttonState_prev = buttonState;
}

void setControlButton(int pin) {
    cpin = pin;
    pinMode(cpin, INPUT);
    attachInterrupt(cpin, isr, CHANGE);
    button_time = millis();
    buttonState_prev = HIGH;
    button_time_prev = button_time;
}
enum CONTROL getControlStatus() {
    enum CONTROL r = controlButton;
    if(controlButton != NONE) {
        controlButton = NONE;
    }
    return r;
}

#endif /* CONTROLBUTTON_H_ */