# 💊 Dispenser Automático de Remédios

## 📌 Descrição do Projeto

Este projeto tem como objetivo o desenvolvimento de um **dispenser automático de remédios**, que visa auxiliar na administração correta e eficiente de medicamentos no dia a dia.

O sistema possui:

- Compartimento para **até 5 remédios diferentes**.
- Uma **tela de controle** para visualização dos medicamentos cadastrados.
- Uma **tela de cadastro** para inserção, edição e remoção dos remédios.
- Integração com o **Telegram** para envio de alertas em tempo real.
- Registro e monitoramento do funcionamento em uma **planilha do Excel**.
- Armazenamento de dados em um **banco de dados**.

---

## ⚙️ Funcionalidades

- ✅ **Cadastro de remédios** com horários e dias específicos.
- ⏰ **Despejo automático** do remédio no horário agendado.
- 💬 **Alerta via Telegram** na hora de tomar o medicamento.
- 📉 **Controle de estoque**: monitora a quantidade disponível em cada compartimento.
- 🔔 **Alerta de estoque baixo**: quando o compartimento estiver vazio, o sistema envia um aviso via Telegram.
- 📋 **Visualização dos remédios cadastrados** e status de cada um na tela de controle.
- 📊 **Atualização automática da planilha Excel** com o histórico de funcionamento.

---

## 🔒 Restrições

- O **nome do remédio** pode ter no máximo **20 caracteres**.
- Cada remédio pode ter no máximo **3 horários** cadastrados.
- É permitida apenas **1 dose por vez** para cada horário.
- O sistema suporta até **5 remédios cadastrados simultaneamente** (um por compartimento).

---

## 🧠 Funcionamento dos Códigos

## 💻 1) **Interface de cadastro**

  O sistema foi desenvolvido em **Python**, com a interface construída usando **Tkinter** e o armazenamento de dados feito em **MongoDB**.
 
  #### 🖥️ Telas do Sistema
 
  - A **tela de controle** é a **interface principal** do programa:
    - Exibe a **agenda semanal**, com os remédios programados por dia e horário.
    - Mostra o **estoque atual de cada compartimento** em tempo real.
 
  - A **tela de cadastro** (ou edição) é **aberta separadamente** quando o usuário deseja:
    - Inserir um novo remédio com seus dados.
    - Modificar um remédio existente.
 
  #### 🔗 Comunicação
 
  O sistema utiliza **MQTT** para:
  - **Receber comandos externos** que disparam a liberação do remédio no horário certo.
  - **Atualizar o estoque** automaticamente no banco de dados.
  - **Enviar alertas** via MQTT quando o compartimento está vazio.
 
  Além disso, o sistema se comunica com:
  - O **Telegram**, para envio de mensagens de alerta ao usuário.
  - Uma **planilha Excel**, onde registra os dados e histórico de funcionamento do dispenser.


## 🤖 2) Sistema Embarcado (ESP32)

O firmware foi desenvolvido em **C++** com o **Arduino Framework**, executando no **ESP32**, que atua como cérebro do sistema de liberação.

### ⚙️ Funcionalidades principais

- Conexão segura via **Wi-Fi com HTTPS**.
- Sincronização de horário com **servidores NTP** (UTC-3, Brasil).
- Assinatura de tópicos **MQTT** para:
  - Cadastro e atualização de remédios.
  - Reabastecimento.
  - Testes manuais.

### Bibliotecas utilizadas:

- `WiFi.h`: conexão à rede Wi-Fi.
- `WiFiClientSecure.h`: comunicação segura via HTTPS.
- `MQTT.h`: envio/recebimento de mensagens por MQTT.
- `ArduinoJson.h`: leitura e manipulação de JSON.
- `time.h`: sincronização de horário via NTP.
- `Serial1`: comunicação com a unidade de motor e tela.
- `EEPROM.h`: armazenamento persistente.

### 🧠 Lógica embarcada

- A cada 10 segundos:
  - Verifica a **hora atual**.
  - Checa se há algum lembrete programado.
  - Se sim, **envia sinal pela `Serial1`** para liberar o compartimento.
  - **Publica no tópico `hora_remedio`** o nome do remédio.

- Usa variáveis locais para:
  - Nome dos remédios por compartimento.
  - Até 3 horários por dia por remédio.
  - Dias da semana em formato binário `"0110010"`.

