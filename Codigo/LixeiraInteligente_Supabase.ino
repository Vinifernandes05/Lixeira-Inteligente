/*
  ================================================================
  LIXEIRA INTELIGENTE - ESP32 DevKit V1 + Arduino IoT Cloud
  ================================================================
  Sensores:
    - HC-SR04   -> mede distancia at o fundo do lixo (nivel) - na tampa
    - DHT11     -> temperatura e umidade - dentro da lixeira
    - MQ-135    -> qualidade do ar / gases - dentro da lixeira
    - TCRT5000  -> deteccao de presenca - fora da lixeira


  Atuadores:
    - Servo motor -> abre/fecha a tampa (controle direto por PWM)
    - LED Vermelho -> lixo cheio OU precisa trocar (gas alto)
    - LED Amarelo  -> atencao (nivel medio)
    - LED Verde    -> tudo certo


  Nuvem:
    - Arduino IoT Cloud funciona como dashboard E como "banco de
      dados" (historico das variaveis), substituindo o uso de um
      backend externo (ex: Supabase) para este projeto.
    - As variaveis nivelLixo, leituraGas, temperatura, umidade e
      tampaAberta sao definidas em thingProperties.h e sincronizadas
      automaticamente pela chamada ArduinoCloud.update() no loop().


  Logica principal:
    1) TCRT5000 detecta presenca -> servo move a tampa para ABERTA
    2) Enquanto a tampa estiver aberta, a leitura do HC-SR04 fica
       PAUSADA (para nao gerar leituras erradas de nivel)
    3) Sem presenca por um tempo -> servo move a tampa para FECHADA
    4) Apos fechar, aguarda estabilizacao e volta a ler o HC-SR04
    5) MQ-135 e DHT11 sao lidos continuamente (independem da tampa)
    6) LEDs refletem o estado do lixo (cheio / atencao / ok)


  IMPORTANTE - leia as observacoes no chat:
    - ECHO do HC-SR04 trabalha em 5V -> usar divisor de tensao para o ESP32 (3.3V)
    - Verificar a tensao de saida analogica do MQ-135 (ideal <= 3.3V)
    - O servo deve ter o fio de alimentacao (vermelho) em 5V e, se for
      um servo maior, alimentado por fonte externa
    - Preencha arduino_secrets.h (Wi-Fi e Device Key) e o Device ID em
      thingProperties.h antes de compilar
  ================================================================
*/


#include <DHT.h>
#include <ESP32Servo.h>
#include "thingProperties.h"


// ===================== PINOS =====================
// HC-SR04 (nivel do lixo - na tampa)
#define PINO_TRIG 5
#define PINO_ECHO 18


// DHT11 (temperatura/umidade - dentro da lixeira)
#define PINO_DHT 4
#define TIPO_DHT DHT11


// MQ-135 (gases - dentro da lixeira)
#define PINO_MQ135 34   // ADC1 - seguro para usar com Wi-Fi ativo


// TCRT5000 (presenca - fora da lixeira)
#define PINO_TCRT5000 19


// Servo motor - controla abertura/fechamento da tampa
#define PINO_SERVO 13


// LEDs indicadores
#define PINO_LED_VERMELHO 21
#define PINO_LED_AMARELO  22
#define PINO_LED_VERDE    23


DHT dht(PINO_DHT, TIPO_DHT);
Servo servoTampa;


// ===================== PARAMETROS (ajustar na calibracao) =====================
const float DIST_LIXEIRA_VAZIA_CM = 30.0;  // distancia sensor-fundo com lixeira vazia
const float DIST_LIXEIRA_CHEIA_CM = 5.0;   // distancia sensor-fundo com lixeira cheia


const int NIVEL_CHEIO_PERCENT   = 85; // >= isso -> LED vermelho (cheio)
const int NIVEL_ATENCAO_PERCENT = 50; // >= isso -> LED amarelo (atencao)


const int LIMIAR_GAS = 1800; // leitura crua do ADC (0-4095) - AJUSTAR EXPERIMENTALMENTE


// Muitos modulos TCRT5000 com comparador (LM393) ficam em LOW quando detectam objeto.
// Se o seu funcionar invertido, troque para HIGH.
const int NIVEL_PRESENCA_DETECTADA = LOW;


// Angulos do servo - AJUSTAR conforme a montagem mecanica da tampa
const int ANGULO_TAMPA_FECHADA = 0;
const int ANGULO_TAMPA_ABERTA  = 90;


