/*
Proyecto: Afinador Automatico
Nombre: Facundo Maldonado
Profesores: -Alejo Giles(Microcontroladores),-Nicolas Gonzalez(Robótica y Automatización Industrial).

Como interactúa el usuario:
El usuario prende un interruptor (que energiza motor , pantalla led,arduino nano, y controlador l298n); paso seguido se prende afinador (que toma la nota de la guitarra)
paso seguido se pone el encastre(el encastre está acoplado al motor ) del afinador automático en la clavija (para hacer girar  la clavija, ajustando o desajustando segun  la referencia del
afinador).

*/


#include "Stepper.h" 
#include "LiquidCrystal_I2C.h" 


LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MS_INTERVALO_LED_TEST            250 //Function LedTest()
#define LED_TEST                         13
#define CONFIG_LED_TEST                  pinMode(LED_TEST, OUTPUT)
#define ACTUALIZAR_LED_TEST(x)           digitalWrite(LED_TEST, x)


int modo_manual = 0;

const int pasosPorRevolucion = 200;                 // 200 pasos por revolución - 1,8º (por paso) para una vuelta entera del motor paso a paso NEMA 17
Stepper stepper(pasosPorRevolucion, 8, 9, 10, 11); // Inicializar la librería para el motor paso a paso con los pines 8, 9, 10 y 11

#define VELOCIDAD_MOTOR                stepper.setSpeed(50); 
#define DIRECCION_MOTOR_IZQUIERDA      stepper.step(-1); 
#define DIRECCION_MOTOR_DERECHA        stepper.step(1);

// Defines de pantalla 

#define INICIALIZAR_DISPLAY  lcd.init() 
#define ENCENDER_DISPLAY   lcd.backlight() 
#define LIMPIAR_DISPLAY     lcd.clear()

//Defines  LED Afinado y Bemol (del afinador Externo donde se toma la lectura de pulso)

#define LEER_BEMOL        analogRead(A2)   //Conectado a  led bemol del afinador
#define LEER_SOSTENIDO     analogRead(A3)  // Conectado a led sostenido del afinador


//Estados del afinador
#define ESTADO_AFINADO    0
#define ESTADO_SOSTENIDO  1
#define ESTADO_BEMOL      2
int estado=ESTADO_AFINADO;
int estado_ant;

/*
  IMPORTANTE: Usar el I2C Scanner para encontrar la dirección correcta, que en este caso es 0x27

  Se definen también las 16 columnas y 2 filas del display LCD con driver I2C. SDA es el pin A4 y SCL es el pin A5
*/


void setup() 
{
  Serial.begin(9600); 
  CONFIG_LED_TEST;  
  VELOCIDAD_MOTOR; 
  INICIALIZAR_DISPLAY; 
  ENCENDER_DISPLAY;

 
}

void loop() 
{ 
  lectura_estados_afinador();
  ledTest();
  ActualizaLCD();
  afinador(); 
  recepcionSerie();
   
  
}

void ledTest()
{
  static int ledState= 0;
  static unsigned long antMillis = 0;
 
  if(millis() - antMillis < MS_INTERVALO_LED_TEST) return;    
  antMillis = millis();

  ledState = !ledState;
  ACTUALIZAR_LED_TEST(ledState);
}



void afinador() 
{
 
if(modo_manual)return;
afinado();
afinar_bemol();
afinar_sostenido();

}

void afinar_bemol(){
   /*
    Comprobar si el LED de Bemol (valores bemol <1  && sostenido > 1)

    Si se cumple esa condición gira el motor hacia la izquierda e imprime la palabra "Bemol"
  */
  if(LEER_BEMOL > 1) return;
  if(LEER_SOSTENIDO < 1) return;
  
  DIRECCION_MOTOR_IZQUIERDA;
  
   //Serial.println("bemol");
  //lcd.clear();
 //lcd.print("bemol");
    
}

void afinar_sostenido(){
  
  if(LEER_BEMOL < 1) return;
  if(LEER_SOSTENIDO > 1 ) return;
  
   
   DIRECCION_MOTOR_DERECHA;
   
    //Serial.println("sostenido");
   // lcd.clear();
  //lcd.print("Sostenido");
 } 

void afinado(){
  /*
    Comprobar si el LED de Bemol es mayor   a 1 (entra)
    Comprobar si el LED de Sostenido es mayor  a 1 (entra)

    Si se cumplen esas condiciones se detiene el motor e imprime la palabra "Afinado"
  */
  
  if(LEER_BEMOL  < 1) return;  // negados
  if(LEER_SOSTENIDO < 1)return;
    //Serial.println("Afinado");
   //lcd.clear();
  //lcd.print("Afinado");
   
}



void ActualizaLCD(){

  if(estado==estado_ant) return;
  estado_ant=estado;

  switch(estado)
    {
    case ESTADO_AFINADO:
           lcd.clear(); 
           lcd.print("Afinado"); break; 
           estado_ant=estado;
    case ESTADO_BEMOL:
           lcd.clear(); 
           lcd.print("Bemol"); break;
            estado_ant=estado; 
    case ESTADO_SOSTENIDO:
           lcd.clear(); 
           lcd.print("Sostendo"); break;
           estado_ant=estado;
     default: break;       
    }
  
  
  }



void recepcionSerie(){
 
   // Modo Manual Monitor Serie
   if(!Serial.available()) return;
   char letras = 0;
   
    letras = Serial.read();

    switch(letras) {
      case 'i':
      case 'I': DIRECCION_MOTOR_IZQUIERDA; break; 
      case 'd':
      case 'D': DIRECCION_MOTOR_DERECHA; break;
      case 'm':
      case 'M': modo_manual=1; break;
      case 'a':
      case 'A': modo_manual=0; break;
      default: break;
    }
       
 }


void lectura_estados_afinador()
  {
  unsigned int bemol, sostenido;
  
  bemol=LEER_BEMOL;
  sostenido=LEER_SOSTENIDO;
  
  if(bemol>1 && sostenido<1)      estado=ESTADO_SOSTENIDO;
  else if(bemol<1 && sostenido>1) estado=ESTADO_BEMOL;
  else                            estado=ESTADO_AFINADO;
  

  }
