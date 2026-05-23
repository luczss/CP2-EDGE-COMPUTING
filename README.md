# 🍷 Vinheria Agnello — Sistema Inteligente de Monitoramento Ambiental

<p align="center">
  <img src="https://img.shields.io/badge/FIAP-Edge%20Computing-red?style=for-the-badge">
  <img src="https://img.shields.io/badge/Arduino-C%2FC%2B%2B-blue?style=for-the-badge">
  <img src="https://img.shields.io/badge/Status-Concluído-success?style=for-the-badge">
  <img src="https://img.shields.io/badge/Checkpoint-02-purple?style=for-the-badge">
</p>

---

# 📖 Sobre o Projeto

O projeto **Vinheria Agnello** foi desenvolvido como parte do **Checkpoint 02** da disciplina de **Edge Computing & Computer Systems** da FIAP.

O sistema tem como objetivo realizar o monitoramento inteligente das condições ambientais de uma vinheria, garantindo que fatores essenciais para a conservação dos vinhos — como:

- 🌡️ Temperatura
- 💧 Umidade
- 💡 Luminosidade

permaneçam dentro dos níveis ideais.

A solução utiliza sensores integrados ao Arduino para capturar dados em tempo real, exibir informações em um display LCD, gerar alertas visuais e sonoros e armazenar registros históricos na EEPROM.

---

# 🔗 Simulação no Wokwi

https://wokwi.com/projects/464367007306848257

---

# ▶️ Vídeo no YouTube

https://youtu.be/6CPv-DlaVm8

---

# 🚀 Funcionalidades

## ✅ Monitoramento em Tempo Real

Leitura contínua da:

- Temperatura
- Umidade
- Luminosidade

---

## ✅ Sistema Inteligente de Alertas

O sistema possui três níveis de status:

| Status | LED | Condição |
|---|---|---|
| Normal | 🟢 Verde | Ambiente ideal |
| Atenção | 🟡 Amarelo | Valores próximos do limite |
| Crítico | 🔴 Vermelho | Valores fora do ideal |

Além disso:

- 🔊 Buzzer sonoro para alertas críticos
- Frequências diferentes para WARNING e CRITICAL

---

## ✅ Display LCD Interativo

Exibição automática de:

- Luminosidade
- Temperatura
- Umidade
- Horário em tempo real via RTC

Com:

- Troca automática de telas
- Ícones personalizados
- Animação inicial

---

## ✅ Sistema de Configuração (Menu Setup)

Através de botões físicos, o usuário pode configurar:

- 🌍 Fuso horário UTC
- 🌡️ Unidade de temperatura:
  - Celsius
  - Fahrenheit
- 🌐 Idioma:
  - Português
  - Inglês

---

## ✅ Armazenamento de Logs

Utilização da EEPROM para:

- Registro histórico dos sensores
- Salvamento de estados críticos
- Verificação de integridade com checksum

---

## ✅ Calibração Automática de Luminosidade

O sensor LDR realiza:

- Ajuste automático de valores mínimos e máximos
- Conversão dinâmica para percentual de luminosidade

---

# 🛠️ Tecnologias Utilizadas

## Hardware

- Arduino Uno
- Sensor DHT22
- Sensor LDR
- RTC DS1307
- LCD I2C 16x2
- EEPROM
- LEDs
- Buzzer
- Push Buttons

---

## Software

- Arduino IDE
- Linguagem C/C++

### Bibliotecas

- Wire.h
- LiquidCrystal_I2C.h
- DHT.h
- EEPROM.h
- RTClib.h

---

# 🔌 Arquitetura do Sistema

```text
                        +------------------+
                        |   Sensor DHT22   |
                        +------------------+
                                 |
                                 v

      +-------------+     +-------------+       +--------------+
      | Sensor LDR  |     |  Arduino    |       | LCD I2C 16x2 |
      +-------------+     +-------------+       +--------------+

                                 |
                                 v

                      +------------------+
                      | LEDs + Buzzer    |
                      +------------------+
                                 |
                                 v

                      +------------------+
                      | EEPROM + RTC     |
                      +------------------+
```

---

# 📋 Estrutura do Código

| Função | Responsabilidade |
|---|---|
| showLogo() | Exibe animação inicial |
| showLux() | Mostra luminosidade |
| showTemp() | Mostra temperatura |
| showHum() | Mostra umidade |
| handleAlerts() | Gerencia LEDs e buzzer |
| saveLog() | Salva logs na EEPROM |
| handleMenu() | Gerencia menu de configuração |
| calibrateLDR() | Faz calibração automática |
| btnPressed() | Realiza debounce dos botões |

---

# ⚙️ Regras de Negócio

## 🌡️ Temperatura Ideal

| Status | Faixa |
|---|---|
| OK | 12°C ~ 16°C |
| Atenção | 10°C ~ 18°C |
| Crítico | Fora desse intervalo |

---

## 💧 Umidade Ideal

| Status | Faixa |
|---|---|
| OK | 60% ~ 75% |
| Atenção | 50% ~ 80% |
| Crítico | Fora desse intervalo |

---

## 💡 Luminosidade Ideal

| Status | Faixa |
|---|---|
| OK | 30% ~ 70% |
| Atenção | 20% ~ 80% |
| Crítico | Fora desse intervalo |

---

# 🧠 Conceitos Aplicados

- Edge Computing
- Sistemas Embarcados
- Monitoramento Inteligente
- Sensoriamento Ambiental
- Persistência de Dados
- Interação Homem-Máquina
- Tratamento de Eventos
- Debounce de Botões
- Programação Modular
- Calibração Dinâmica

---

# ▶️ Como Executar

## 1️⃣ Clone o repositório

```bash
git clone https://github.com/luczss/CP2-EDGE-COMPUTING.git
```

---

## 2️⃣ Abra na Arduino IDE

Abra o arquivo:

```bash
vinheria_agnello.ino
```

---

## 3️⃣ Instale as bibliotecas

Na Arduino IDE:

```text
Sketch > Include Library > Manage Libraries
```

Instale:

- LiquidCrystal_I2C
- DHT sensor library
- RTClib

---

## 4️⃣ Faça upload para o Arduino

- Selecione a porta COM
- Escolha a placa Arduino Uno
- Clique em Upload

---

# 📷 Demonstração Esperada

## Tela Inicial

```text
VINHERIA AGNELLO
MOOCA
```

---

## Tela de Temperatura

```text
Temperatura:
14.5 C   OK
```

---

## Tela de Umidade

```text
Umidade:
65.2%  OK
```

---

## Tela de Luminosidade

```text
Luz:45%
```

---

# 👨‍💻 Integrantes do Grupo

| Nome | RM |
|---|---|
| Lucas Dos Santos Oliveira | RM571718 |
| Eric Kang | RM572575 |
| Diego Caio De Ulhôa Augusto | RM572190 |
| Enzo Leme | RM572148 |

---

# 🏫 Instituição

**FIAP — Faculdade de Informática e Administração Paulista**

## Disciplina
Edge Computing & Computer Systems

---

# 📄 Licença

Projeto desenvolvido exclusivamente para fins acadêmicos.

---

# ⭐ Considerações Finais

O projeto **Vinheria Agnello** demonstra a aplicação prática de conceitos de:

- Edge Computing
- Automação
- Sistemas embarcados
- Monitoramento ambiental inteligente

utilizando uma arquitetura robusta, modular e escalável para controle de ambientes sensíveis como vinherias.

---

<p align="center">
🍷 Vinheria Agnello • FIAP • Edge Computing & Computer Systems
</p>