// Tempo estimado para o servo concluir o movimento
const unsigned long TEMPO_ABERTURA_MS             = 600;
const unsigned long TEMPO_FECHAMENTO_MS            = 600;
const unsigned long TEMPO_SEM_PRESENCA_FECHAR_MS  = 4000;
const unsigned long TEMPO_ESTABILIZACAO_SENSOR_MS = 1000;


const unsigned long INTERVALO_LEITURA_ULTRASSOM_MS = 600;
const unsigned long INTERVALO_LEITURA_GAS_MS        = 1000;
const unsigned long INTERVALO_LEITURA_DHT_MS        = 2500;
const unsigned long INTERVALO_LEITURA_PRESENCA_MS   = 150;
const unsigned long INTERVALO_IMPRESSAO_MS          = 1000;


// ===================== ESTADOS =====================
enum EstadoTampa { TAMPA_FECHADA, TAMPA_ABRINDO, TAMPA_ABERTA, TAMPA_FECHANDO };
EstadoTampa estadoTampa = TAMPA_FECHADA;


unsigned long tInicioMovimento = 0;
unsigned long tUltimaPresenca  = 0;
unsigned long tTampaFechouEm   = 0;


bool leituraUltrassomHabilitada = true;


// Leitura auxiliar (nao vai para a nuvem, so para debug local)
float distanciaCm = -1;


// OBS: nivelLixo, leituraGas, temperatura, umidade e tampaAberta
// agora sao variaveis GLOBAIS definidas em thingProperties.h
// (sincronizadas com o Arduino IoT Cloud) - nao declare-as de novo aqui.


// Timers
unsigned long tUltimaLeituraUltra    = 0;
unsigned long tUltimaLeituraGas      = 0;
unsigned long tUltimaLeituraDHT      = 0;
unsigned long tUltimaLeituraPresenca = 0;
unsigned long tUltimaImpressao       = 0;


// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(1500);


  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  pinMode(PINO_TCRT5000, INPUT);


  servoTampa.attach(PINO_SERVO);
  servoTampa.write(ANGULO_TAMPA_FECHADA); // garante que comeca fechada


  pinMode(PINO_LED_VERMELHO, OUTPUT);
  pinMode(PINO_LED_AMARELO,  OUTPUT);
  pinMode(PINO_LED_VERDE,    OUTPUT);


  dht.begin();


  // ---- Arduino IoT Cloud ----
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();


  Serial.println(F("=== Lixeira Inteligente iniciada ==="));
}


// ===================== LOOP =====================
void loop() {
  ArduinoCloud.update(); // mantem a sincronizacao com a nuvem


  unsigned long agora = millis();


  // Presenca - verificada continuamente
  if (agora - tUltimaLeituraPresenca >= INTERVALO_LEITURA_PRESENCA_MS) {
    tUltimaLeituraPresenca = agora;
    verificarPresenca(agora);
  }


  // Maquina de estados da tampa / servo
  atualizarTampa(agora);


  // Reabilita leitura do ultrassonico apos estabilizacao (tampa fechada)
  if (!leituraUltrassomHabilitada && estadoTampa == TAMPA_FECHADA) {
    if (agora - tTampaFechouEm >= TEMPO_ESTABILIZACAO_SENSOR_MS) {
      leituraUltrassomHabilitada = true;
      Serial.println(F("Leitura do HC-SR04 reabilitada."));
    }
  }


  // HC-SR04 - so le se habilitado (tampa fechada e estavel)
  if (leituraUltrassomHabilitada && (agora - tUltimaLeituraUltra >= INTERVALO_LEITURA_ULTRASSOM_MS)) {
    tUltimaLeituraUltra = agora;
    lerUltrassonico();
  }


  // MQ-135
  if (agora - tUltimaLeituraGas >= INTERVALO_LEITURA_GAS_MS) {
    tUltimaLeituraGas = agora;
    leituraGas = analogRead(PINO_MQ135);
  }


  // DHT11
  if (agora - tUltimaLeituraDHT >= INTERVALO_LEITURA_DHT_MS) {
    tUltimaLeituraDHT = agora;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperatura = t;
    if (!isnan(h)) umidade = h;
  }


  atualizarLeds();
  imprimirStatus(agora);
}


// ===================== PRESENCA / TAMPA =====================
void verificarPresenca(unsigned long agora) {
  int leitura = digitalRead(PINO_TCRT5000);
  if (leitura == NIVEL_PRESENCA_DETECTADA) {
    tUltimaPresenca = agora;
    if (estadoTampa == TAMPA_FECHADA) {
      abrirTampa(agora);
    }
  }
}


