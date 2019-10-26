#include <Servo.h>




#include <SPI.h>
#include <Wire.h>



#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>





//#include <Fonts/FreeSans9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#include "HX711.h"


#define DOUT  3
#define CLK  4

HX711 balanza(DOUT, CLK);


Servo ESC;
int Receptor_CH3 = A1;
int Receptor_CH5 = A2;
int vel_real = 0;
int vAhora_real = 0;
float Varduino = 5; // valor real de la alimentacion de Arduino, Vcc
float R1 = 1000000; // 1M
float R2 = 100000; // 100K

float peso = 0;

float Vfinal = 0;
int velocidad = 1000;
int vAhora = 1000;
int velmin = 1000;
int incremento = 20;
int voltage_max = 12.5;

int iteraciones_escritura = 10;
int escribe = 0;

const int lecturas = 20; // Total lecturas V
int index             = 0; // El indice de la lectura actual
float readings[lecturas]; // Lecturas de la entrada analogica
float Vtotal           = 0.0; // Total
float Vmedia         = 0.0; // Promedio



void setup() {
  Serial.begin(9600);



  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }


  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);



  Serial.print("Lectura del valor del ADC:  ");
  Serial.println(balanza.read());
  Serial.println("No ponga ningun  objeto sobre la balanza");
  Serial.println("Destarando...");
  Serial.println("...");


  balanza.set_scale(119322); // Establecemos la escala
  balanza.tare(20);  //El peso actual es considerado Tara.

  Serial.println("Listo para pesar");




  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(12, 10);
  display.println(F("Amigus"));
  display.println(F("      Labs"));
  display.setCursor(15, 50);
  display.setTextSize(1);
  display.println(F("Power Module v0.1"));
  display.display();      // Show initial text


  pinMode(Receptor_CH3, INPUT);
  pinMode(Receptor_CH5, INPUT);

  Serial.print("Voltaje Máximo: ");
  Serial.print((int)(Varduino / (R2 / (R1 + R2))));
  Serial.println("V");
  Serial.println("");

  ESC.attach(10);  // salida al ESC a D10

  //delay(2000);
}

float leerVoltaje() {
  float v = (analogRead(0) * Varduino) / 1024.0;
  return (v / (R2 / (R1 + R2)));
}
bool switchActivado() {
  int value = pulseIn(Receptor_CH5, HIGH);
  if (value > 1600) {
    return false;
  }
  return true;
}
void printSerial() {
  escribe++;
  if (escribe == iteraciones_escritura) {
    escribe = 0;
    Serial.print("Limitador activado:");
    Serial.println(switchActivado());
    Serial.print("V: ");
    Serial.println(Vfinal);
    Serial.print("Vel: ");
    Serial.println(velocidad);
    Serial.print("vAhora: ");
    Serial.println(vAhora);
    Serial.print("Fuerza: ");
    //   Serial.print(peso);
    Serial.println(" kg");
  }
}
void loop() {

  Serial.print("--- 1 ---");
  Vfinal = leerVoltaje();

  velocidad = pulseIn(Receptor_CH3, HIGH);

  if (Vfinal < voltage_max && vAhora >= velmin) {
    vAhora = vAhora - incremento;
  } else if (vAhora < velocidad) {
    vAhora = vAhora + incremento;
  } else {
    vAhora = velocidad;
  }

  Serial.print("--- 2 ---");
  vel_real = map(velocidad, 995, 1989, 0, 10 );
  vAhora_real = map(vAhora, 995, 1989, 0, 10 );
  if (vAhora_real < 0) {
    vAhora_real = 0;
  }
  if (vel_real < 0) {
    vel_real = 0;
  }



  readings[index] = Vfinal;
  index = index + 1;

  if (index >= lecturas) {
    index = 0;   // ...volvemos al inicio:
  }
  Vtotal = 0;
  for (int i = 0; i <= lecturas; i++) {
    Vtotal = Vtotal + readings[i];
  }
  Vmedia = Vtotal / lecturas;


  String automatico;
  if (switchActivado()) {
    ESC.writeMicroseconds(vAhora);
    automatico = "A";
  } else {
    ESC.writeMicroseconds(velocidad);
    automatico = "M";

    vAhora_real = vel_real;



  }

  Serial.print("--- 3 ---");
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  //display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(F("Vel:"));
  if (vAhora_real < 10) {
    display.print(F(" "));
  }
  display.print(vAhora_real);
  display.print(F("/"));
  if (vel_real < 10) {
    display.print(F(" "));
  }
  display.print(vel_real);
  display.println(automatico);
  display.print(F("Vol:"));
  if (Vfinal < 10) {
    display.print(F(" "));
  }
  display.println(Vfinal);

  display.print(F("VoM:"));
  if (Vmedia < 10) {
    display.print(F(" "));
  }
  display.println(Vmedia);

  display.print(F("P: "));


  //peso = balanza.get_units(20), 2;


  peso = balanza.get_value(10), 0;
  if (peso < 10) {
    display.print(F(" "));
  }
  if (peso < 100) {
    display.print(F(" "));
  }

  display.println(peso);

  display.display();      // Show initial text
  Serial.println("--- 4 ---");
  printSerial();
}
