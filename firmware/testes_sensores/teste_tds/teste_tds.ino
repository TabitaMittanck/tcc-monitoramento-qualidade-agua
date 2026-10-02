/*
  Teste do sensor TDS (TDS Meter V1.0 + ESP32)

  Conversão simplificada tensão -> ppm. Cada módulo tem uma curva
  ligeiramente diferente: ajuste FATOR_PPM com uma solução de
  referência. O firmware final usa compensação de temperatura.
*/

const int   PIN_TDS  = 35;      // ADC1
const float ADC_REF  = 3.3f;
const int   ADC_RES  = 4095;
const float FATOR_PPM = 500.0f; // ppm por volt (aproximado)

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Teste do sensor TDS");
}

void loop() {
  int bruto = analogRead(PIN_TDS);
  float tensao = bruto * (ADC_REF / ADC_RES);
  float tds = tensao * FATOR_PPM;

  Serial.printf("Bruto: %d | V: %.3f | TDS: %.1f ppm\n", bruto, tensao, tds);
  delay(1000);
}
