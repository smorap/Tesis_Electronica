/* Ruler 1         2         3         4         5         6         7        */

/*****************************  Nodo_Central.ino  *****************************/
/*                                                                            */
/*   Purpose: Modulo de comunicacion maestro para sistema de monitoreo        */
/*                                                                            */
/*   Origin:  Escrito por Sergio Mora y Miguel Suarez, 9 septiembre, 2022     */
/*                                                                            */
/*   e-mail:  s_mora@javeriana.edu.co y miguel-suarez@javeriana.edu.co        */
/*                                                                            */
/*   Pendientes: Ninguno                                                      */                              
/*                                                                            */
/*   FECHA      RESPONSABLE   COMENTARIO                                      */
/*   -----------------------------------------------------------------------  */
/*   Sep 09/22  Sergio Mora   Adecuación del formato con comentarios          */
/******************************************************************************/

/************************** Biliotecas de Funciones ***************************/
#include <RF24.h>                 // Para el modulo NRF24L01
#include <SoftwareSerial.h>       // Para el modulo BT HC-06
/******************************************************************************/

/************************** Defincion de los pines ****************************/
#define CE_PIN 9                  // Pin CE  del modulo NRF24L01*
#define CSN_PIN 10                // Pin CSN del modulo NRF24L01*
#define BT_RX 0                   // Pin Rx del modulo BT HC-06
#define BT_TX 1                   // Pin Tx del modulo BT HC-06
/* *Los pines SCK, MOSI y MISO del NRF24L01 están definidos por defecto dentro*/
/* de la bliblioteca de funciones. Para el micro Atmega328p son:              */
/*                             SCK:  Pin 13                                   */ 
/*                             MISO: Pin 12                                   */
/*                             MOSI: Pin 11                                   */
/******************************************************************************/

/*************************** Defincion de tiempos *****************************/
#define Segundos 2                         // un conteo = 1 segundo
/******************************************************************************/

/**************************** Asignación de Pines *****************************/
RF24 radio(CE_PIN, CSN_PIN);                // Para el modulo RF NRF24L01
SoftwareSerial BT(BT_RX,BT_TX);             // Para el modulo BT HC-06
/******************************************************************************/

/*******************Declaración direcciones nodos sensores********+************/
byte direccion[5] ={'y','a','n','a','1'};   //Dirección Nodo sensor 1 
byte direccion2[5] ={'x','a','n','a','1'};  //Dirección Nodo sensor 2 
/******************************************************************************/

/*****************************Declaración variables****************************/
int16_t datos_CH1[5];                   //Vector de datos en donde se recibe información del Nodo Sensor 1
int16_t datos_CH2[5];                   //Vector de datos en donde se recibe información del Nodo Sensor 2
int16_t promedios_CH1 [360] {};         //Vector de datos en donde se guardan los promedios del Nodo Sensor 1 (90 datos por cada variable sensada)
int16_t promedios_CH2 [360] {};         //Vector de datos en donde se guardan los promedios del Nodo Sensor 2 (90 datos por cada variable sensada)
int16_t promedios_actuales_CH1 [8] {};  //Vector de datos en donde se guarda el último dato de promedio del Nodo Sensor 1
int16_t promedios_actuales_CH2 [8] {};  //Vector de datos en donde se guarda el último dato de promedio del Nodo Sensor 2

int8_t canal;                           //Variable en la que se guarda en canal por dónde llega el paquete de datos del módulo de comunicación NRF24L01

//Variables que indican las posiciones en las que se guardan los datos promediados de agua, comida, humedad y temperatura del Nodo Sensor 1
int16_t posicion_datos_agua_CH1 = 0;          //En las primeras 90 posiciones del vector "promedios_CH1" se guardan los promedios del agua
int16_t posicion_datos_comida_CH1 = 90;       //En las siguientes 90, los promedios de comida
int16_t posicion_datos_humedad_CH1 = 180;     // ""
int16_t posicion_datos_temperatura_CH1 = 270; // ""

