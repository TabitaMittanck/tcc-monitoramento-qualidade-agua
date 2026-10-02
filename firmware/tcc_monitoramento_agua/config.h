#pragma once

#include <Arduino.h>

// =============================================================
// Configurações do firmware (pinos, calibração e temporização)
// Credenciais NÃO ficam aqui: veja secrets.h.example
// =============================================================

// ---------- Pinos (ESP32 DevKit, 30 pinos) ----------
// Os sensores analógicos usam ADC1 (GPIO 32-39), que continua
// disponível com o Wi-Fi ligado. GPIO 4 e 25 (ADC2) são usados
// apenas como entradas digitais.
constexpr uint8_t PIN_TEMPERATURA = 4;   // DS18B20 (OneWire)
constexpr uint8_t PIN_NIVEL       = 25;  // Chave boia
constexpr uint8_t PIN_TURBIDEZ    = 32;  // Sensor de turbidez
constexpr uint8_t PIN_PH          = 33;  // Módulo de pH
constexpr uint8_t PIN_TDS         = 35;  // TDS Meter V1.0

// ---------- Temporização ----------
constexpr unsigned long INTERVALO_ENVIO_MS  = 30000;  // Amostragem/envio
constexpr unsigned long TIMEOUT_WIFI_MS     = 20000;  // Espera na conexão inicial

// ---------- ADC ----------
constexpr int   NUM_AMOSTRAS = 20;      // Média por leitura
constexpr float V_REF        = 3.3f;
constexpr int   ADC_MAX      = 4095;    // 12 bits
constexpr float ALPHA_EMA    = 0.20f;   // Peso da média móvel exponencial

// Correção do ADC do ESP32 (não linear): 2,31 V no multímetro
// contra 2,058 V lidos pelo ESP32 => 2,31 / 2,058 ≈ 1,1224
constexpr float FATOR_CORRECAO_ADC = 1.1224f;

// ---------- Calibração: turbidez ----------
// Leituras brutas do ADC para água limpa e água muito suja.
constexpr int TURBIDEZ_LIMPA = 2000;
constexpr int TURBIDEZ_SUJA  = 700;

// ---------- Calibração: pH ----------
// Ponto neutro e sensibilidade do eletrodo, com compensação
// térmica (referência de 25 °C).
constexpr float PH_NEUTRO         = 7.02f;    // pH no ponto de referência
constexpr float V_NEUTRO          = 2.305f;   // Tensão (V) no ponto neutro
constexpr float PH_SENSIBILIDADE  = 0.1933f;  // V por unidade de pH a 25 °C
constexpr float TEMP_REF_KELVIN   = 298.15f;

// ---------- Calibração: TDS ----------
constexpr float TDS_FATOR_CONVERSAO = 0.5f;    // EC (µS/cm) -> TDS (ppm)
constexpr float TDS_DIVISOR_CALIB   = 29.82f;  // mV -> µS/cm (ajuste empírico)
constexpr float TDS_MAX_PPM         = 5000.0f;

// ---------- Campos do canal ThingSpeak ----------
constexpr uint8_t CAMPO_TEMPERATURA = 1;
constexpr uint8_t CAMPO_NIVEL       = 2;  // 1 = cheio, 0 = vazio
constexpr uint8_t CAMPO_TURBIDEZ    = 3;  // %
constexpr uint8_t CAMPO_PH          = 4;
constexpr uint8_t CAMPO_TDS         = 5;  // ppm
