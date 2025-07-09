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

### 1) **Interface de cadastro**

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

### 2)

---

## 🧰 Tecnologias Utilizadas

- **Python** (lógica principal e integração)
- **Tkinter** (interface gráfica)
- **MongoDB** (banco de dados)
- **Python Telegram Bot API** (mensagens automáticas)
- **OpenPyXL / Pandas** (geração e edição da planilha Excel)
- **MQTT (Paho Client)** (comunicação entre módulos)
- **Hardware/Protótipo físico** (motor para liberação dos remédios - opcional)


