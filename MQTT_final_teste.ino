#include <WiFi.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include "certificados.h"

unsigned long instanteAnteriorHora = 0;

String nome_remedios[5];     // Vetor com nomes dos remédios
String hora_remedios[5][3];  // Vetor com a hora dos remédios
bool diaSemana[5][7] = { { false, false, false, false, false, false, false }, { false, false, false, false, false, false, false }, { false, false, false, false, false, false, false }, { false, false, false, false, false, false, false }, { false, false, false, false, false, false, false } };

bool lembreteAgoraEnviado[5][3] = { { false, false, false }, { false, false, false }, { false, false, false }, { false, false, false }, { false, false, false } };


// --- MQTT Broker ---  endereço do broker MQTT que recebe as mensagens
WiFiClientSecure conexaoSegura;
MQTTClient mqtt(50000);  // Tamanho max da mensagem (1000 bytes)

const char* nome = "Projeto";         // Wifi do lab
const char* password = "2022-11-07";  // Senha do Wifi do lab


void reconectarMQTT() {  // Função para conectar o MQTT
  if (!mqtt.connected()) {
    Serial.print("Conectando MQTT...");
    while (!mqtt.connected()) {
      mqtt.connect("jvcasc", "aula", "zowmad-tavQez");
      Serial.print("...");
      delay(1000);
    }
    Serial.println("Conectado");
    mqtt.subscribe("cadastro_remedio");  // Inscrição no topico de cadastro do remédio
    mqtt.subscribe("manda_um");          // Inscrição no topico de cadastro do remédio

    mqtt.subscribe("reabastecimento_remedio");  // Inscrição no topico de reabastecimento do remédio
  }
}

int horaParaMinutos(String horario) {  // Converte o horário "00:00" para minutos depois da meia noite
  int horas = horario.substring(0, 2).toInt();
  int minutos = horario.substring(3, 5).toInt();

  return horas * 60 + minutos;
}

// Função que converte os dias da semana em uma string binária
String diasParaBinario(JsonArray dias, int indice) {
  String diasDaSemana[7] = { "Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado" };
  String binario = "0000000";  // Inicializa com 7 bits 0

  // Para cada dia fornecido, coloca 1 no índice correspondente
  for (int i = 0; i < dias.size(); i++) {
    String dia = dias[i].as<String>();  // Pega o valor do JsonArray como String
    for (int j = 0; j < 7; j++) {
      if (dia == diasDaSemana[j]) {
        binario[j] = '1';  // Coloca 1 no índice correspondente
        diaSemana[indice][j] = true;
        break;
      }
    }
  }

  return binario;
}

void recebeuMensagem(String topico, String conteudo) {  // Toda vez que receber uma mensagem no servidor MQTT...
  if (topico == "cadastro_remedio") {                   // Toda vez que receber mensagem no topico cadastro_remedio...
    Serial1.println("RESET");
    JsonDocument dados_remedio;
    deserializeJson(dados_remedio, conteudo);  // Função padrão para passar o Json Recebido para uma forma que de para manipular ele

    JsonArray remedios = dados_remedio.as<JsonArray>();

    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 7; j++) {
        diaSemana[i][j] = false;
      }
    }

    for (int i = 0; i < remedios.size(); i++) {
      nome_remedios[i] = "";
      for (int j = 0; j < 3; j++) {
        hora_remedios[i][j] = "";
      }
    }





    for (int i = 0; i < remedios.size(); i++) {
      JsonObject remedio = remedios[i].as<JsonObject>();
      int slot_remedio_original = remedio["compartimento"].as<int>();
      int slot_remedio = slot_remedio_original - 1;

      nome_remedios[slot_remedio] = remedio["remedio"].as<String>();
      JsonArray horarios = remedio["horarios"].as<JsonArray>();
      JsonArray dias_da_semana = remedio["dias"].as<JsonArray>();
      for (int j = 0; j < 3; j++) {                                 // j < horarios.size() ?
        hora_remedios[slot_remedio][j] = horarios[j].as<String>();  // Armazena os horários dos remédios dependendo da quantidade por dia ( Limitado a 3 doses por dia do mesmo remedio )
      }

      String dia_semana_convertido = diasParaBinario(dias_da_semana, slot_remedio);  // Converte o dia da semana para formato "011000"

      if (horarios.size() == 1) {
        int horario_minutos = horaParaMinutos(hora_remedios[slot_remedio][0]);                                                                 // Passa a hora para minutos
        Serial1.println("ADD: " + nome_remedios[slot_remedio] + "," + horarios.size() + "," + horario_minutos + "," + dia_semana_convertido);  // Formato a ser enviado na Serial
        Serial.println("ADD: " + nome_remedios[slot_remedio] + "," + horarios.size() + "," + horario_minutos + "," + dia_semana_convertido);   // Formato a ser enviado na Serial
        delay(800);

      } else if (horarios.size() == 2) {
        int horario_minutos = horaParaMinutos(hora_remedios[slot_remedio][0]);
        int horario_minutos2 = horaParaMinutos(hora_remedios[slot_remedio][1]);
        Serial1.println("ADD: " + nome_remedios[slot_remedio] + "," + horarios.size() + "," + horario_minutos + "," + horario_minutos2 + "," + dia_semana_convertido);  // Formato a ser enviado na Serial
        Serial.println("ADD: " + nome_remedios[slot_remedio] + "," + horarios.size() + "," + horario_minutos + "," + horario_minutos2 + "," + dia_semana_convertido);   // Formato a ser enviado na Serial
        delay(800);

      } else if (horarios.size() == 3) {
        int horario_minutos = horaParaMinutos(hora_remedios[slot_remedio][0]);
        int horario_minutos2 = horaParaMinutos(hora_remedios[slot_remedio][1]);
        int horario_minutos3 = horaParaMinutos(hora_remedios[slot_remedio][2]);
        Serial1.println("ADD: " + nome_remedios[slot_remedio] + "," + horarios.size() + "," + horario_minutos + "," + horario_minutos2 + "," + horario_minutos3 + "," + dia_semana_convertido);  // Formato a ser enviado na Serial
        Serial.println("ADD: " + nome_remedios[slot_remedio] + "," + horarios.size() + "," + horario_minutos + "," + horario_minutos2 + "," + horario_minutos3 + "," + dia_semana_convertido);   // Formato a ser enviado na Serial
        delay(800);
      }
    }
  }

  if (topico == "manda_um") {
    Serial1.println("zero");
  }
}

