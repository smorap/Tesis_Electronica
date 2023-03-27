/* Ruler 1         2         3         4         5         6         7        */

/*****************************  Nodo_Sensor_1.ino  ****************************/
/*                                                                            */
/*   Purpose: Nodo Sensor 1 para sistema de monitoreo                         */
/*                                                                            */
/*   Origin:  Escrito por Sergio Mora y Miguel Suarez, 9 septiembre, 2022     */
/*                                                                            */
/*   e-mail:  s_mora@javeriana.edu.co  miguel-suarez@javeriana.edu.co         */
/*                                                                            */
/*   Pendientes: Ninguno                                                      */                              
/*                                                                            */
/*   FECHA      RESPONSABLE   COMENTARIO                                      */
/*   -----------------------------------------------------------------------  */
/******************************************************************************/


/************************** Biliotecas de Funciones ***************************/
#include <RF24.h>                     // Para el módulo NRF24L01
#include "DHT.h"                      // Para el módulo DHT22
#include "HX711.h"                    // Para el módulo HX711
#include <SPI.h>                      // Para el módulo HX711
/******************************************************************************/

/************************** Definición de los pines ****************************/
#define CE_PIN 9                  // Pin CE  del módulo NRF24L01*
#define CSN_PIN 10                // Pin CSN del módulo NRF24L01*
#define DHTPIN 2                  // Pin OUT  el módulo DHT22  
#define CLK   A0                  // Pin CLK  el módulo HX711  
#define DOUT  A1                  // Pin DOUT el módulo HX711   
#define CLK2  A2                  // Pin CLK  el módulo HX711 
#define DOUT2 A3                  // Pin DOUT el módulo HX711   

/* *Los pines SCK, MOSI y MISO del NRF24L01 están definidos por defecto dentro*/
/* de la biblioteca de funciones. Para el micro Atmega328p son:              */
/*                             SCK:  Pin 13                                   */ 
/*                             MISO: Pin 12                                   */
/*                             MOSI: Pin 11                                   */
/******************************************************************************/

/*************************** Definición de tiempos *****************************/
#define Segundos 5                           // Contador de segundos usado para definir cada cuanto tiempo se leen los sensores
#define Promedio 2                           // Contador usados para definir la cantidad de datos a promediar 
#define tiempo_muestreo 913                  // Contador de tiempo usado para definir le periodo de envío de los datos promediados
/******************************************************************************/

/********************** Definición tipo de sensor DHT **************************/
#define DHTTYPE DHT22                       // Sensor DHT22 
/******************************************************************************/

/**************************** Asignación de Pines *****************************/
RF24 radio(CE_PIN, CSN_PIN);                // Para el módulo RF NRF24L01
DHT dht(DHTPIN, DHTTYPE);                   // Para el módulo DHT22
/******************************************************************************/

/*******************Declaración dirección nodo sensor 2********+************/
byte direccion[5] ={'x','a','n','a','1'};  //Se define la dirección con la que se va a idenficiar este Nodo Sensor 
/******************************************************************************/

/******************************Máquina de estados******************************/
enum estados {LECTURA, ESPERA, ENVIO, ENVIO_OK, ENVIO_NO_OK, TIMER};      // Se crean los estados del sistema
enum estados ESTADO;                       
/******************************************************************************/

/*****************************Declaración variables****************************/
int16_t datos[5];   // Vector en le que se almacenan los datos leídos de los sensores

//Vectores usados para guardas los datos de cada sensor que se van a promediar
int16_t agua [Promedio];                    
int16_t comida [Promedio];
int16_t humedad [Promedio];
int16_t temperatura [Promedio];

int tiempo_transcurrido = 0;                //Contador de timepo usado para determinar cuándo se deben enviar los promedios
int Promedios_tomados = 0;                  //Cuenta los promedios 
volatile int timer=0;                       // Contador de timer (Segundos)
/******************************************************************************/

/**********************************Config HX711********************************/
HX711 balanza;
HX711 balanza2;
/******************************************************************************/

/************************************SET UP************************************/
void setup()
{
/******************************módulo RF NRF24L01******************************/  
  radio.begin();                            // Inicializa el NRF24L01
  radio.setPALevel(RF24_PA_MAX);            //configura la máxima potencia de transmisión
  radio.openWritingPipe(direccion);         // Se abre el canal de escritura 
/*********************************módulo DHT22*********************************/ 
  dht.begin();                              // inicializa el DHT22
/***********************************módulo HX711*******************************/ 
  balanza.begin(DOUT, CLK);                 // inicializa el HX711
  balanza.set_scale(-1168.795789);          // Se establece la escala previamente obtenida de la calibración
  balanza.set_offset(466501);               // Se establece el offset previamente obtenido de la calibración
  /***********************************módulo HX711*******************************/ 
  balanza2.begin(DOUT2, CLK2);              // inicializa el HX711
  balanza2.set_scale(-1151.308496);         // Se establece la escala previamente obtenida de la calibración
  balanza2.set_offset(-175095);             // Se establece el offset previamente obtenido de la calibración
/**************************Configuración del TIMER0****************************/
  TCCR1A = TCCR1B = 0;      // eliminar configuraciones anteriores 
  TCNT1 = 0;                // eliminar configuraciones anteriores 
  OCR1B = 15625;            // conteo para tener un periodo de 1 segundo
  TCCR1B |= (1 << WGM12);   // MODO DE COMPARACIÓN 
  TIMSK1 |= (1 << OCIE1B);  // INTERRUPCIONES
  TCCR1B |= (1 << CS12);    // PRESCALER 1024  
  TCCR1B |= (1 << CS10);    // PRESCALER 1024 
/***************************** Estado Inicial *********************************/
  ESTADO=LECTURA; //Se define el estado inicial
}
/******************************************************************************/


