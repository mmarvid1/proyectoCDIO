#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <cmath>
#include <ESP8266WiFi.h>
#include "Sensor.h"
// Comentar/Descomentar para conexion Fuera/Dentro de UPV
//       |
//       v
//#define WiFi_CONNECTION_UPV

//definir el wifi                                          

//---------------- DECLARACIÓN SENSORES ----------------------------------------------
Sensor sens;      //Sensores

//------------ WIFI ------------------------------------------------------------------
// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/1935509)
//#define REST_SERVER_DWEET //Selecciona tu canal para ver los datos en la web (http://dweet.io/follow/PruebaGTI)

///////////////////////////////////////////////////////
/////////////// WiFi Definitions /////////////////////
//////////////////////////////////////////////////////

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  //const char WiFiSSID[] = "Redmi Note 11S";
  //const char WiFiPSK[] = "majoelar12";

  const char WiFiSSID[] = "AndroidAP_6690";
  const char WiFiPSK[] = "123456789";
//const char WiFiSSID[] = "MySSID";
  //const char WiFiPSK[] = "MyPassWord";
#endif


///////////////////////////////////////////////////////
/////////////// SERVER Definitions /////////////////////
//////////////////////////////////////////////////////

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

///////////////////////////////////////////////////////
/////////////// HTTP REST Connection ////////////////
//////////////////////////////////////////////////////

const char Rest_Host[] = "dweet.io";
String MyWriteAPIKey= "cdiocurso2024g01"; // Escribe la clave de tu canal Dweet

// numero de sensores a mostrar
#define NUM_FIELDS_TO_SEND 5 

/////////////////////////////////////////////////////
/////////////// Pin Definitions ////////////////
//////////////////////////////////////////////////////

const int LED_PIN = 5; // Thing's onboard, green LED

/////////////////////////////////////////////////////
/////////////// WiFi Connection ////////////////
//////////////////////////////////////////////////////

void connectWiFi()
{
  byte ledStatus = LOW;

   
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
       Serial.println(".");

    delay(500);
  }

     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address

}


////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){

              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&sensor" + String( field ) + "=" + fieldData[ field ];
           }
           
              Serial.println( "Connecting to Server for update..." );

           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();

              Serial.println( PostData );
              Serial.println();
    }
}

void setup() {
   //Inicializamos el monitor serie
  Serial.begin(9600);
  ads1115.begin(0X48);       //Inicializamos el ADS1115 con la dirección 0X48  
  ads1115.setGain(GAIN_ONE);
  
 #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(115200);
  #endif
  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif

  
}

void loop(){

double humedad = sens.medirHumedad();
double salinidad = sens.medirSalinidad();
double temperatura = sens.medirTemperatura();
double pH = sens.medirpH();
double luminosidad = sens.medirLuminosidad();

String data[ 5 ];

    
    data[ 1 ] = String(1); //Escribimos el dato de pH
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.println( data[ 1 ] );
    #endif

    data[ 2 ] = String(2); //Escribimos el dato de humedad
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.println( data[ 2 ] );
    #endif

    data[ 3 ] = String(3);
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Tempreatura = " );
        Serial.println( data[ 3 ] );
    #endif

    data[ 4 ] = String(4);
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "PH = " );
        Serial.println( data[ 4 ] );
    #endif

     data[ 5 ] = String(5);
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Luminosidad = " );
        Serial.println( data[ 5 ] );
    #endif
    
  //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
  //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

  //delay(2000); // Delay para evitar lecturas demasiado rápidas
  delay(15000);

}