- Evita envio duplicado de lembrete no mesmo minuto.

### 🔗 Comunicação

- **MQTT**:
  - Recebe configurações em JSON:

    ```json
    {
      "remedio": "Dipirona",
      "dias": ["Segunda", "Quarta"],
      "horarios": ["10:30", "22:00"],
      "compartimento": 2
    }
    ```

  - Envia lembretes:

    ```
    Hora do Dipirona
    ```

- **Serial1**:
  - Comunicação com display ou sistema mecânico (motor), exemplo de comando:

    ```
    ADD: Dipirona,2,630,1320,0101000
    ```

- **Serial (USB)**:
  - Saída para depuração e testes

---

## 📱 3) Módulo com Tela de Controle e Motor (Interface Local)

Este módulo é responsável por **exibir os remédios na tela**, permitir **interação local via toque** e **liberar os comprimidos** com controle de motor de passo.

### 🧰 Componentes Utilizados

- **ESP32** (com Serial e controle do motor)
- **Tela LCD Touch 2.4” (MCUFRIEND)**
- **Motor de passo 28BYJ-48** + driver ULN2003
- **Sensor IR (presença do dedo/remédio retirado)**
- **EEPROM interna**

### Bibliotecas utilizadas:

- `MCUFRIEND_kbv`: controle gráfico da tela LCD.
- `TouchScreen`: leitura da tela resistiva.
- `JKSButton`, `GFButton`: botões gráficos e de toque.
- `Stepper`: controle do motor de passo.
- `EEPROM`: armazenamento dos dados dos remédios.


### 🧠 Funcionalidades

- Interface gráfica com exibição de:
  - Lista de remédios.
  - Horários programados.
  - Dias da semana de cada remédio (com check verde ✅).
- Leitura de comandos da `Serial1`:
  - `"ADD:Nome,NumHorarios,Min1,Min2,...,diasBinario"` → Cadastra e salva na EEPROM.
  - `"RESET"` → Apaga todos os dados.
  - `"zero"` → Define a posição inicial do motor.
  - `"0"` a `"4"` → Seleciona e libera o remédio no tubo correspondente.

- Controle do motor de passo:
  - Gira até o compartimento correto.
  - Executa movimento de liberação do comprimido.
  - Realinha para posição inicial (zero).

- Uso da **EEPROM**:
  - Armazena até 5 remédios com seus dados.
  - Dados persistem mesmo após desligar o dispositivo.

- Tela Touch:
  - Permite **navegar nos remédios cadastrados**.
  - Exibe mensagem visual de **“Tomar agora”** após liberação.

### 🧪 Exemplo de Comando Recebido via Serial

```plaintext
ADD:Dipirona,2,480,1320,1111100
```

- Nome: Dipirona
- 2 horários: 08:00 (480 min), 22:00 (1320 min)
- Dias: segunda a sexta (em binário)

### 💾 Exibição na EEPROM

```c
struct Remedio {
  char nome[20];
  int horarios[3];       // até 3 horários por remédio
  int numHorarios;
  bool diasSemana[7];    // Dom a Sáb
};
```

## 🧰 Tecnologias Utilizadas

### 💻 1. Software de Cadastro (Desktop)

- **Python**: linguagem principal da lógica do sistema.
- **Tkinter**: criação das interfaces gráficas (cadastro e controle).
- **MongoDB**: banco de dados NoSQL para armazenar os remédios, horários e estoques.
- **Paho MQTT (Python)**: comunicação entre o sistema e o dispositivo embarcado.
- **Python Telegram Bot API**: envio de alertas automáticos via Telegram.

### 🤖 2) Firmware ESP32 (Controle Remoto de Horários)
- **Linguagem**: C++ com Arduino Framework.
- **ESP32**: microcontrolador principal que recebe configurações, verifica horários e dispara comandos via Serial.

### 📱 3. Módulo Local (Tela Touch + Motor)
- **Linguagem**: C++
- **Arduino Mega 2560** (ou similar)
- **Tela LCD Touch 2.4” (MCUFRIEND)**
- **Motor de passo 28BYJ-48** com driver **ULN2003**
- **Sensor IR** para detectar se o remédio foi retirado
- **EEPROM interna**
- **Sensor fim de curso (limit switch)** *(opcional para calibrar zero)*

