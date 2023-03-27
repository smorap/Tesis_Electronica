# **Tesis_Electrónica**
En este repositorio estarán todos los archivos y documentos referentes a mi tesis de pregrado titulado "Monitoreo de ambiente en granja de grillos Gryllodes Sigillatus".

## Abstrac
ccording to the CDIO methodology, this document records the process of conception, design, implementation and testing of an environment monitoring system for a cricket farm. This monitoring system has two Sensor Nodes, a Central Node and a smartphone application to view the information. The nodes communicate with each other through the NRF24L01 module and the Central Node uses a Bluetooth HC-06 module to communicate with the smartphone app. Each node incorporates an ATMEGA238p that receives information from the communication modules and from two sensors in the case of the Sensor Node, the DHT22 and two load cells. The environmental variables monitored by each Sensor Node are humidity and temperature with the DHT22, and weight with a load cell. The data is processed by the microcontroller and sent to a mobile phone to be displayed in an application programmed with the MIT APP inventor tool. In the application the environmental variations are visualized in real time and in graphs an average record of the last 22 hours of the received data is shown. The application has the ability to alert if the recorded values are out of ranges that can be configured within the application. Throughout the document each of the above aspects is shown in detail and at the end the system is evaluated for each of the parts that comprise it

## Descripción 
El trabajo desarrollado consiste en el diseño, implementación y prueba de un sistema de monitoreo ambiental para una granja de grillos, siendo este insecto utilizado como complemento proteínico para consumo humano. La empresa Arthrofood planea fabricar una harina a base de este insecto. Para su cría es muy importante conocer la cantidad de alimento, la temperatura y humedad en el ambiente. Con este sistema es posible hacer los cambios necesarios en el ambiente para garantizar el crecimiento eficiente del insecto, disminuyendo costos de producción y tiempo en comparación a otras fuentes de proteína. El sistema de monitoreo implementado tiene dos Nodos Sensores, un Nodo Central y una aplicación de teléfono inteligente programada con la herramienta MIT APP inventor para ver la información. Cada Nodo incorpora un ATMEGA238p que recibe información de los módulos de comunicación. Las variables ambientales monitoreadas por cada Nodo Sensor son la humedad y la temperatura con el DHT22, y la cantidad de comida y agua es medida con dos celdas de carga. En la aplicación se pueden observar las variaciones en tiempo real y mediante gráficas se visualiza un registro promedio de las últimas 22 horas para cada una de las variables de cada Nodo Sensor. Dentro de la aplicación se pueden configurar los rangos aceptables de las variables medidas con el fin de alertar mediante una notificación en la aplicación cuando estos están por fuera de dichos rangos. Aunque el sistema no fue puesto a prueba en una granja real, en laboratorio se obtuvieron resultados satisfactorios de funcionamiento. 


## Contenidos
 1. [Documento](/Tesis_Grillos__Final.pdf) 
 2. [Código_final_comentado](/Codigo_final_comentado)
 3. [Ejemplos_Modulos](/Ejemplos_Modulos)
 4. [App monitoreo](/APP)
 
### [Documento](/Tesis_Grillos__Final.pdf)
 Documento oficial de la Tesis. En el se encuentra al detalle el procesos de concepción, desarrollo e implementacion del sistema de monitoreo.

### [Código final](/Codigo_final_comentado)
 El sistema está conformado por un Nodo Central y dos Nodos sensores. Acá se encontrarán los archivos (.ino) respectivos de los nodos.

### [Ejemplos Modulos](/Ejemplos_Modulos)
 Codigos ejemplos de como usar los sensores y modulos del sistema. 

### [App Monitoreo](/APP) 
 Esta es la app para Android (.apk) donde se pueden visualizar los datos. (Puede descargarse pero no mostrará ningun dato si no se conecta al Nodo Central.      Usuario/Contraseña: admin)
