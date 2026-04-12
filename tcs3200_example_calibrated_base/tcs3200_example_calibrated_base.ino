/* Programa de ejemplo para el uso del sensor de color TCS3200
 * autor: Panjkrc
 * fecha: 12/14/2019
 * url: https://github.com/Panjkrc/TCS3200_library
 */


#include <tcs3200.h>

int red, green, blue, white;

tcs3200 tcs(8, 9, 11, 10, 12); // (S0, S1, S2, S3, pin de salida)  //

void setup() {
  Serial.begin(115200);
}

void loop() {
  //red = tcs.colorRead('r', 0);    //la escala tambien puede ponerse a 0%, 20%, y 100% (la escala por defecto es 20%)   ---    lee mas en: https://www.mouser.com/catalog/specsheets/TCS3200-E11.pdf
  //red = tcs.colorRead('r', 20);
  //red = tcs.colorRead('r', 100);

  red = tcs.colorRead('r');   //lee el valor de color para rojo
  Serial.print("R= ");
  Serial.print(red);
  Serial.print("    ");
  
  green = tcs.colorRead('g');   //lee el valor de color para verde
  Serial.print("G= ");
  Serial.print(green);
  Serial.print("    ");

  blue = tcs.colorRead('b');    //lee el valor de color para azul
  Serial.print("B= ");
  Serial.print(blue);
  Serial.print("    ");

  white = tcs.colorRead('c');    //lee el valor de color para blanco(claro)
  Serial.print("W(clear)= ");
  Serial.print(white);
  Serial.print("    ");

  Serial.println();

  delay(200);

}
