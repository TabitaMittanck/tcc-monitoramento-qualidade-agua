/*
  Teste do sensor de pH (ESP32)

  Converte a tensão do módulo em pH com uma reta de calibração
  ajustada por regressão em soluções tampão:
      pH = A * V + B
  Refaça a calibração (A e B) sempre que trocar ou envelhecer o eletrodo.
*/

const int   PIN_PH      = 33;      // ADC1 (GPIO 32-39)
const float ADC_REF     = 3.3f;
const int   ADC_RES     = 4095;
const int   N_AMOSTRAS  = 20;

// Coeficientes da regressão (3 pontos)
const float A = -4.9330607719f;
const float B = 19.1806641951f;

void setup() {
  Serial.begin(115200);
  analogSetPinAttenuation(PIN_PH, ADC_11db);  // Faixa útil ~0-3,3 V
  delay(200);
  Serial.println("Teste do sensor de pH (calibração por regressão)");
}

void loop() {
  long soma = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(PIN_PH);
    delay(10);
  }

  float media = soma / static_cast<float>(N_AMOSTRAS);
  float tensao = media * (ADC_REF / ADC_RES);
  float ph = A * tensao + B;

  Serial.printf("Bruto: %d | V: %.3f | pH: %.2f\n",
                static_cast<int>(media), tensao, ph);
  delay(1000);
}
