/* This code works with ESP8266 12E and ZMPT101B AC voltage sensor
 * It can measure the TRMS of Any voltage up to 250 VAC 50/60Hz and send the values to Adafruit MQTT
 * Refer to www.SurtrTech.com for more details 
 */

#include <ESP8266WiFi.h>                         //Libraries needed
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Filters.h>
#include "StatusLed.h"
#include "ErrorLed.h"
#include "ControlButton.h"
#include "Display.h"
#include "Util.h"

#define CONTROL_LED    13
#define ERROR_LED      12
#define CONTROL_BUTTON 14
#define ZMPT101B       A0   //ZMPT101B analog pin



#define WLAN_SSID       "DelProgreso2"             //Your WiFi SSID and Passcode
#define WLAN_PASS       "Argentina1978"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                  

#define AIO_USERNAME    "claudiopires"
#define AIO_KEY         ""
#define AIO_FEED        "/feeds/Del_Progreso_1031_-_Mediciones_De_Tension"

#define REFERENCIA      220.0
 
int TiempoEntreRegistros = 10000; // Se registra cada 10 segundos
int TiempoEntreRegistros2 = 1000; // Se registra cada 10 segundos

bool s = true;

char buffer[20];

int Current_Time=0, Previous_Time=0, Previous_Time2=0;  //We send a value every 10s, the maximum is 30 value per minute

float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 100/testFrequency;      // how long to average the signal, for statistist
float Desviacion;
int RawValue = 0;

float intercept = 0; // to be adjusted based on calibration testing
float slope = 1;    // to be adjusted based on calibration testing
float Volts_TRMS;   // estimated actual voltage in Volts

//For more details about the intercept and slope check the Base code at SurtrTech

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish AdafruitMQTT = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME AIO_FEED);        
RunningStatistics inputStats;

LiquidCrystal_I2C lcd(DISPLAY_ADDRESS, DISPLAY_COLS, DISPLAY_ROWS);
Display display(&lcd);
StatusLed statusLed(CONTROL_LED);
ErrorLed errorLed(ERROR_LED);


// -------------------------------------------------------------------------------------------------------------
//  setup() method
// -------------------------------------------------------------------------------------------------------------
void setup() {
  
    statusLed.setLedMode(INITIALIZING);                     // Enciende LED de estado indicando inicializacion
    errorLed.setLedMode(E_NORMAL);                          // inicializa indicador de error a normal
    setControlButton(CONTROL_BUTTON);                       // Activa el boton de control
    pinMode(ZMPT101B,INPUT);

    Serial.begin(115200);                                   // Inicializa serial
    display.init();                                         // Inicializa display
 
    display.Show((char *)"Connecting to", (char *)"WIFI...");       // Muestra mensaje conexion a WIFI
                                                                    // en el display

    LOG_D("Connecting to %s", WLAN_SSID);

    WiFi.begin(WLAN_SSID, WLAN_PASS);                       // Intenta conectarse al Wifi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    statusLed.setLedMode(NORMAL);                           // Logra conectarse entonces cambia el estado del led
                                                            // luego muestra IP por display
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    LOG_D("WiFi connected! my IP Address is %s", buffer);
    display.Show((char *)"WiFi connected", buffer);
    delay(5000);                                            // Muestra IP por 5 segundos
    display.setBacklight(millis());                         // Programa el backlight para que se apague luego del timeout

    inputStats.setWindowSecs( windowLength );               // configura el modulo de estadisticas

}


// -------------------------------------------------------------------------------------------------------------
//  loop() method
// -------------------------------------------------------------------------------------------------------------
void loop() {

    
    Volts_TRMS = ReadVoltage();                                     // Lee voltaje
    LOG_D("Se leyeron %.2f volt", Volts_TRMS);

    Current_Time = millis();                                        // Obtiene referencia de tiempo 

    if(Current_Time - Previous_Time >= TiempoEntreRegistros) {      // Every period we send the value to the service provider
        MQTT_connect();                                             // Keep the MQTT connected
        s = AdafruitMQTT.publish(Volts_TRMS);                       // Publish on Adafruit

        LOG_D("Sending Value %f %s...", Volts_TRMS, (s?"OK":"Failed"));  
        Previous_Time = Current_Time;
        sprintf(buffer, "    %3.1f volt     ", Volts_TRMS);
        display.Show(0, 0, buffer);
        if(!s) {
            display.Show(15, 0, (char *)"*");
        }
        Desviacion = 100 * (REFERENCIA  - Volts_TRMS) / REFERENCIA;
        if(abs(Desviacion) > 10 ) {
            if(Desviacion < 0) {
                display.Show(0, 1, (char *)"  SOBRETENSION  ");
                errorLed.setLedMode(E_ERROR);
            } else {
                display.Show(0, 1, (char *)"  BAJA TENSION  ");
                errorLed.setLedMode(E_ERROR);
            }
        } else {
            display.Show(0, 1, (char *)"     NORMAL     ");
            errorLed.setLedMode(E_NORMAL);
        }
        Previous_Time2 = Current_Time;
    }

    // Si se hizo click simple se enciende el backlight y se inicializa el timer
    if(getControlStatus() == C_NORMAL) {
        display.setBacklight(Current_Time);
    }
    display.Refresh(Current_Time);
 
    //delay(1000);                // Se mide cada un segundo
}

// -----------------------------------------------------------------------------------
// MQTT_connect() method
//
// Auto reconnect to MQTT, otherwise it will trigger a watchdog reset
// -----------------------------------------------------------------------------------
void MQTT_connect() {                  //
  int8_t ret;

  // Stop if already connected.
   if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}



// -----------------------------------------------------------------------------------
// ReadVoltage() method
//
// Devuelve el valor de tension medido RMS
// -----------------------------------------------------------------------------------
float ReadVoltage() {
  
      //LOG_D("Obteniendo medicion de tension");

      int RawValue = analogRead(ZMPT101B);  // read the analog in value:
      inputStats.input(RawValue);  // log to Stats function
     
      float Volts_TRMS = inputStats.sigma()* slope + intercept;
     // Volts_TRMS = Volts_TRMS*0.979;

      return Volts_TRMS;
}

