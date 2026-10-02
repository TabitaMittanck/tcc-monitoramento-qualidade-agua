/*
  Teste do sensor de nível (chave boia)

  Ligação: um terminal no GPIO 25, o outro no GND.
  Usa o pull-up interno: boia em cima = contato fechado = LOW.
*/

const int PIN_NIVEL = 25;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_NIVEL, INPUT_PULLUP);
}

void loop() {
  bool cheio = digitalRead(PIN_NIVEL) == LOW;
  Serial.println(cheio ? "Nível ALTO: tanque cheio (boia subiu)"
                       : "Nível BAIXO: tanque vazio (boia desceu)");
  delay(1000);
}