/*************************************LOOP*************************************/
void loop(){
/******************************Máquina de estados******************************/
  switch (ESTADO){                          // Selecciona el estado actual
/*-------------------------------Estado LECTURA: -----------------------------*/     
  case LECTURA:   
    dht.begin();                                      //inicializa el DHT22                
    datos[0] = balanza.get_units(10);                 // Se obtiene cantidad de agua
    datos[1] = balanza2.get_units(10);                // Se obtiene cantidad de comida
    datos[2] = (dht.readHumidity()*1.4295-10.18)*10;  // Se obtiene valor de humedad con su respectivo ajuste de calibración y se multiplica x10 para no usar variables de tipo flotante
    datos[3] = (dht.readTemperature()*0.99-2.1)*10;   // Se obtiene valor de temperatura con su respectivo ajuste de calibración y se multiplica x10 para no usar variables de tipo flotante
    datos[4] = 0;                                     // Se pone la cuarta posisción del vector en 0, indicando que son datos de "tiempo real"
    
    radio.write(datos, sizeof(datos));                // Se envian los datos por RF
    
    if (Promedios_tomados < Promedio){                // Se verifica si ya se tomaron todos los datos para el promedio
      // Si no se han tomado, se agregan los datos recien leídos a los vectores de promedios       
      agua[Promedios_tomados]= datos [0];               
      comida[Promedios_tomados]= datos [1];            
      humedad[Promedios_tomados]= datos [2];            
      temperatura[Promedios_tomados]= datos [3];      
      Promedios_tomados ++;
      ESTADO=TIMER; // Se pasa al estado TIMER
    }else{
      // Si ya se tomaron todos los datos de promedio se pasa al estado ESPERA
      ESTADO=ESPERA;
    }
    
                                   
  break; 
/*----------------------------------------------------------------------------*/

/*------------------------------Estado ESPERA: --------------------------------*/   
  case ESPERA:  
    // Una vez se hayan tomado todos los datos de promedio se pregunta si ya se cumplió el tiempo de espera para enviar los datos de promedios
    if (tiempo_transcurrido >= tiempo_muestreo){
      ESTADO=ENVIO;
    }else{
      ESTADO = TIMER;
    }
  break;
/*----------------------------------------------------------------------------*/

/*------------------------------Estado ENVIO: --------------------------------*/   
  case ENVIO:  
    // se borran los datos anteriores
    datos[0] = 0; 
    datos[1] = 0;
    datos[2] = 0;
    datos[3] = 0;
    
    for(int i=0; i< Promedio; i++){
      // se suman todas las lecturas
      datos[0] = datos[0] + agua[i];
      datos[1] = datos[1] + comida[i];
      datos[2] = datos[2] + humedad[i];
      datos[3] = datos[3] + temperatura[i];
    }
    // Se divide en la cantidad de lecturas para obtener el promedio 
    datos[0] = datos[0]/Promedio;  
    datos[1] = datos[1]/Promedio;
    datos[2] = datos[2]/Promedio;
    datos[3] = datos[3]/Promedio;
    //Se pone la cuarta posición del vector en 1, indicando que son datos de promedios
    datos[4] = 1;                 
    
    radio.write(datos, sizeof(datos)); // Se envían los datos por RF
    
    ESTADO=TIMER; 
  break;
/*----------------------------------------------------------------------------*/

/*--------------------------------Estado TIMER: -------------------------------*/   
  case TIMER:
    if(timer>=Segundos){                  //El sistema espera en este estado hasta que se cumpla el tiempo definido en la variable "timer"
    ESTADO=LECTURA;                       // Se actualiza el estado actual
    timer=0;                              
    }else{                               
    ESTADO=TIMER;                         
    }
  break;
  }
/*----------------------------------------------------------------------------*/
/***************************Fin máquina de estados*****************************/
}
/******************************************************************************/

/**************************Interrupciones TIMER0*******************************/
ISR(TIMER1_COMPB_vect){
  TCNT1 = 0;              // Reinicia el contador del Timer 1
  tiempo_transcurrido ++; 
  timer ++;               
}
/******************************************************************************/
