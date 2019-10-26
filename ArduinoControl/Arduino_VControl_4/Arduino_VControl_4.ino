#include <Servo.h>

#include <EEPROM.h>
#include <U8g2lib.h>
#include "HX711.h"
#include <Wire.h>


#include "Adafruit_SI1145.h"
Adafruit_SI1145 uv = Adafruit_SI1145();

const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 4;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
HX711 scale;

float calibration_factor = 105246 ; // lectura calibracion / kg -//- 1Kg = 1000g    // 68200/0.648
float romread = 0;
const byte interruptPin = 2;
int taremoi = 1;


float Visindex ;
float IRindex ;
float UVindex ;



Servo ESC;
int Receptor_CH3 = A1;
int Receptor_CH5 = A2;
int vel_real = 0;
int vAhora_real = 0;
float Varduino = 4.8; // valor real de la alimentacion de Arduino, Vcc
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

const int lecturas = 5; // Total lecturas V
int index             = 0; // El indice de la lectura actual
float readings[lecturas]; // Lecturas de la entrada analogica
float Vtotal           = 0.0; // Total
float Vmedia         = 0.0; // Promedio

float Ptotal;
float peso_media[lecturas]; // Lecturas de peso

void setup() {

  u8g2.begin();

  Serial.begin(9600);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  if (! uv.begin()) {
    Serial.println("Didn't find Si1145");
    //  while (1);
  }


  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.drawStr(5, 38, "@migus Labs");
    u8g2.setFont(u8g2_font_helvR08_tr);
    u8g2.drawStr(18, 52, "Power Module v0.1");

  } while ( u8g2.nextPage() );

  delay(300);
  EEPROM.get( 0, romread );
  if (romread == romread) {
    calibration_factor = romread;
  }

  scale.set_scale();
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), tare, CHANGE);
  long zero_factor = scale.read_average(); //Get a baseline reading




  pinMode(Receptor_CH3, INPUT);
  pinMode(Receptor_CH5, INPUT);

  Serial.print("Voltaje Máximo: ");
  Serial.print((int)(Varduino / (R2 / (R1 + R2))));
  Serial.println("V");
  Serial.println("");

  ESC.attach(10);  // salida al ESC a D10

  //delay(2000);
}

void tare() {
  taremoi = 1;
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
    Serial.print("Limitador activado:");    Serial.println(switchActivado());
    Serial.print("V: ");    Serial.println(Vfinal);
    Serial.print("Vel: ");    Serial.println(velocidad);
    Serial.print("vAhora: ");    Serial.println(vAhora);
    Serial.print("Fuerza: ");    Serial.print(peso);    Serial.println(" kg");
    Serial.print("Vis: "); Serial.println(Visindex);
    Serial.print("IR: "); Serial.println(IRindex);
    Serial.print("UV: ");  Serial.println(UVindex);
  }
}
void loop() {

  Vfinal = leerVoltaje();

  velocidad = pulseIn(Receptor_CH3, HIGH);

  if (Vfinal < voltage_max && vAhora >= velmin) {
    vAhora = vAhora - incremento;
  } else if (vAhora < velocidad) {
    vAhora = vAhora + incremento;
  } else {
    vAhora = velocidad;
  }

  vel_real = map(velocidad, 995, 1989, 0, 100 );
  vAhora_real = map(vAhora, 995, 1989, 0, 100 );
  if (vAhora_real < 0) {
    vAhora_real = 0;
  }
  if (vel_real < 0) {
    vel_real = 0;
  }

  if (taremoi != 0) {
    Serial.println("Calibrando Báscula");
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_logisoso16_tr);
      u8g2.drawStr(5, 38, "@migus Labs");
      u8g2.setFont(u8g2_font_helvR08_tr);
      u8g2.drawStr(18, 52, "Power Module v0.1");
      u8g2.setFont(u8g2_font_helvR08_tr);
      u8g2.drawStr(32, 64, "Calibrando...");
    } while ( u8g2.nextPage() );
    delay(200);
    scale.tare(); //Reset the scale to 0
    delay(200);
    taremoi = 0;
  }

  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  readings[index] = Vfinal;
  float peso_ahora =  scale.get_units();
  peso_media[index] = peso_ahora;

  index = index + 1;

  if (index >= lecturas) {
    index = 0;   // ...volvemos al inicio:
  }
  Vtotal = 0;
  Ptotal = 0;
  for (int i = 0; i <= lecturas; i++) {
    Vtotal = Vtotal + readings[i];
    Ptotal = Ptotal + peso_media[i];
  }

  Vmedia = Vtotal / lecturas;
  peso = Ptotal / lecturas; //



  if (peso < 0.09) {
    if (peso > -0.09) {
      peso = 0;
    }
  }



  String automatico;
  if (switchActivado()) {
    ESC.writeMicroseconds(vAhora);
    automatico = "A";
  } else {
    ESC.writeMicroseconds(velocidad);
    automatico = "M";

    vAhora_real = vel_real;



  }

  Visindex = uv.readVisible();
  IRindex = uv.readIR();
  UVindex = uv.readUV();
  UVindex /= 100.0;


  char g[10];
  dtostrf(peso, 5, 0, g);
  char vA[10];
  dtostrf(vAhora_real, 5, 0, vA);
  char vR[10];
  dtostrf(vel_real, 5, 0, vR);
  char vo[10];
  dtostrf(Vfinal, 5, 2, vo);
  char voM[10];
  dtostrf(Vmedia, 5, 2, voM);
  char Vis[10];
  dtostrf(Visindex, 5, 0 , Vis);
  char IR[10];
  dtostrf(IRindex, 5, 0, IR);
  char UV[10];
  dtostrf(UVindex, 5, 2, UV);

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_helvR14_tr);
    u8g2.drawStr(14, 14, vA);
    u8g2.drawStr(60, 14, vR);
    u8g2.drawStr(14, 32, vo);
    u8g2.drawStr(80, 32, voM);
    u8g2.drawStr(58, 12, "/");
    u8g2.setFont(u8g2_font_helvB24_tr);
    u8g2.drawStr(48, 64, g);
    u8g2.setFont(u8g2_font_helvR08_tr);
    u8g2.drawStr(122, 62, "g");
    u8g2.drawStr(0, 12, "Vel:");
    u8g2.drawStr(0, 30, "V:");
    u8g2.drawStr(60, 30, "VM:");
    u8g2.drawStr(0, 44, "Vis:");
    u8g2.drawStr(0, 54, "IR:");
    u8g2.drawStr(0, 64, "UV:");
    u8g2.drawStr(20, 44, Vis); //Vis
    u8g2.drawStr(20, 54, IR); //IR
    u8g2.drawStr(20, 64, UV); //UV
    u8g2.setFont(u8g2_font_tenstamps_mf);

    if (automatico == "A") {
      u8g2.drawStr(110, 12, "A");
    } else {
      u8g2.drawStr(110, 12, "M");

    }

  } while ( u8g2.nextPage() );



  if (Serial.available())
  {
    char temp = Serial.read();
    if (temp == '+' || temp == 'a') {
      calibration_factor += 10;
      EEPROM.put(0, calibration_factor);
    } else if (temp == '-' || temp == 'z') {
      calibration_factor -= 10;
      EEPROM.put(0, calibration_factor);
    }
  }




  printSerial();
}