//Variables que indican la posiciones en las que se guardan los datos promediados de agua, comida, humedad y temperatura del Nodo Sensor 2
int16_t posicion_datos_agua_CH2 = 0;          //Se maneja la misma estructura que para el Nodo Sensor 1
int16_t posicion_datos_comida_CH2 = 90;
int16_t posicion_datos_humedad_CH2 = 180;
int16_t posicion_datos_temperatura_CH2 = 270;


volatile int timer=0;                   //Contador que aumenta cada segundo usado como variable de control en el sistema
/******************************************************************************/

/******************************Máquina de estados******************************/
enum estados {RADIO_FIFO, CANAL, CH1, CH2 , B_T, TIMER};  //Se crean los estados del sistema 
enum estados ESTADO;                                      
/******************************************************************************/

/************************************SET UP************************************/
void setup()
{
/*******************************Modulo BT HC-06 *******************************/
  BT.begin(57600);                         // Inicializa el puerto serial para el módulo BT 
/******************************Modulo RF NRF24L01******************************/  
  radio.begin();                          //inicializa el NRF24L01
  radio.setPALevel(RF24_PA_MAX);          //configura la máxima potencia de transmisión 
  radio.openReadingPipe(2, direccion);    //Se abre el canal de lectura con la dirección del Nodo Sensor 1
  radio.openReadingPipe(1, direccion2);   //Se abre el canal de lectura con la dirección del Nodo Sensor 2
  radio.startListening();                 //Empieza a "escuchar" por los canales
/**************************Configuracion del TIMER0****************************/
  TCCR1A = TCCR1B = 0;      // eliminar configuraciones anteriores 
  TCNT1 = 0;                // eliminar configuraciones anteriores 
  OCR1B = 15625;            // conteo para tener un periodo de 1 segundo
  TCCR1B |= (1 << WGM12);   // MODO DE COMPARACIÓN 
  TIMSK1 |= (1 << OCIE1B);  // INTERRUPCIONES
  TCCR1B |= (1 << CS12);    // PRESCALER 1024  
  TCCR1B |= (1 << CS10);    // PRESCALER 1024 
/*****************************Maquina de estados ******************************/  
  ESTADO=RADIO_FIFO;                // Declaración estado incial 
}
/******************************************************************************/


