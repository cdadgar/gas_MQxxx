/*
 * MQ-135 - benzene, alcohol, smoke 
 * 
 * seems like I need a MQ-2 - LPG, propane, hydrogen
 *  
 * Vcc - 5v  (NOT 3.3V capable) 
 * Gnd - ground
 * D0 - digital out
 *   adjust the pot.  when threshold is reached, this line goes high (5V)
 * A0 - 0V to 5V output
 * 
 * note:  to use this device with esp8266 or wemos, a voltage divider need to be used
 * see: https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_ESP8266.PNG
 * 5V is converted down to 3.3V
 * use 3.3Kohm and 1.7kohm resistors
 * 
 * note:  the device need to preheat before it will output accurat results
 * (preheat time?  at least 20 seconds)
 *  
 * library: https://github.com/miguel5612/MQSensorsLib
 * learning: https://components101.com/sensors/mq135-gas-sensor-for-air-quality
 * 
 * how to test?  for the 135, add smoke from and candle?
 */


//Include the library
#include <MQUnifiedsensor.h>

/************************Hardware Related Macros************************************/
#define         Board                   ("ESP8266")
#define         Pin                     (A0)  //Analog input 3 of your arduino
/***********************Software Related Macros************************************/
//#define         Type                    ("MQ-3") //MQ3
#define         Type                    ("MQ-135") //MQ135
#define         Voltage_Resolution      (3.3) // 3V3 <- IMPORTANT
#define         ADC_Bit_Resolution      (10)  // For ESP8266
#define         RatioMQ135CleanAir      (3.6) // RS / R0 = 3.6 ppm 
// is this right?  see https://www.teachmemicro.com/mq-135-air-quality-sensor-tutorial/#:~:text=The%20x%2Daxis%20is%20the,the%20sensor%20in%20various%20gases.&text=At%20PPM%20%3D%2010%2C%20the%20Rs,and%201.2589%20or%20100.1.
/*****************************Globals***********************************************/
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/*****************************Globals***********************************************/


void setup() {
  // start serial port
  Serial.begin(115200);
  Serial.print(F("\n\n MQxxx gas detector test\n\n"));
  
  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(110.47); MQ135.setB(-2.862); // Configurate the equation values to get CO2 concentration
  /*
    MQ-135 Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937  
  Alcohol  | 77.255 | -3.18 
  CO2      | 110.47 | -2.862
  Toluene  | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Acetone  | 34.668 | -3.369
  */
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ135.init(); 
 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ135.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  MQ135.serialDebug(true);
}

void loop() {
  MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
  float PPM = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  MQ135.serialDebug(); // Will print the table on the serial port
  delay(500); //Sampling frequency
}
