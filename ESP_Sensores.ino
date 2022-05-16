//********************************Variables para Configuración AccessPoint*********************
#include <WiFi.h>
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";
WiFiServer server(80);
String header;

//Entradas
#define PE1 digitalRead(pe1)
#define PE2 digitalRead(pe2)

//Salidas
#define   TS0   19          //Pines para enviar al Arduino Uno
#define   TS1   18          //Pines para enviar al Arduino Uno
#define   TS2   5           //Pines para enviar al Arduino Uno

//Detección de color
#define S0 36    // S0 a pin 4
#define S1 39    // S1 a pin 5
#define S2 34    // S2 a pin 6
#define S3 35    // S3 a pin 7
#define salidaTCS 32 // salidaTCS a pin 8
//#define Boton 3

// DECLARACION DE VARIABLES PARA PINES
const int pe1 = 23;
const int pe2 = 21;
const int pintrigger = 22;
const int pinled = 13;
// VARIABLES PARA CALCULOS
unsigned int distancia1, distancia2;

// varaibles globales
short obs;                  //Obstaculos
short objP;                 //Posición de la pelota
boolean limC = false;       //Detección de límite de cancha
boolean Nav = false;        //Bandera de Navegacion
boolean evObs = false;      //Bandera de Evación de obstáculos
short i;                    //Varaible Auxiliar

//Variables detección de color
int rva[3] = {0,0,0};
int rvaExterior[3] = {0,0,0};
int rvaInterior[3] = {0,0,0};
int rvaLineas[3] = {0,0,0};
int rvaRed[3] = {0,0,0};

void setup() {
  // put your setup code here, to run once:
  // PREPARAR LA COMUNICACION SERIAL
  Serial.begin(9600);
  // Entradas
  pinMode(pe1, INPUT);          //Pines Ultrasónico
  pinMode(pe2, INPUT);      
  
  //Salidas
  pinMode(TS0,OUTPUT);          //declarar como salida los pines de comunicación con el Arduino
  pinMode(TS1,OUTPUT);          //declarar como salida los pines de comunicación con el Arduino
  pinMode(TS2,OUTPUT);          //declarar como salida los pines de comunicación con el Arduino
  pinMode(pintrigger, OUTPUT);  //pines Ultrasónico
  pinMode(pinled, OUTPUT);

  //Color
//  pinMode(Boton, INPUT); // pin 3 como entrada
//  pinMode(Pelota, INPUT);
//  pinMode(Tope, INPUT);
//  pinMode(in1, OUTPUT);
//  pinMode(in2, OUTPUT);
  pinMode(S0, OUTPUT);    // pin 4 como salida
  pinMode(S1, OUTPUT);    // pin 5 como salida
  pinMode(S2, OUTPUT);    // pin 6 como salida
  pinMode(S3, OUTPUT);    // pin 7 como salida
  pinMode(salidaTCS, INPUT);  // pin 8 como salida
  digitalWrite(S0,HIGH);  // establece frecuencia de salida
  digitalWrite(S1,LOW);   // del modulo al 20 por ciento
  
  accessPointInit();            //Inicializa el punto de acceso del ESP32

  //reconocerCancha();
  Serial.println("Termino del Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //Modo Búsqueda
  Busqueda:
    Nav = false;                //bandera navegacion apagada
    goto lecSens;               //leer sensores
    if(objP == 3){              //si no se detectan pelotas
      digitalWrite(TS0,LOW);      //girar espiral 1 0 0
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,HIGH);
      goto lecSens;                //leer sensores
      }
    else{                         //si se detectan pelotas
      digitalWrite(TS0,LOW);      //detener 0 0 0
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,LOW);
      Nav = true;                 //bandera de navegacion activa
      goto Navegacion;            //Modo Navegacion
    }
    

  //Módo navegación
  Navegacion:
    if(objP == 1){                  //Pelota a la derecha
      digitalWrite(TS0,HIGH);       //girar derecha 0 0 1
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,LOW);
      goto lecSens;                 //leer sensores     
    }
    if(objP == 2){                //Pelota a la izquierda
      digitalWrite(TS0,LOW);      //girar izquierda 0 1 0
      digitalWrite(TS1,HIGH);
      digitalWrite(TS2,LOW);
      goto lecSens;               //leer sensores 
    }
    if(objP == 0){                //Pelota centrada
      goto lecSens;               //leer sensores
      digitalWrite(TS0,LOW);      //adelante 0 1 1
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,HIGH);
    }

  //Modo Obstáculos
  Obstaculos:
    evObs = true;                 //bandera de evacion de obstaculos activa
    digitalWrite(TS0,LOW);          //Detener motores 0 0 0
    digitalWrite(TS1,LOW);
    digitalWrite(TS2,LOW);
    delay(250);                   //Esperar pequeño tiempo, evita cambio de polaridad brusca en motores
    if(obs == 1){                 //Obstáculo a la derecha
      digitalWrite(TS0,LOW);      //girar izquierda 0 1 0
      digitalWrite(TS1,HIGH);
      digitalWrite(TS2,LOW);
      goto lecSens;               //leer sensores
      return;
    }
    if(obs == 2){                 //Obstáculo a la izquierda
      digitalWrite(TS0,HIGH);       //girar derecha 0 0 1
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,LOW);
      goto lecSens;               //leer sensores
      return;
    }
    if(obs == 3){                 //Obstáculo al frente
      digitalWrite(TS0,HIGH);     //girar derecha 0 0 1
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,LOW);
      goto lecSens;               //leer sensores
      return;
    }
    /*if(obs == 4){                 //Obstáculo atrás
      digitalWrite(TS0,HIGH);       //girar derecha 0 0 1
      digitalWrite(TS1,LOW);
      digitalWrite(TS2,LOW);
      goto lecSens;                 //leer sensores
      return;
    }*/
    if(limC){                     //Detección del límite de cancha
      goto lecSens;               //leer sensores
      digitalWrite(TS0,LOW);      //girar izquierda 0 1 0
      digitalWrite(TS1,HIGH);
      digitalWrite(TS2,LOW);
      delay(1000);                //control de giro Izquierdo
      goto lecSens;               //leer sensores
      return;
    } 

  lecSens:
    //Sección para asignar valores de los sensores y cuantizarlos
    visionPelotas();                                                //Busca centro de pelota, 0-centro,1-derecha,2-izquierda, 3-sin objetivo
    lecObs();
    if( Nav && (limC || obs ==3)){                                  //Si se encuentra un obstáculo enfrente, o se determina el límite de cancha mientras navega
      goto Obstaculos;
    }
    if( evObs && (limC || obs != 0)){                               //Si se encuentra evadiendo Obtáculos
      goto Obstaculos;
    }
    if(!limC && obs == 0){
      evObs == false;                                               //Bandera de evación de obstáculos desactivada
    }
    if(objP == 3){                                                  //Si no se detectan pelotas
      goto Busqueda;
    }
    return;
}