/*************************************LOOP*************************************/
void loop() {
/******************************Maquina de estados******************************/
  switch (ESTADO){
/*-----------------------------Estado: RADIO_FIFO-----------------------------*/
  case RADIO_FIFO: 
    if( radio.available(&canal)){ //Pregunta si han llegado datos por algún canal 
      ESTADO=CANAL; //Si es así, pasa al estado CANAL
    }else{
    ESTADO=TIMER;//De lo contrario pasa al estado TIMER
    }
  break;
/*----------------------------------------------------------------------------*/

/*--------------------------------Estado: CANAL-------------------------------*/
  case CANAL://Una vez han llegado datos pregunta por el canal por el que llegaron y pasa al estado correspondiente (CH1 o CH2)
    if (canal == 1){
      ESTADO=CH1;
    }else if(canal == 2) {
      ESTADO=CH2;    
    }else{//Si llegó por un canal inesperado, pasa al estado TIMER
      ESTADO=TIMER;
    }
  break;
/*----------------------------------------------------------------------------*/
/*----------------------------------Estado: CH1-------------------------------*/
  case CH1: //Una vez se sepa que el paquete de datos llegó del canal 1, se guardan los datos y se verifica 
            //si los datos corresponden a promedios o datos en "tiempo real"
            
    radio.read(datos_CH1,sizeof(datos_CH1)); //Función que lee y guarda los datos en el vector pasado como parámetro
    if (datos_CH1[4] == 1){//Si el dato en la posición 4 del vector recibido es 1, los datos son de promedios
      if(posicion_datos_agua_CH1 < 89){//Si han llegado menos de 89 datos de promedio, se guardan los datos en la siguiente posición
        promedios_CH1[posicion_datos_agua_CH1]        = datos_CH1[0];
        promedios_CH1[posicion_datos_comida_CH1]      = datos_CH1[1];
        promedios_CH1[posicion_datos_humedad_CH1]     = datos_CH1[2];
        promedios_CH1[posicion_datos_temperatura_CH1] = datos_CH1[3];
        
        //Se actualizan los promedios actuales
        promedios_actuales_CH1[0]=datos_CH1[0];
        promedios_actuales_CH1[1]=datos_CH1[1];
        promedios_actuales_CH1[2]=datos_CH1[2];
        promedios_actuales_CH1[3]=datos_CH1[3];

        //Se aumentan las posicones para la siguiente recepción
        posicion_datos_agua_CH1 ++;
        posicion_datos_comida_CH1 ++;
        posicion_datos_humedad_CH1 ++;
        posicion_datos_temperatura_CH1 ++;
      }else{//Si es espacio reservado para cada variable sensada está lleno se hace un corrimiento de datos,
            //eliminando la muesta más antigua y guardando la más reciente 
        for (int i=0; i<89; i++){
          promedios_CH1 [i] = promedios_CH1 [i+1];
          promedios_CH1 [i + 90]  = promedios_CH1 [i + 1 + 90];
          promedios_CH1 [i + 180] = promedios_CH1 [i + 1 + 180];
          promedios_CH1 [i + 270] = promedios_CH1 [i + 1 + 270];
        }
        promedios_CH1[posicion_datos_agua_CH1 -1]        = datos_CH1[0];
        promedios_CH1[posicion_datos_comida_CH1 -1]      = datos_CH1[1];
        promedios_CH1[posicion_datos_humedad_CH1 -1]     = datos_CH1[2];
        promedios_CH1[posicion_datos_temperatura_CH1 -1] = datos_CH1[3];
      }
    }
 
    ESTADO=B_T;// Una vez guardados los datos se pasa al estado B_T
  break;
/*----------------------------------------------------------------------------*/
/*----------------------------------Estado: CH2-------------------------------*/
  case CH2://Una vez se sepa que el paquete de datos llegó del canal 2, se guardan los datos y se verifica 
           //si los datos corresponden a promedios o datos en "tiempo real"
           
    radio.read(datos_CH2,sizeof(datos_CH2));//Función que lee y guarda los datos en el vector pasado como parámetro
    if (datos_CH2[4] == 1){//Si el dato en la posición 4 del vector recibido es 1, los datos son de promedios
      if(posicion_datos_agua_CH2 <89){//Si han llegado menos de 89 datos de promedio, se guardan los datos en la siguiente posición
        promedios_CH2[posicion_datos_agua_CH2]        = datos_CH2[0];
        promedios_CH2[posicion_datos_comida_CH2]      = datos_CH2[1];
        promedios_CH2[posicion_datos_humedad_CH2]     = datos_CH2[2];
        promedios_CH2[posicion_datos_temperatura_CH2] = datos_CH2[3];

        promedios_actuales_CH2[0]=datos_CH2[0];
        promedios_actuales_CH2[1]=datos_CH2[1];
        promedios_actuales_CH2[2]=datos_CH2[2];
        promedios_actuales_CH2[3]=datos_CH2[3];
        
        posicion_datos_agua_CH2 ++;
        posicion_datos_comida_CH2 ++;
        posicion_datos_humedad_CH2 ++;
        posicion_datos_temperatura_CH2 ++;
      }else{//Si es espacio reservado para cada variable sensada está lleno se hace un corrimiento de datos,
            //eliminando la muesta más antigua y guardando la más reciente 
        for (int i=0; i<89; i++){
          promedios_CH2 [i]       = promedios_CH2 [i+1];
          promedios_CH2 [i + 90]  = promedios_CH2 [i + 1 + 90];
          promedios_CH2 [i + 180] = promedios_CH2 [i + 1 + 180];
          promedios_CH2 [i + 270] = promedios_CH2 [i + 1 + 270];
        }
        promedios_CH2[posicion_datos_agua_CH2 -1]        = datos_CH2[0];
        promedios_CH2[posicion_datos_comida_CH2 -1]      = datos_CH2[1];
        promedios_CH2[posicion_datos_humedad_CH2 -1]     = datos_CH2[2];
        promedios_CH2[posicion_datos_temperatura_CH2 -1] = datos_CH2[3];
      }
      
    } 
    ESTADO=B_T;// Una vez guardados los datos se pasa al estado B_T
  break;
/*----------------------------------------------------------------------------*/
/*----------------------------------Estado: B_T-------------------------------*/
  case B_T:{//Por medio de la función BT.print(), se envían uno a uno los datos 
    //Para que la aplicación móvil pueda identificar y separar los datos, se envían un "|" separando cada dato
    //La información se organiza y se envía obedeciendo al siguiente orden 
    
    //Los primeros 8 datos corresponden a los datos entiempo real
    BT.print(datos_CH1[0]);
    BT.print("|");
    BT.print(datos_CH1[1]);
    BT.print("|");
    BT.print(datos_CH1[2]);
    BT.print("|");
    BT.print(datos_CH1[3]);
    BT.print("|");
    BT.print(datos_CH2[0]);
    BT.print("|");
    BT.print(datos_CH2[1]);
    BT.print("|");
    BT.print(datos_CH2[2]);
    BT.print("|");
    BT.print(datos_CH2[3]);
    
    //Los siguientes 8 datos corresponden a los promedios actuales
    BT.print("|");
    BT.print(promedios_actuales_CH1[0]);
    BT.print("|");
    BT.print(promedios_actuales_CH1[1]);
    BT.print("|");
    BT.print(promedios_actuales_CH1[2]);
    BT.print("|");
    BT.print(promedios_actuales_CH1[3]);
    BT.print("|");
    BT.print(promedios_actuales_CH2[0]);
    BT.print("|");
    BT.print(promedios_actuales_CH2[1]);
    BT.print("|");
    BT.print(promedios_actuales_CH2[2]);
    BT.print("|");
    BT.print(promedios_actuales_CH2[3]);

    //Los siguientes 360 datos corresponden a los promedios de las variables sensadas por el Nodo Sensor 1
    for (int i=0; i<360;i++){
      BT.print("|");
      BT.print(promedios_CH1[i]);
    }
    //Los últimos 360 datos corresponden a los promedios de las variables sensadas por el Nodo Sensor 2
    for (int i=0; i<360;i++){
      BT.print("|");
      BT.print(promedios_CH2[i]);
    }
    BT.print("|");

    ESTADO=TIMER;//Una vez completado el envío se pasa al estado TIMER
   }
  break;
/*----------------------------------------------------------------------------*/
/*--------------------------------Estado: TIMER-------------------------------*/
  case TIMER://El sistema espera en este estado hasta que se cumpla el tiempo definido en la variable "timer"
    if(timer>=Segundos){//Cuando se cumpla, pasa al estado RADIO_FIFO
      ESTADO=RADIO_FIFO;
      timer=0;
    }else{
      ESTADO=TIMER;
    }
  break;
  }
/*----------------------------------------------------------------------------*/
/***************************Fin maquina de estados*****************************/
}
/******************************************************************************/


/**************************Interrupciones TIMER0*******************************/
ISR(TIMER1_COMPB_vect){
  TCNT1 = 0;            // Reinicia el contador del Timer 1
  timer ++;             //Incrementa la variable de comparacion 1 segundo
}
/******************************************************************************/