void atualizarTampa(unsigned long agora) {
  switch (estadoTampa) {
    case TAMPA_FECHADA:
      // servo parado na posicao fechada, nada a fazer
      break;


    case TAMPA_ABRINDO:
      if (agora - tInicioMovimento >= TEMPO_ABERTURA_MS) {
        estadoTampa = TAMPA_ABERTA;
        Serial.println(F("Tampa ABERTA."));
      }
      break;


    case TAMPA_ABERTA:
      if (agora - tUltimaPresenca >= TEMPO_SEM_PRESENCA_FECHAR_MS) {
        fecharTampa(agora);
      }
      break;


    case TAMPA_FECHANDO:
      if (agora - tInicioMovimento >= TEMPO_FECHAMENTO_MS) {
        estadoTampa = TAMPA_FECHADA;
        tTampaFechouEm = agora;
        tampaAberta = false; // atualiza variavel da nuvem
        Serial.println(F("Tampa FECHADA. Aguardando estabilizacao do sensor..."));
      }
      break;
  }
}


void abrirTampa(unsigned long agora) {
  Serial.println(F("Presenca detectada! Abrindo tampa..."));
  estadoTampa = TAMPA_ABRINDO;
  tInicioMovimento = agora;
  leituraUltrassomHabilitada = false; // pausa leitura assim que comeca a abrir
  tampaAberta = true; // atualiza variavel da nuvem
  servoTampa.write(ANGULO_TAMPA_ABERTA);
}


void fecharTampa(unsigned long agora) {
  Serial.println(F("Sem presenca. Fechando tampa..."));
  estadoTampa = TAMPA_FECHANDO;
  tInicioMovimento = agora;
  servoTampa.write(ANGULO_TAMPA_FECHADA);
}


// ===================== SENSORES =====================
void lerUltrassonico() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);


  unsigned long duracao = pulseIn(PINO_ECHO, HIGH, 30000UL); // timeout 30ms
  if (duracao == 0) return; // leitura invalida, mantem valor anterior


  float dist = duracao * 0.0343f / 2.0f; // cm
  distanciaCm = dist;


  float faixa = DIST_LIXEIRA_VAZIA_CM - DIST_LIXEIRA_CHEIA_CM;
  float ocupacao = (DIST_LIXEIRA_VAZIA_CM - dist) / faixa * 100.0f;
  ocupacao = constrain(ocupacao, 0, 100);
  nivelLixo = (int) ocupacao;
}


// ===================== LEDs =====================
void atualizarLeds() {
  bool precisaTrocar = (leituraGas >= LIMIAR_GAS);
  bool lixoCheio      = (nivelLixo >= NIVEL_CHEIO_PERCENT);
  bool atencao         = (nivelLixo >= NIVEL_ATENCAO_PERCENT);


  if (lixoCheio || precisaTrocar) {
    digitalWrite(PINO_LED_VERMELHO, HIGH);
    digitalWrite(PINO_LED_AMARELO,  LOW);
    digitalWrite(PINO_LED_VERDE,    LOW);
  } else if (atencao) {
    digitalWrite(PINO_LED_VERMELHO, LOW);
    digitalWrite(PINO_LED_AMARELO,  HIGH);
    digitalWrite(PINO_LED_VERDE,    LOW);
  } else {
    digitalWrite(PINO_LED_VERMELHO, LOW);
    digitalWrite(PINO_LED_AMARELO,  LOW);
    digitalWrite(PINO_LED_VERDE,    HIGH);
  }
}


// ===================== DEBUG =====================
void imprimirStatus(unsigned long agora) {
  if (agora - tUltimaImpressao < INTERVALO_IMPRESSAO_MS) return;
  tUltimaImpressao = agora;


  Serial.print(F("Tampa: "));
  switch (estadoTampa) {
    case TAMPA_FECHADA:  Serial.print(F("FECHADA"));  break;
    case TAMPA_ABRINDO:  Serial.print(F("ABRINDO"));  break;
    case TAMPA_ABERTA:   Serial.print(F("ABERTA"));   break;
    case TAMPA_FECHANDO: Serial.print(F("FECHANDO")); break;
  }


  Serial.print(F(" | Nivel: "));
  Serial.print(nivelLixo);
  Serial.print(F("% ("));
  Serial.print(distanciaCm);
  Serial.print(F(" cm) | Gas(ADC): "));
  Serial.print(leituraGas);
  Serial.print(F(" | Temp: "));
  Serial.print(temperatura);
  Serial.print(F("C | Umid: "));
  Serial.print(umidade);
  Serial.println(F("%"));
}


