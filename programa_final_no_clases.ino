#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <cmath>

// Definición de pines y variables
//-----------------------------------------------------------------------------------------------------
#define channelhumValue 1     // Canal para medir humedad
#define power_pin 5           // Pin para alimentar el sensor de salinidad
#define channeltemValue 0        // Canal para medir la temperatura
#define channelLuzValue 3     //Canal para medir luz
int valor = 0;

//Definir el canal del ADS1115 por el que leeremos la tensión del sensor de PH
#define channelphValue 2
#define Offset 1
#define samplingInterval 20
#define printInterval 800
#define ArrayLenght 40 //Numero de muestras

// Variables globales comunes -------------------------------------------------------------------------
Adafruit_ADS1115 ads1115;  // Instancia del ADS1115

// Variables para humedad
int medidaSeco = 20200, medidaMojado = 9800;
int sensorValue = 0, humidityValue = 0;

//Valores obtenidos al calcular la expresión analítica
double a = -0.0096;
double b = 194.23;

// Variables para temperatura
float m = 35*pow(10,-3);         // Pendiente
float bTemp = 790*pow(10,-3);          // Valor independiente
float Vmax = 4.096;        // Voltaje máximo del ADC

//Almacenar muestras para el PH
int pHArray[ArrayLenght];
int pHArrayIndex=0;

//Luz
double umbralluminosidad[] = {500, 1500, 3000, 4000};

// Variables para salinidad
//int c0 = 200, c1 = 400, c2 = 600, c3 = 800, c4 = 1000;
// Puntos de calibración (valores digitales y sus correspondientes gramos de sal)
// Valores calibrados de referencia
//float x[5] = {200,400,600,800,1000}; // Valores digitales calibrados del ADS1115
//float fx[5] = {5, 10, 15, 20, 25};      // Valores correspondientes en gramos

//-----------------------------------------------------------------------------------------------------------
void setup() {
  // put your setup code here, to run once:
  //Inicializamos el monitor serie
  Serial.begin(9600);
  ads1115.setGain(GAIN_ONE);
  ads1115.begin(0x48);         //Inicializamos el ADS1115 con la dirección 0X48  
  pinMode(power_pin, OUTPUT);  // Configurar pin de alimentación como salida
  Serial.println("Inicializando el medidor de pH");
 
  calibrarHumedad();   // Calibración inicial del sensor de humedad
}

//--------------------------------Función para calibrar el sensor de humedad--------------------------------//
void calibrarHumedad() {
  Serial.println("Iniciando calibración de humedad...");

  // Calibrar valor en seco
  Serial.println("Coloque el sensor en seco y escriba 'OK'.");
  esperarComando("OK");
  medidaSeco = ads1115.readADC_SingleEnded(channelhumValue);
  Serial.println("Calibración en seco completada.");

  // Calibrar valor en mojado
  Serial.println("Coloque el sensor en mojado y escriba 'OK'.");
  esperarComando("OK");
  medidaMojado = ads1115.readADC_SingleEnded(channelhumValue);
  Serial.println("Calibración en mojado completada.");

  //Calibración completada
  Serial.println("Calibración completada.");
  Serial.print("Valores ajustados: Seco = ");
  Serial.print(medidaSeco);
  Serial.print(", Mojado = ");
  Serial.println(medidaMojado);
}

//-----------------------------------------------HUMEDAD--------------------------------------------------//
void medirHumedad() {
  sensorValue = ads1115.readADC_SingleEnded(channelhumValue);
  humidityValue = map(sensorValue, medidaSeco, medidaMojado, 0, 100);

  //Imprimimos el valor leído de la humedad
  Serial.println();
  Serial.print("Humedad: ");
  Serial.print(humidityValue,DEC); //DEC para que muestra la salida en decimal
  delay(1000);

  //Imprimimos el valor digital
  Serial.print(" Valor digital = ");
  Serial.print(sensorValue, DEC); //DEC para que muestra la salida en decimal
  delay(1000);

  //Expresión analítica para la representación gráfica de la función utilizada para calibrar el sensor de humedad
  double x = humidityValue;
  double y= a*x+b;

  //Imprimimos los valores para x e y a partir de la expresión analítica
  Serial.print(" Para x = ");
  Serial.print(x);
  Serial.print(", y = ");
  Serial.print(y);
  Serial.print('\n');
  delay(1000);
}

