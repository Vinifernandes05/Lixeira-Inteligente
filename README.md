# 🗑️ Lixeira Inteligente IoT

## 📌 Sobre o Projeto

Este projeto consiste no desenvolvimento de uma **Lixeira Inteligente IoT**, criada com o objetivo de aplicar conceitos de **Internet das Coisas (IoT)**, sistemas embarcados, automação, sensores eletrônicos e monitoramento em tempo real.

O sistema foi desenvolvido utilizando um **ESP32 DevKit V1**, responsável por integrar diversos sensores e atuadores para automatizar o funcionamento da lixeira, tornando-a mais inteligente, higiênica e eficiente.

A lixeira é capaz de detectar a aproximação de uma pessoa através de um **sensor infravermelho**, acionando automaticamente a abertura da tampa utilizando um **servo motor**. Após alguns segundos, a tampa é fechada automaticamente.

Além da abertura automática, o projeto realiza o monitoramento contínuo do nível do lixo, temperatura, umidade e concentração de gases, disponibilizando essas informações em um **Dashboard na Arduino IoT Cloud**, permitindo acompanhamento remoto do estado e nível da lixeira.

---

## 🎯 Objetivos do Projeto

- Desenvolver automação utilizando ESP32
- Desenvolver uma solução inteligente para gerenciamento de resíduos
- Integrar múltiplos sensores em um único sistema
- Implementar monitoramento remoto em tempo real
- Trabalhar com sistemas embarcados
- Aplicar conceitos de Internet das Coisas (IoT)
- Desenvolver lógica de automação utilizando sensores e atuadores
- Melhorar conhecimentos em eletrônica e programação embarcada

---

## 🚀 Tecnologias/Componentes Utilizados

Este projeto foi desenvolvido utilizando as seguintes tecnologias/componentes:

### 🔹 Programação

- C++

### 🔹 Plataforma IoT

- Arduino IoT Cloud

### 🔹 Componentes

- ESP32 DevKit V1 (Wi-Fi + Bluetooth)
- Sensor Ultrassônico HC-SR04
- Sensor de Temperatura e Umidade DHT11
- Sensor de Gases MQ-135
- Sensor Infravermelho TCRT5000
- Fonte de Alimentação (Bateria 3.4V)
- Servo Motor
- LED Verde
- LED Vermelho
- Protoboard
- Resistores
- Fios Jumper

### 🔹 Outros

- Git
- GitHub

---

## ⚙️ Funcionalidades

✔️ Abertura automática da tampa através de sensor infravermelho

✔️ Fechamento automático da tampa após alguns segundos

✔️ Detecção da presença de usuários utilizando sensor TCRT5000

✔️ Medição do nível de lixo utilizando sensor ultrassônico HC-SR04

✔️ Interrupção temporária da leitura do sensor ultrassônico durante a abertura da tampa, evitando leituras incorretas

✔️ Monitoramento da temperatura interna da lixeira

✔️ Monitoramento da umidade interna

✔️ Indicação visual do estado da lixeira através de LEDs

✔️ LED Verde indicando funcionamento normal

✔️ LED Vermelho indicando lixeira cheia ou necessidade de troca devido à presença de gases

✔️ Monitoramento remoto através da Arduino IoT Cloud

✔️ Atualização em tempo real das variáveis do sistema

---

## 📂 Estrutura do Projeto

```bash
LIXEIRA-INTELIGENTE-IOT
│
├── codigo
│   └── Lixeira_Inteligente.ino              # Código principal do ESP32
│
├── dashboard
│   ├── Arduino IoT Cloud.pdf                # Configuração do Dashboard
│   └── Variaveis Dashboard.pdf              # Variáveis monitoradas
│
├── imagens
│   ├── Circuito Elétrico.png                # Esquemático eletrônico
│   ├── Protótipo.png                        # Protótipo da lixeira
│   └── Componentes.png                      # Componentes utilizados
│
├── videos
│   └── Demonstração.mp4                     # Funcionamento do projeto
│
└── README.md
```

---

## 🔄 Fluxo da Aplicação

1. O sensor infravermelho monitora continuamente a presença de pessoas próximas à lixeira.

2. Ao detectar aproximação, o servo motor realiza a abertura automática da tampa.

3. Durante a abertura, o sensor ultrassônico interrompe temporariamente suas leituras para evitar medições incorretas.

4. Após alguns segundos, a tampa é fechada automaticamente.

5. O sensor ultrassônico realiza a medição da distância entre a tampa e o lixo para determinar o nível de preenchimento.

6. O sensor DHT11 monitora continuamente temperatura e umidade interna.

7. O sensor MQ-135 mede a concentração de gases produzidos pelos resíduos.

8. O sistema processa todas as informações recebidas pelos sensores.

9. Os LEDs indicam visualmente o estado atual da lixeira.

10. Todas as informações são enviadas para a Arduino IoT Cloud, permitindo monitoramento remoto em tempo real.

---

## 📊 Variáveis Monitoradas

O sistema realiza o monitoramento das seguintes variáveis:

- 🌡️ Temperatura
- 💧 Umidade
- 📏 Distância interna (nível do lixo)
- ☁️ Concentração de gases
- 👋 Detecção de presença
- 🚪 Estado da tampa
- 🚦 Status da lixeira

---

## ☁️ Dashboard IoT

O projeto utiliza a **Arduino IoT Cloud** para monitoramento remoto.

O Dashboard permite acompanhar em tempo real:

- Temperatura
- Umidade
- Nível do lixo
- Concentração de gases
- Estado da tampa
- Presença detectada
- Status geral da lixeira

---

## 📞 Contato

👤 **Vinicius Sousa Fernandes**

- 📧 Email: vinifernandes2005@gmail.com
- 💼 LinkedIn: https://linkedin.com/in/viniciussousaf
- 💻 GitHub: https://github.com/Vinifernandes05

---