// --- Setup Wi-Fi --- Conecta no wifi
void setup_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(nome, password);
    Serial.print("Conectado ao wifi...");
    while (WiFi.status() != WL_CONNECTED) {  // Loop que trava quando nao ta conectado na internet
      Serial.print(".");
      delay(1000);
    }
    Serial.println("Conectado!");
  }
}

// Formata o horário para hora / minuto / segundo
String getFormattedTime() {
  time_t now;  // time_t é um tipo padrão para funções de data e hora
  struct tm timeinfo;

  time(&now);                    // Pega a hora atual do sistema e armazena em time
  localtime_r(&now, &timeinfo);  // Deixa tudo formatado em hora , minuto e segundo quando passa para a timeinfo

  char buffer[10];                                       // "HH/MM/SS" Vetor com tamanho 10
  strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);  // Formata do jeito certo

  return String(buffer);
}

int getFormattedTimeWeek() {
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);

  return timeinfo.tm_wday;  // Retorna um valor entre 0 (domingo) e 6 (sábado)
}


void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 18, 17);

  delay(500);
  setup_wifi();  // Conecta no wifi
  conexaoSegura.setCACert(certificado1);
  configTzTime("<-03>3", "a.ntp.br", "pool.ntp.org");  // UTC-3 Brasil Ele acerta o horário do sistema pegando desse site
  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(recebeuMensagem);
  mqtt.setKeepAlive(10);
  mqtt.setWill("off", "3000");

  reconectarMQTT();
}

void loop() {
  setup_wifi();
  reconectarMQTT();
  mqtt.loop();

  String horario = getFormattedTime();
  int diaSemanaAtual = getFormattedTimeWeek();

  if (millis() > instanteAnteriorHora + 10000) {  // A cada 10 segundos
    Serial.println(horario);                      // Imprime a hora atual para verificação

    // Para cada remédio
    for (int i = 0; i < 5; i++) {
      if (diaSemana[i][diaSemanaAtual] == true) {
        for (int j = 0; j < 3; j++) {
          // Se o horário atual é o mesmo e o lembrete ainda não foi enviado
          if (horario == hora_remedios[i][j] && !lembreteAgoraEnviado[i][j]) {
            Serial1.println(i);
            Serial.println(i);

            mqtt.publish("hora_remedio", "Hora do " + nome_remedios[i]);
            lembreteAgoraEnviado[i][j] = true;  // Marca que o lembrete foi enviado
          }
          // Se o horário atual não corresponder ao horário armazenado
          else if (horario != hora_remedios[i][j] && lembreteAgoraEnviado[i][j]) {
            lembreteAgoraEnviado[i][j] = false;  // Reseta a flag apenas para o horário específico
          }
        }
      }


      // Para cada horário do remédio
    }

    instanteAnteriorHora = millis();  // Atualiza o tempo para a próxima verificação
  }
}