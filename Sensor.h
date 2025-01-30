#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <cmath>

Adafruit_ADS1115 ads1115;  // Instancia del ADS1115

double adc0;
double adc0Temp;
double adc0Luz;

class Sensor {
 private:

  
	 //Variable humedad
	 const int medidaSeco = 20200;
	 const int medidaMojado = 9800;

	 //Pin para alimentar el sensor de salinidad
	 int power_pin = 5;
	
  //Valores obtenidos al calcular la expresión analítica
   double a = -0.0096;
   double b = 194.23;

	 // Variables para temperatura
	 float m = 35 * pow(10, -3);         // Pendiente
	 float bTemp = 790 * pow(10, -3);          // Valor independiente
	 float Vmax = 4.096;        // Voltaje m�ximo del ADC

	 int samplingInterval = 20;
	 int printInterval = 800;
	 int ArrayLenght = 40; //Numero de muestras del array

	 //Offset para calibrar el sensor de pH
	 int Offset = 1;
	 //Almacenar muestras para el pH
	 int pHArray[40];
	 int pHArrayIndex = 0;

   //Variables para luminosidad
   double umbralluminosidad[4]= { 500, 1500, 3000, 4000}; //Valores de referencia
   int porcentaje_lum; 

 public:

	 //sensores
	 void Inicializar();
	 double medirHumedad();
	 double medirSalinidad();
	 double medirTemperatura();
	 double medirpH();
	 double medirLuminosidad();
	 
}; // class

// ---------------------------------------------------

void Sensor::Inicializar() {
 
	//Inicializamos el monitor serie
	Serial.begin(9600);

	ads1115.begin(0x48);         //Inicializamos el ADS1115 con la direcci�n 0X48  
	ads1115.setGain(GAIN_ONE);
	pinMode(power_pin, OUTPUT);  // Configurar pin de alimentaci�n como salida
}


double Sensor::medirHumedad() {
  int sensorValue = ads1115.readADC_SingleEnded(1);

  int humidityValue = map(sensorValue, medidaSeco, medidaMojado, 0, 100);
if (humidityValue > 100) {
	humidityValue = 100;
}
if (humidityValue < 0) {
	humidityValue = 0;
}
return humidityValue;

//Imprimimos el valor le�do de la humedad
Serial.println();
Serial.print("Humedad: ");
Serial.print(humidityValue, DEC); //DEC para que muestra la salida en decimal
delay(1000);


//Imprimimos el valor digital
Serial.print(" Valor digital = ");
Serial.print(sensorValue, DEC); //DEC para que muestra la salida en decimal
delay(1000);


//Expresi�n anal�tica para la representaci�n gr�fica de la funci�n utilizada para calibrar el sensor de humedad
double x = humidityValue;
double y = a * x + b;


//Imprimimos los valores para x e y a partir de la expresi�n anal�tica
Serial.print(" Para x = ");
Serial.print(x);
Serial.print(", y = ");
Serial.print(y);
Serial.print('\n');
delay(1000);
}

double Sensor::medirSalinidad() {

	digitalWrite(power_pin, HIGH);
	delay(100);
	adc0 = analogRead(0);
	digitalWrite(power_pin, LOW);
	delay(100);

	double c0 = 920.00;
	double c1 = 930.00;
	double c2 = 960.00;
	double c3 = 940.00;
	double c4 = 945.00;


	double d0 = (c0 - c1) * (c0 - c2) * (c0 - c3) * (c0 - c4);
	double d1 = (c1 - c0) * (c1 - c2) * (c1 - c3) * (c1 - c4);
	double d2 = (c2 - c0) * (c2 - c1) * (c2 - c3) * (c2 - c4);
	double d3 = (c3 - c0) * (c3 - c1) * (c3 - c2) * (c3 - c4);
	double d4 = (c4 - c0) * (c4 - c1) * (c4 - c2) * (c4 - c3);


	double fx0l0 = 5 * ((adc0 - c1) * (adc0 - c2) * (adc0 - c3) * (adc0 - c4)) / d0;
	double fx1l1 = 10 * ((adc0 - c0) * (adc0 - c2) * (adc0 - c3) * (adc0 - c4)) / d1;
	double fx2l2 = 15 * ((adc0 - c0) * (adc0 - c1) * (adc0 - c3) * (adc0 - c4)) / d2;
	double fx3l3 = 20 * ((adc0 - c0) * (adc0 - c1) * (adc0 - c2) * (adc0 - c4)) / d3;
	double fx4l4 = 25 * ((adc0 - c0) * (adc0 - c1) * (adc0 - c2) * (adc0 - c3)) / d4;


	double ng = fx0l0 + fx1l1 + fx2l2 + fx3l3 + fx4l4;


	if (adc0 < 910) {
		ng = 0;
	}
	if (adc0 >= 950) {
		ng = 30;
	}


	Serial.print("Lectura digital = ");
	Serial.println(adc0, DEC);
	Serial.print("Lectura en gramos normal = ");
	Serial.println(ng);
	Serial.println("");
  return ng;

}


double Sensor::medirTemperatura() {
	int adc0Temp = ads1115.readADC_SingleEnded(0); //Captura una muestra del ads1115

	float Vo = (adc0Temp * Vmax) / (32767); // Convertir el valor ADC a voltaje, 32767 es el valor máximo para un ADC de 16 bits
	float temperatura = (Vo - bTemp) / m; //Aplicamos la fórmula de la temperatura en función de la lectura digital

	
	//Imprimimos el valor del voltaje
	Serial.print("Voltaje: ");
	Serial.println(Vo, DEC);


	//Imprimimos el valor de la temperatura
	Serial.print("Temperatura: ");
	Serial.print(temperatura, DEC);
	Serial.println(" ºC");
	Serial.println();
	delay(1000);

  return temperatura;
}

double Sensor::medirpH() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval) { // Cada samplingTime segundos se toma una muestra

	  pHArray[pHArrayIndex++] = ads1115.readADC_SingleEnded(2);
	  if (pHArrayIndex == ArrayLenght)pHArrayIndex = 0;

	  // convertir la lectura a tensión
	  voltage = (ads1115.readADC_SingleEnded(2) * 0.125) / 1000.0;
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
  return pHValue;
}

double Sensor::medirLuminosidad() {
	int adc0Luz = ads1115.readADC_SingleEnded(3); //Captura una muestra del ads111
 
  float Vo= (adc0*Vmax)/(32767); // Convertir el valor ADC a voltaje, 32767 es el valor máximo para un ADC de 16 bits


  //Imprimimos el valor leído de la lectura
  Serial.print("Lectura del ADS1115: ");
  Serial.println(adc0);


  //Imprimimos el nivel de luminosidad
 if ( adc0 < umbralluminosidad[0]){
  Serial.println("Nivel de luminosidad: Oscuridad");
  porcentaje_lum = 0;
 }
 else if ( adc0 < umbralluminosidad[1]){
  Serial.println("Nivel de luminosidad: Sombra");
  porcentaje_lum = 25;
 }
 else if (adc0 < umbralluminosidad[2]){
  Serial.println("Nivel de luminosidad: Luz ambiente");
  porcentaje_lum = 50;
 }
 else if (adc0 < umbralluminosidad[3]){
  Serial.println("Nivel alto de iluminación.");
  porcentaje_lum = 75;
 }
  else {
  Serial.println("Nivel muy alto de iluminación.");
  porcentaje_lum = 100;
  }


  //Imprimimos el valor del voltaje
  Serial.print("Voltaje: ");
  Serial.println(Vo,DEC);


  delay(1000);

  return porcentaje_lum;
}

