#include <Arduino.h>
#include <Wire.h>
#include <WS_V2.h>
#include <Adafruit_MLX90640.h>

#define ARRAY_SIZE 7

Adafruit_MLX90640 mlx;
TwoWire mxl_bus = TwoWire(0);

float getMinTemp();
float getMaxTemp();
float getAvgBottomTemp();
void checkAndInsertTopTemps(float temp);
void checkAndInsertBottomTemps(float temp);

float topTemps[ARRAY_SIZE] = {0}; // Array para almacenar las 5 temperaturas más altas
float bottomTemps[ARRAY_SIZE] = {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY}; // Inicializa con valores infinitos

void setup() {
  Serial.begin(115200);
  while (!Serial); // Espera a que el puerto serie esté disponible

  while (!mxl_bus.begin(I2C_SDA, I2C_SCL)){
    Serial.println("Error al iniciar el bus I2C");
    delay(1000);
  }

  Serial.println("Inicializando MLX90640...");
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &mxl_bus)) {
    Serial.println("¡Error al iniciar el sensor MLX90640!");
    while (1);
  }
  
  Serial.println("Sensor MLX90640 iniciado correctamente");

  mlx.setRefreshRate(MLX90640_1_HZ);
}

void loop() {
  float pixelTemps[32 * 24]; // Array temporal para almacenar las temperaturas de todos los píxeles

  if (!mlx.getFrame(pixelTemps)) {
    for (int i = 0; i < 32 * 24; i++)  checkAndInsertBottomTemps(pixelTemps[i]);

    // Imprimir los 5 valores más altos
    Serial.println("Top" + String(ARRAY_SIZE)+" temperaturas más bajas:");
    for (int i = 0; i < ARRAY_SIZE; i++) Serial.print(String(bottomTemps[i])+ " \t");

    Serial.println(); 

    // min and max temps
    Serial.println("Min: "+String(getMinTemp()));
    Serial.println("Max: "+String(getMaxTemp()));

    for (int i = 0; i < ARRAY_SIZE; i++) bottomTemps[i] = INFINITY;

  } else {
    Serial.println("Error al leer el frame del sensor MLX90640");
  }

  Serial.println();
  delay(1000); // Espera un segundo antes de leer nuevamente
}

float getAvgBottomTemp(){
  float sum = 0;
  for (int i = 0; i < ARRAY_SIZE; i++) sum += bottomTemps[i];
  return sum / ARRAY_SIZE;
}

float getMinTemp() {
  float minTemp = bottomTemps[0];
  for (int i = 1; i < ARRAY_SIZE; i++) {
    if (bottomTemps[i] < minTemp) minTemp = bottomTemps[i];
  }
  return minTemp;
}

float getMaxTemp() {
  float maxTemp = bottomTemps[0];
  for (int i = 1; i < ARRAY_SIZE; i++) {
    if (bottomTemps[i] > maxTemp) maxTemp = bottomTemps[i];
  }
  return maxTemp;
}

void checkAndInsertTopTemps(float temp) {
  if (temp > topTemps[0]) { 
    topTemps[0] = temp; 
    
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
      if (topTemps[i] > topTemps[i + 1]) {
        float tmp = topTemps[i];
        topTemps[i] = topTemps[i + 1];
        topTemps[i + 1] = tmp;
      }
    }
  }
}

void checkAndInsertBottomTemps(float temp) {
  if (temp < bottomTemps[ARRAY_SIZE - 1]) {
    bottomTemps[ARRAY_SIZE - 1] = temp; // Reemplaza el valor más alto con la nueva temperatura
    // Ordena el array para mantener el valor más alto en bottomTemps[4]
    for (int i = ARRAY_SIZE - 1; i > 0; i--) {
      if (bottomTemps[i] < bottomTemps[i - 1]) {
        float tmp = bottomTemps[i];
        bottomTemps[i] = bottomTemps[i - 1];
        bottomTemps[i - 1] = tmp;
      }
    }
  }
}