//--------------------------------------------SALINIDAD---------------------------------------------------//
void medirSalinidad() {
  int16_t adc0;
  //float interpolatedValue = lagrangeInterpolation(adcValue, x, fx, 5);
  digitalWrite(power_pin, HIGH);
  delay(100);
  adc0 = analogRead(0);
  digitalWrite(power_pin, LOW);
  delay(100);

  double c0=920.00;
  double c1=930.00;
  double c2=960.00;
  double c3=940.00;
  double c4=945.00;

  double d0= (c0-c1)*(c0-c2)*(c0-c3)*(c0-c4);
  double d1= (c1-c0)*(c1-c2)*(c1-c3)*(c1-c4);
  double d2= (c2-c0)*(c2-c1)*(c2-c3)*(c2-c4);
  double d3= (c3-c0)*(c3-c1)*(c3-c2)*(c3-c4);
  double d4= (c4-c0)*(c4-c1)*(c4-c2)*(c4-c3);

  double fx0l0 = 5*((adc0-c1)*(adc0-c2)*(adc0-c3)*(adc0-c4))/d0;
  double fx1l1 = 10*((adc0-c0)*(adc0-c2)*(adc0-c3)*(adc0-c4))/d1;
  double fx2l2 = 15*((adc0-c0)*(adc0-c1)*(adc0-c3)*(adc0-c4))/d2;
  double fx3l3 = 20*((adc0-c0)*(adc0-c1)*(adc0-c2)*(adc0-c4))/d3;
  double fx4l4 = 25*((adc0-c0)*(adc0-c1)*(adc0-c2)*(adc0-c3))/d4;

  double ng = fx0l0 + fx1l1 + fx2l2 + fx3l3 + fx4l4;

  if (adc0 < 910){
    ng = 0;
  }
  if (adc0 >= 950){
    ng = 30;
  }

  Serial.print("Lectura digital = ");
  Serial.println(adc0, DEC);
  Serial.print("Lectura en gramos normal = ");
  Serial.println(ng);
  Serial.println("");
}

//------------------------------------------------Array de temperaturas-----------------------------------//
double averageSample(double* listamuestras, int numeromuestras)
{
  double sumamuestras= 0.0; //Acumula la suma de las muestras
  for (int i=0; i<numeromuestras; i++)
  {
    listamuestras[i] = ads1115.readADC_SingleEnded(channeltemValue);
    sumamuestras+= listamuestras[i]; //Sumar cada muestras
  }
  return sumamuestras/numeromuestras; //Media de las sumas;
}

//---------------------------------------TEMPERATURA-----------------------------------------------------//
void medirTemperatura() {
  int16_t adc0Temp = ads1115.readADC_SingleEnded(channeltemValue); //Captura una muestra del ads1115
 
  const int numeromuestras= 30;
  double listamuestras[numeromuestras];
  double media = averageSample(&listamuestras[0], numeromuestras);

  //Imprimimos el valor leído de la media
  Serial.print("La media es: ");
  Serial.println(media);

  float Vo= (media*Vmax)/(32767); // Convertir el valor ADC a voltaje, 32767 es el valor máximo para un ADC de 16 bits
  float temperatura = (Vo-bTemp)/m; //Aplicamos la fórmula de la temperatura en función de la lectura digital

  //Imprimimos el valor leído de la lectura
  Serial.print("Lectura del ADS1115: ");
  Serial.println(adc0Temp);

  //Imprimimos el valor del voltaje
  Serial.print("Voltaje: ");
  Serial.println(Vo,DEC);

  //Imprimimos el valor de la temperatura
  Serial.print("Temperatura: ");
  Serial.print(temperatura, DEC);
  Serial.println(" ºC");
  Serial.println();
  delay(1000);
}

