/*
  Teste do sensor de temperatura DS18B20 (OneWire + ESP32)

  Ligação: VCC -> 3V3, GND -> GND, DQ -> GPIO 4,
  com resistor pull-up de 10 kΩ entre DQ e VCC.
*/

#include <OneWire.h>
#include <DallasTemperature.h>

const int PIN_ONE_WIRE = 4;

OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature sensores(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(1000);
  sensores.begin();

  int encontrados = sensores.getDeviceCount();
  Serial.printf("Sensores encontrados: %d\n", encontrados);
  if (encontrados == 0) {
    Serial.println("Nenhum sensor detectado. Verifique pino, pull-up e conexões.");
  }
}

void loop() {
  sensores.requestTemperatures();
  float tempC = sensores.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Erro: sensor desconectado.");
  } else {
    Serial.printf("Temperatura: %.2f C\n", tempC);
  }
  delay(2000);
}
