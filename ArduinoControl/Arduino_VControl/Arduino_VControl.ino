#include <Servo.h>

Servo ESC;
int Receptor_CH3 = A1;
int Receptor_CH5 = A2;

float Varduino = 5; // valor real de la alimentacion de Arduino, Vcc
float R1 = 1000000; // 1M
float R2 = 100000; // 100K

float Vfinal = 0;
int velocidad = 1000;
int vAhora = 1000;
int velmin = 1000;
int incremento = 20;
int voltage_max = 12.5;

int iteraciones_escritura=50;
int escribe = 0;


void setup() {
pinMode(Receptor_CH3, INPUT);
pinMode(Receptor_CH5, INPUT);
Serial.begin(9600);
Serial.print("Voltaje Máximo: ");
Serial.print((int)(Varduino / (R2 / (R1 + R2))));
Serial.println("V");
Serial.println(""); 

ESC.attach(10);  // salida al ESC a D10

//delay(2000);
}

float leerVoltaje(){
 float v = (analogRead(0) * Varduino) / 1024.0;
 return(v / (R2 / (R1 + R2)));
}
bool switchActivado(){
int value = pulseIn(Receptor_CH5,HIGH); 
  if (value >1600){
    return false;
  }
  return true;
}
void printSerial(){
  escribe++;
  if (escribe==iteraciones_escritura){
    escribe=0;
    Serial.print("Limitador activado:");
    Serial.println(switchActivado());
    Serial.print("V: ");
    Serial.println(Vfinal);
    Serial.print("Vel: ");
    Serial.println(velocidad);
    Serial.print("vAhora: ");
    Serial.println(vAhora);
  }
}
void loop() {

Vfinal = leerVoltaje();

velocidad=pulseIn(Receptor_CH3,HIGH);

if (Vfinal<voltage_max && vAhora>=velmin){
  vAhora = vAhora - incremento;
}else if (vAhora<velocidad) {
  vAhora=vAhora+incremento;
}else{
  vAhora=velocidad;
}

if (switchActivado()){
  ESC.writeMicroseconds(vAhora);
}else{
  ESC.writeMicroseconds(velocidad);
}

printSerial();
}