//----------------------------------------------Array de PH----------------------------------------------//
double averagesample(double* listamuestras, int numeromuestras)
{
  double sumamuestras= 0.0; //Acumula la suma de las muestras
  for (int i=0; i<numeromuestras; i++)
  {
    listamuestras[i] = ads1115.readADC_SingleEnded(channelphValue);
    sumamuestras+= listamuestras[i]; //Sumar cada muestras
  }
  return sumamuestras/numeromuestras; //Media de las sumas;
}

//------------------------------------------PH-----------------------------------------------------------//

void medirpH(void) {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval) // Cada samplingTime segundos se toma una muestra
  {
    //realizar una lectura del ADS1115
    const int numeromuestras = 30;
    double listamuestras[numeromuestras];
    double media = averageSample(&listamuestras[0], numeromuestras); //Media de las 30 muestras tomadas por el ads

    pHArray[pHArrayIndex++] = ads1115.readADC_SingleEnded(channelphValue);
    if(pHArrayIndex == ArrayLenght)pHArrayIndex = 0;
   
    // convertir la lectura a tensión
    voltage = (ads1115.readADC_SingleEnded(channelphValue) * 0.125) / 1000.0;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }

  if (millis() - printTime > printInterval) //Cada printTime segundos se escribe un dato en pantalla
  {
    Serial.print("Voltage: ");
    Serial.println(voltage, 2);
    Serial.print("   pH value: ");
    Serial.println(pHValue, 2);
      printTime = millis();
  }
}

// Función para esperar un comando
void esperarComando(String comandoEsperado) {
  while (true) {
    if (Serial.available() > 0) {
      String comando = Serial.readStringUntil('\n');
      comando.trim();
      if (comando.equalsIgnoreCase(comandoEsperado)) {
        break;
      }
    }
  }
}

//--------------------------------------------------LUMINOSIDAD------------------------------------------//
void medirLuz() {
  int16_t adc0 = ads1115.readADC_SingleEnded(channelLuzValue);
  float Vo = (adc0 * Vmax) / 32767;

  Serial.print("Lectura del sensor de luz: ");
  Serial.println(adc0);

  if (adc0 < umbralluminosidad[0]) {
    Serial.println("Nivel de luminosidad: Oscuridad");
  } else if (adc0 < umbralluminosidad[1]) {
    Serial.println("Nivel de luminosidad: Sombra");
  } else if (adc0 < umbralluminosidad[2]) {
    Serial.println("Nivel de luminosidad: Luz ambiente");
  } else if (adc0 < umbralluminosidad[3]) {
    Serial.println("Nivel alto de iluminación");
  } else {
    Serial.println("Nivel muy alto de iluminación");
  }

  Serial.print("Voltaje: ");
  Serial.println(Vo);
}


//--------------------------------------------------------------------------------------------------------//

void loop() {
  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n');
    valor = entrada.toInt();
    Serial.print("El número es: ");
    Serial.println(valor);
  }

  Serial.println("\nMenú de sensores:");
  Serial.println("1. HUMEDAD");
  Serial.println("2. SALINIDAD");
  Serial.println("3. TEMPERATURA");
  Serial.println("4. PH");
  Serial.println("5. LUMINOSIDAD");
  Serial.println("Seleccionada: ");
 
  switch (valor) {
    case 1:
      medirHumedad();
      break;
    case 2:
      medirSalinidad();
      break;
    case 3:
      medirTemperatura();
      break;
    case 4:
      medirpH();
      break;
    case 5:
      medirLuz();
      break;
    default:
      Serial.println("Opción no válida.");
      break;
  }
  delay(2000); // Delay para evitar lecturas demasiado rápidas
}



