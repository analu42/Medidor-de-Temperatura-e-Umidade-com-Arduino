#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT11
#define DHTPIN 13
DHT_Unified dht(DHTPIN, DHTTYPE);

#define BOTAO 9
#define ANODO_COMUM true

const byte pinosSeg[7] = {2, 3, 4, 5, 6, 7, 8};      // A, B, C, D, E, F, G
const byte pinosDig[4] = {A0, 12, 11, 10};           // dígitos

const byte NUMEROS[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};
const byte GRAU    = 0b01100011;   // °
const byte LETRA_C = 0b00111001;   // C
const byte LETRA_H = 0b01110110;   // H

byte buffer[4] = {0, 0, GRAU, LETRA_C};

int tempAtual = 0;
int umidAtual = 0;
bool mostrarUmidade = false;

unsigned long ultimoRefresh = 0;
unsigned long ultimaLeitura = 0;
unsigned long ultimoBotao = 0;
bool botaoAnterior = HIGH;

void atualizarDisplay() {
  static byte dig = 0;

  for (byte i = 0; i < 4; i++)
    digitalWrite(pinosDig[i], ANODO_COMUM ? LOW : HIGH);

  for (byte s = 0; s < 7; s++) {
    bool ligado = buffer[dig] & (1 << s);
    digitalWrite(pinosSeg[s], ANODO_COMUM ? !ligado : ligado);
  }

  digitalWrite(pinosDig[dig], ANODO_COMUM ? HIGH : LOW);
  dig = (dig + 1) % 4;
}

// Monta o que aparece no display conforme o modo atual
void atualizarBuffer() {
  int valor = mostrarUmidade ? umidAtual : tempAtual;
  valor = constrain(valor, 0, 99);

  buffer[0] = (valor >= 10) ? NUMEROS[valor / 10] : 0;   // apaga zero à esquerda
  buffer[1] = NUMEROS[valor % 10];

  if (mostrarUmidade) {
    buffer[2] = 0;          // espaço
    buffer[3] = LETRA_H;    // H de umidade
  } else {
    buffer[2] = GRAU;
    buffer[3] = LETRA_C;
  }
}

void lerSensor() {
  sensors_event_t event;

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Erro na leitura da Temperatura!");
  } else {
    tempAtual = (int)event.temperature;
    Serial.print("Temperatura: ");
    Serial.print(tempAtual);
    Serial.println(" *C");
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Erro na leitura da Umidade!");
  } else {
    umidAtual = (int)event.relative_humidity;
    Serial.print("Umidade: ");
    Serial.print(umidAtual);
    Serial.println("%");
  }

  atualizarBuffer();
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(BOTAO, INPUT_PULLUP);
  for (byte i = 0; i < 7; i++) pinMode(pinosSeg[i], OUTPUT);
  for (byte i = 0; i < 4; i++) pinMode(pinosDig[i], OUTPUT);
}

void loop() {
  unsigned long agora = millis();

  // multiplexação do display
  if (agora - ultimoRefresh >= 2) {
    ultimoRefresh = agora;
    atualizarDisplay();
  }

  // leitura do sensor a cada 2 s
  if (agora - ultimaLeitura >= 2000) {
    ultimaLeitura = agora;
    lerSensor();
  }

  // botão: alterna entre temperatura e umidade
  bool botao = digitalRead(BOTAO);
  if (botao == LOW && botaoAnterior == HIGH && agora - ultimoBotao > 50) {
    ultimoBotao = agora;
    mostrarUmidade = !mostrarUmidade;
    atualizarBuffer();
  }
  botaoAnterior = botao;
}
