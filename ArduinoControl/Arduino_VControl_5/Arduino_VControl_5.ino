

// #include <SoftwareSerial.h>
// #include <Arduino.h>
#include <Servo.h>

#define RECEPTOR_CH3_PIN A1 // Canal 3 de velocidad
#define RECEPTOR_CH6_PIN A2 // Canal 6 de Auto/Manual
#define SALIDA_ESC_PIN 10

const unsigned long TIEMPO_PASO_TEST = 1000;

long baud = 9600;

// Voltaje

volatile int test = 0;
float iteracion_paso_test = 0;

Servo ESC;
int vel_real = 0;
int vAhora_real = 0;

float voltage_final = 0;
volatile int velocidad = 0;
int vAhora = 0;
int velmin = 0;
int incremento = 20;
int voltage_max = 12.0;

int iteraciones_escritura = 10;
int escribe = 0;

const int lecturas = 5;   // Total lecturas V
int index = 0;            // El indice de la lectura actual
float readings[lecturas]; // Lecturas de la entrada analogica
float Vtotal = 0.0;       // Total
float Vmedia = 0.0;       // Promedio

unsigned long timeout_pwm_micros = 50000;

volatile unsigned long siguientePaso = 0; // millis en los que tiene que saltar al siguiente paso

void test_funcion();

#define PIN_VOLTAJE A0
double VREF = 1.09; // valor de referencia interno del Atmega 328p, 1.1V teóricos
double R1 = 330000;  // 1M
double R2 = 6800;    // 100K

void setup()
{
  analogReference(INTERNAL);
  Serial.begin(baud);


  pinMode(RECEPTOR_CH3_PIN, INPUT);
  pinMode(RECEPTOR_CH6_PIN, INPUT);

  Serial.print("Voltaje Máximo: ");
  Serial.print(voltajeMaximo());
  Serial.println("V");
  Serial.println("");

  ESC.attach(SALIDA_ESC_PIN); // salida al ESC a D10


}


float voltajeMaximo()
{
  return VREF / (R2 / (R1 + R2));
}

float leerVoltaje()
{
  int valor = analogRead(PIN_VOLTAJE);
  //Serial.println(valor);
  double v = (valor * VREF) / 1024.0;
  return (v / (R2 / (R1 + R2)));
}



bool switchActivado()
{
  unsigned long value1 = pulseIn(RECEPTOR_CH6_PIN, HIGH, timeout_pwm_micros);
  unsigned long value2 = pulseIn(RECEPTOR_CH6_PIN, HIGH, timeout_pwm_micros);
  if ((value1 > 1600) && (value2 > 1600))
  {
    return false;
  }
  return true;
}

void printSerial()
{
  escribe++;
  if (escribe == iteraciones_escritura)
  {
    escribe = 0;

    Serial.print("Limitador activado:");
    Serial.println(switchActivado());
    Serial.print("V: ");
    Serial.println(voltage_final);
    Serial.print("Vel: ");
    Serial.println(velocidad);
    Serial.print("vAhora: ");

    Serial.println(vAhora);
  }
}

void loop()
{
  voltage_final = leerVoltaje();
unsigned long velocidad=pulseIn(RECEPTOR_CH3_PIN,HIGH);
  if (voltage_final < voltage_max && vAhora >= velmin)
  {
    vAhora = vAhora - incremento;
  }
  else if (vAhora < velocidad)
  {
    vAhora = vAhora + incremento;
  }
  else
  {
    vAhora = velocidad;
  }

  if (switchActivado())
  {
    ESC.writeMicroseconds(vAhora);

  }
  else
  {
    ESC.writeMicroseconds(velocidad);
  }

  printSerial();
}