void accessPointInit(){
  WiFi.softAP(ssid, password);
  Serial.print("AccessPoint IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

int visionPelotas()
{
int ball;
WiFiClient client = server.available();     // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
     
            if (header.indexOf("right") >= 0){ 
              Serial.println("Pelota a la derecha");
              ball=1;
            }
            if (header.indexOf("left") >= 0){ 
              Serial.println("Pelota a la izquierda");
              ball=2;
            }
            if (header.indexOf("null") >= 0) {
              Serial.println("NO se detecta pelota");
              ball=3;
            }
            if (header.indexOf("center") >= 0){
              Serial.println("Pelota en el centro");
              ball=0;
            }
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  return ball;
} 

void lecObs() {
  // ENVIAR PULSO DE DISPARO EN EL PIN "TRIGGER"
  digitalWrite(pintrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pintrigger, HIGH);
  // EL PULSO DURA AL MENOS 10 uS EN ESTADO ALTO
  delayMicroseconds(10);
  digitalWrite(pintrigger, LOW);

unsigned int y1 = 0;
unsigned int y2 = 0;
unsigned int y3 = 0;
unsigned int y4 = 0;


  b00:
  if(PE1 == LOW && PE2 == LOW){
    delayMicroseconds(1);
    goto b00;
  }

  bi:
    if(PE1 == HIGH){
      y1++;
    }
    if(PE2 == HIGH){
      y2++;
    }
    //Descomentar para usar cuatro sensores
    /*if(PE3 == HIGH){
      y3++;
    }
    if(PE4 == HIGH){
      y4++;
    }*/

    if(PE1 == LOW && PE2 == LOW){
    //Descomentar para usar cuatro sensores
    //if(PE1 == LOW && PE2 == LOW && PE3 == LOW && PE4 == LOW){ 
      goto bf;
    }
  
  delayMicroseconds(1);
  goto bi;
  bf:

  //Por calibraciÃ³n, 20 fue el mejor valor, al menos de manera preliminar... 
  distancia1 = y1 / 20;
  distancia2 = y2 / 20; 
 
  //Serial
  Serial.print(distancia1);
  Serial.print(" --cm[1] ");
  Serial.print(distancia2);
  Serial.println(" --cm[2] ");
  delay(200);

  distinguirPosicion();
}

void distinguirPosicion()
{
   LecturaSensorColor(rva);
   if(compararRVA(rva,rvaExterior)==1)
   {
    Serial.print("Estas en Exterior de la cancha \n");
   }
   else
   {
    if(compararRVA(rva,rvaInterior)==1)
    {
     Serial.print("Estas en interior de la cancha \n");
    }
    else
    {
      if(compararRVA(rva,rvaLineas)==1)
      {
       Serial.print("Estas sobre una linea de la cancha \n");
      }
      else
      {
        if(compararRVA(rva,rvaRed)==1)
          {
           Serial.print("Estas en la red de la cancha \n");
          }
        else
        {
          Serial.print("Color no reconocido muevete un poco \n");
        }
      }
    }
   }
}

int compararRVA(int RVA1[],int RVA2[])
{
  if(RVA1[0]<=(RVA2[0]+15) && RVA1[0]>=(RVA2[0]-15))
   {
    if(RVA1[1]<=(RVA2[1]+15) && RVA1[1]>=(RVA2[1]-15))
    {
      if(RVA1[2]<=(RVA2[2]+15) && RVA1[2]>=(RVA2[2]-15))
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      return 0;
    }
   }
   else
   {
    return 0;
   }
}

void reconocerCancha()
{
  Serial.print("Hola, para reconocer la cancha nececitamos muestras de 4 colores");
  Serial.print("\n"); 
  Serial.print("El primer color es el del exterior de la cancha");
  Serial.print("\n"); // Salto de linea
  Serial.print("Por favor coloca el sensor en el exterior de la cancha y presiona el boton para empezar");
  Serial.print("\n");
  tomarMuestras(rvaExterior);
  Serial.print("Hemos tomado las muestras del exterior de la cancha");
  Serial.print("\n");
  Serial.print("Por favor coloca el sensor en el interior de la cancha y presiona el boton para empezar");
  Serial.print("\n");
  tomarMuestras(rvaInterior);
  Serial.print("Hemos tomado las muestras del interior de la cancha");
  Serial.print("\n");
  Serial.print("Por favor coloca el sensor sobre una linea que conforma la cancha y presiona el boton para empezar");
  Serial.print("\n");
  tomarMuestras(rvaLineas);
  Serial.print("Hemos tomado las muestras de las lineas de la cancha");
  Serial.print("\n");
  Serial.print("Por favor coloca el sensor en la marca debajo de la red de la cancha y presiona el boton para empezar");
  Serial.print("\n");
  tomarMuestras(rvaRed);
  Serial.print("Hemos tomado las muestras de la red de la cancha");
  Serial.print("\n");
}
void tomarMuestras(int RVA[])
{
  int PromedioR = 0;
  int PromedioV = 0;
  int PromedioA = 0;
  //while(leerEstadoBoton()== 1); //Esperamos que presione el boton sin hacer nada
  for(int i=0;i<10;i++)
  {
    LecturaSensorColor(rva);
    PromedioR += rva[0];
    PromedioV += rva[1];
    PromedioA += rva[2];
    imprimirValoresSensorColor(rva);
  }
  RVA[0] = PromedioR/10;
  RVA[1] = PromedioV/10;
  RVA[2] = PromedioA/10;
  Serial.print(RVA[0]);
  Serial.print("\t");
  Serial.print(RVA[1]);
  Serial.print("\t");
  Serial.print(RVA[2]);
  Serial.print("\n");
}
void LecturaSensorColor(int RVA[])
{
  digitalWrite(S2,LOW);     // establece fotodiodos
  digitalWrite(S3,LOW);     // con filtro rojo
  RVA[0] = pulseIn(salidaTCS, LOW); // obtiene duracion de pulso de salida del sensor
  delay(200);       // demora de 200 mseg
  
  digitalWrite(S2,HIGH);    // establece fotodiodos
  digitalWrite(S3,HIGH);    // con filtro verde
  RVA[1] = pulseIn(salidaTCS, LOW);  // obtiene duracion de pulso de salida del sensor
  delay(200);       // demora de 200 mseg
  
  digitalWrite(S2,LOW);     // establece fotodiodos
  digitalWrite(S3,HIGH);    // con filtro azul
  RVA[2] = pulseIn(salidaTCS, LOW); // obtiene duracion de pulso de salida del sensor
  delay(200);       // demora de 200 mseg       
}
void imprimirValoresSensorColor(int RVA[])
{  
  //Imprime rojo
  Serial.print("R:");     // muestra texto
  Serial.print(RVA[0]);     // muestra valor de variable rojo
  Serial.print("\t"); // salto de linea
  //Imprime verde
  Serial.print("V:");     // muestra texto
  Serial.print(RVA[1]);      // muestra valor de variable verde
  Serial.print("\t"); // espacio de tabulacion
  Serial.print("\t");     // espacio de tabulacion
  //imprime azul
  Serial.print("A:");     // muestra texto
  Serial.println(RVA[2]);     // muestra valor de variable azul         
}
