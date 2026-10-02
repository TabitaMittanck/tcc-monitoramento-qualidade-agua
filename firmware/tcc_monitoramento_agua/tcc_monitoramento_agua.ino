/*
  Monitoramento da qualidade da água em piscicultura (ESP32 + ThingSpeak)

  Lê temperatura, nível, turbidez, pH e TDS, aplica média móvel
  exponencial (EMA) e envia os dados ao ThingSpeak a cada
  INTERVALO_ENVIO_MS.

  Bibliotecas: ThingSpeak, OneWire, DallasTemperature
  Antes de compilar: copie secrets.h.example para secrets.h.
*/

#include <WiFi.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"
#include "secrets.h"

// =============================================================
// Estruturas
// =============================================================

// Média móvel exponencial: y[n] = a * x[n] + (1 - a) * y[n-1]
struct FiltroEMA {
  float valor = NAN;

  float atualiza(float amostra) {
    valor = isnan(valor) ? amostra
                         : ALPHA_EMA * amostra + (1.0f - ALPHA_EMA) * valor;
    return valor;
  }
};

struct Leituras {
  float temperaturaC;
  bool  nivelCheio;
  float turbidezPct;
  float ph;
  float tdsPpm;
};

// =============================================================
// Estado global
// =============================================================

OneWire oneWire(PIN_TEMPERATURA);
DallasTemperature sensorTemperatura(&oneWire);
WiFiClient clienteWifi;

FiltroEMA filtroTurbidez;
FiltroEMA filtroPh;
FiltroEMA filtroTds;

float ultimaTemperaturaC = 25.0f;  // Fallback se o DS18B20 falhar

// =============================================================
// Leitura dos sensores
// =============================================================

// Média de NUM_AMOSTRAS leituras do ADC, convertida em tensão
// e corrigida pelo fator de não linearidade do ESP32.
float lerTensao(uint8_t pino) {
  long soma = 0;
  for (int i = 0; i < NUM_AMOSTRAS; i++) {
    soma += analogRead(pino);
    delay(1);
  }
  float media = static_cast<float>(soma) / NUM_AMOSTRAS;
  return media * (V_REF / ADC_MAX) * FATOR_CORRECAO_ADC;
}

float lerTemperatura() {
  sensorTemperatura.requestTemperatures();
  float t = sensorTemperatura.getTempCByIndex(0);

  bool invalida = (t == DEVICE_DISCONNECTED_C) || t < -50 || t > 150;
  if (invalida) {
    return ultimaTemperaturaC;
  }
  ultimaTemperaturaC = t;
  return t;
}

bool lerNivelCheio() {
  return digitalRead(PIN_NIVEL) == LOW;  // Boia fecha o contato ao subir
}

// 0 % = água limpa, 100 % = água muito suja
float calcularTurbidez(float tensao) {
  int bruto = static_cast<int>(tensao * (ADC_MAX / V_REF));
  int pct = map(bruto,
                min(TURBIDEZ_LIMPA, TURBIDEZ_SUJA),
                max(TURBIDEZ_LIMPA, TURBIDEZ_SUJA),
                100, 0);
  return constrain(pct, 0, 100);
}

// pH com compensação térmica da sensibilidade do eletrodo (Nernst):
// a inclinação da reta varia com 298,15 K / T.
float calcularPh(float tensao, float tempC) {
  float fatorTemp = TEMP_REF_KELVIN / (273.15f + tempC);
  float sensibilidade = PH_SENSIBILIDADE * fatorTemp;
  float ph = PH_NEUTRO + (tensao - V_NEUTRO) / sensibilidade;
  return constrain(ph, 0.0f, 14.0f);
}

// TDS a partir da condutividade, compensada para 25 °C (2 %/°C).
float calcularTds(float tensao, float tempC) {
  float ecBruta = (tensao * 1000.0f) / TDS_DIVISOR_CALIB;
  float ecCompensada = ecBruta / (1.0f + 0.02f * (tempC - 25.0f));
  return constrain(ecCompensada * TDS_FATOR_CONVERSAO, 0.0f, TDS_MAX_PPM);
}

Leituras lerSensores() {
  Leituras l;
  l.temperaturaC = lerTemperatura();
  l.nivelCheio   = lerNivelCheio();
  l.turbidezPct  = filtroTurbidez.atualiza(calcularTurbidez(lerTensao(PIN_TURBIDEZ)));
  l.ph           = filtroPh.atualiza(calcularPh(lerTensao(PIN_PH), l.temperaturaC));
  l.tdsPpm       = filtroTds.atualiza(calcularTds(lerTensao(PIN_TDS), l.temperaturaC));
  return l;
}

// =============================================================
// Comunicação
// =============================================================

bool conectarWifi(unsigned long timeoutMs) {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < timeoutMs) {
    Serial.print(".");
    delay(500);
  }

  bool ok = WiFi.status() == WL_CONNECTED;
  Serial.println(ok ? "\nConectado!" : "\nSem Wi-Fi (seguindo offline).");
  return ok;
}

void enviarThingSpeak(const Leituras& l) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    WiFi.reconnect();
    return;
  }

  ThingSpeak.setField(CAMPO_TEMPERATURA, l.temperaturaC);
  ThingSpeak.setField(CAMPO_NIVEL, l.nivelCheio ? 1 : 0);
  ThingSpeak.setField(CAMPO_TURBIDEZ, l.turbidezPct);
  ThingSpeak.setField(CAMPO_PH, l.ph);
  ThingSpeak.setField(CAMPO_TDS, l.tdsPpm);

  int status = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_API_KEY);
  if (status == 200) {
    Serial.println("Dados enviados ao ThingSpeak.");
  } else {
    Serial.print("Falha no envio. Código HTTP: ");
    Serial.println(status);
  }
}

void imprimirSerial(const Leituras& l) {
  Serial.println("\n===== LEITURAS (filtradas e corrigidas) =====");
  Serial.printf("Temperatura : %.1f C\n", l.temperaturaC);
  Serial.printf("Nivel       : %s\n", l.nivelCheio ? "CHEIO" : "VAZIO");
  Serial.printf("Turbidez    : %.1f %%\n", l.turbidezPct);
  Serial.printf("pH          : %.2f\n", l.ph);
  Serial.printf("TDS         : %.0f ppm\n", l.tdsPpm);
  Serial.println("=============================================");
}

// =============================================================
// Arduino
// =============================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_NIVEL, INPUT_PULLUP);

  analogSetWidth(12);
  analogSetPinAttenuation(PIN_TURBIDEZ, ADC_11db);
  analogSetPinAttenuation(PIN_PH, ADC_11db);
  analogSetPinAttenuation(PIN_TDS, ADC_11db);

  sensorTemperatura.begin();
  ultimaTemperaturaC = lerTemperatura();

  conectarWifi(TIMEOUT_WIFI_MS);
  ThingSpeak.begin(clienteWifi);
}

void loop() {
  static unsigned long ultimoCiclo = 0;
  static bool primeiroCiclo = true;

  if (!primeiroCiclo && millis() - ultimoCiclo < INTERVALO_ENVIO_MS) {
    return;
  }
  primeiroCiclo = false;
  ultimoCiclo = millis();

  Leituras leituras = lerSensores();
  imprimirSerial(leituras);
  enviarThingSpeak(leituras);
}
