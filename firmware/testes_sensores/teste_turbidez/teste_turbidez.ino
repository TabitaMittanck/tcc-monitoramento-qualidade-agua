/*
  Teste do sensor de turbidez (ESP32, GPIO 32)

  Calibração pelo monitor serial (baud 115200):
    'c' + Enter : registra a leitura atual como ÁGUA LIMPA
    'd' + Enter : registra a leitura atual como ÁGUA SUJA
  Sem calibrar, usa limites padrão aproximados.

  Resultado: 0 % = limpa, 100 % = muito suja.
*/

const int           PIN_TURBIDEZ   = 32;
const int           N_AMOSTRAS     = 20;
const unsigned long INTERVALO_MS   = 500;

// Limites padrão (observados nos testes: leituras entre ~700 e ~2000)
const int LIMPA_PADRAO = 2000;
const int SUJA_PADRAO  = 700;

int valorLimpa = -1;
int valorSuja  = -1;

int lerMedia() {
  long soma = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(PIN_TURBIDEZ);
    delay(5);
  }
  return static_cast<int>(soma / N_AMOSTRAS);
}

void calibrar(const char* nome, int& destino) {
  Serial.printf("Calibrando %s: posicione o sensor e pressione Enter.\n", nome);
  while (!Serial.available()) { delay(50); }
  Serial.readStringUntil('\n');
  destino = lerMedia();
  Serial.printf("Valor registrado (%s): %d\n", nome, destino);
}

void tratarComandosSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd == "c" || cmd == "C") calibrar("ÁGUA LIMPA", valorLimpa);
  if (cmd == "d" || cmd == "D") calibrar("ÁGUA SUJA", valorSuja);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  analogSetPinAttenuation(PIN_TURBIDEZ, ADC_11db);
  analogSetWidth(12);

  Serial.println("\n=== Sensor de turbidez (ESP32, GPIO 32) ===");
  Serial.println("'c' = calibrar água limpa | 'd' = calibrar água suja");
}

void loop() {
  tratarComandosSerial();

  int limpa = (valorLimpa < 0) ? LIMPA_PADRAO : valorLimpa;
  int suja  = (valorSuja  < 0) ? SUJA_PADRAO  : valorSuja;
  int alto  = max(limpa, suja);
  int baixo = min(limpa, suja);

  int leitura = lerMedia();

  int pct;
  if (leitura <= baixo)      pct = 100;
  else if (leitura >= alto)  pct = 0;
  else                       pct = constrain(map(leitura, baixo, alto, 100, 0), 0, 100);

  const char* estado = (pct <= 20) ? "LIMPA"
                     : (pct <= 50) ? "UM POUCO SUJA"
                                   : "MUITO SUJA";

  Serial.printf("Bruto: %d | Turbidez: %d %% | Estado: %s\n", leitura, pct, estado);
  delay(INTERVALO_MS);
}
