# include <Adafruit_GFX.h>
# include <MCUFRIEND_kbv.h>
# include <GFButton.h>
# include <JKSButton.h>
# include <EEPROM.h>
# include <unistd.h>
# include <Stepper.h>

# define STEPS_PER_REV 2048
# define MAX_REMEDIOS 5
# define MAX_HORARIOS 3  //máximo de 3 horários por remédio
Stepper motor(STEPS_PER_REV, 50, 46, 48, 44);
MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);
const int ONE_FIFTH = 410;
const int GRAB_OFFSET = 210;
GFButton PIN_LIMIT_SWITCH(28);
long posicaoAtual = 0;
bool zeroDefinido = false;
const int TS_LEFT = 145, TS_RT = 887, TS_TOP = 934, TS_BOT = 158;
struct Remedio {
  char nome[20];
  //int dosagem;
  int horarios[MAX_HORARIOS];  //em minutos desde 00:00
  int numHorarios;
  bool diasSemana[7];
};
Remedio remedios[MAX_REMEDIOS];
int totalRemedios = 0;
JKSButton botoes[MAX_REMEDIOS];
JKSButton voltar;
String estado = "menu";
const char* dias[7] = { "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado", "Domingo" };
int pinoSensor = 52;  // OUT do sensor conectado ao pino digital 50

// ========================= MOTOR ============================
void andaUmQuinto() {
  motor.step(ONE_FIFTH);
  posicaoAtual += ONE_FIFTH;
}
void pegaComprimido() {
  motor.step(-GRAB_OFFSET);
  delay(300);
  motor.step(STEPS_PER_REV);
  posicaoAtual += STEPS_PER_REV - GRAB_OFFSET;
  int passosRestantes = STEPS_PER_REV - (posicaoAtual % STEPS_PER_REV);
  motor.step(passosRestantes);
  posicaoAtual = 0;
  Serial.println(F("Comprimido liberado. Girando mais uma volta para esperar redefinir zero..."));
  motor.step(STEPS_PER_REV);
  //Serial.println(F("Aguardando comando SERIAL para definir novo zero..."));
  //zeroDefinido = false;
}
void selecionaComprimido(int tubo) {
  for (int i = 0; i < tubo; i++) andaUmQuinto();
  pegaComprimido();
  tomarRemedio();
}
// ========================= EEPROM ============================
void salvarEEPROM() {
  // Salva quantos remédios existem (totalRemedios) + dados de cada remédio (remedios[i])
  EEPROM.put(0, totalRemedios);  //grava o número total de remédios (um int) no endereço 0 da EEPROM
  int endereco = sizeof(int);    //posição onde os dados dos remédios vão começar. como totalRemedios já ocupou sizeof(int), o próximo espaço disponível é ali.
  for (int i = 0; i < totalRemedios; i++) {
    EEPROM.put(endereco, remedios[i]);
    endereco += sizeof(Remedio);
  }
}
void carregarEEPROM() {
  // Ler da EEPROM todos os remédios que foram salvos anteriormente
  EEPROM.get(0, totalRemedios);
  if (totalRemedios < 0) {
    totalRemedios = 0;  //corrige o totalRemedios se estiver inválido
  }
  if (totalRemedios > MAX_REMEDIOS) totalRemedios = MAX_REMEDIOS;  //garante que totalRemedios nunca ultrapasse 5
  int endereco = sizeof(int);
  for (int i = 0; i < totalRemedios; i++) {
    EEPROM.get(endereco, remedios[i]);
    endereco += sizeof(Remedio);
  }
}
// ========== CONVERSÃO ==========
String minutosParaHorario(int minutos) {
  int h = minutos / 60;
  int m = minutos % 60;
  char buffer[6];                      //cria um array de caracteres com espaço para 5 caracteres + \0
  sprintf(buffer, "%02d:%02d", h, m);  //formata os inteiros h e m (horas e minutos) no estilo hh:mm
  return String(buffer);
}
// ========== INTERFACE ==========
void mostrarRemedio(int index) {
  estado = "detalhes";
  Remedio r = remedios[index];
  for (int i = 0; i < MAX_REMEDIOS; i++) {
  botoes[i].init(&tela, &touch, 0, 0, 0, 0, 0, 0, 0, "", 0);  // desativa
  botoes[i].setPressHandler(nullptr);
}
  tela.fillScreen(TFT_BLACK);
  tela.setCursor(10, 15);
  tela.setTextColor(TFT_RED);
  tela.setTextSize(2);
  tela.print(r.nome);
/*
  tela.setCursor(10, 47);
  tela.setTextColor(TFT_WHITE);
  tela.print("Dosagem: ");
  tela.setCursor(155, 47);
  tela.print(r.dosagem);
*/
  // horários
  tela.setTextColor(TFT_WHITE);
  tela.setCursor(10, 45);
  tela.setTextSize(2);
  tela.print("Horarios:");
  int comp = 10;
  for (int i = 0; i < r.numHorarios; i++) {
    tela.setCursor(comp, 70);
    tela.print(minutosParaHorario(r.horarios[i]));
    comp += 85;
  }
  int altura = 110;
  for (int i = 0; i < 7; i++) {
    tela.setCursor(10, altura);
    tela.print(dias[i]);
    if (r.diasSemana[i]) {
      int x = 150;
      int y = altura + 5;
      tela.drawLine(x, y, x + 5, y + 5, TFT_GREEN);
      tela.drawLine(x + 5, y + 5, x + 12, y - 5, TFT_GREEN);
    }
    altura += 20;
  }
  voltar.init(&tela, &touch, 120, 290, 100, 30, TFT_WHITE, TFT_RED, TFT_WHITE, "Voltar", 2);
  voltar.setPressHandler(volta);
}
void telaInicial() {
  Serial.println(">>> Chamou tela()");
  estado = "menu";
  tela.fillScreen(TFT_BLACK);
  for (int i = 0; i < totalRemedios; i++) {
    botoes[i].init(&tela, &touch, 120, 53 + i * 53, 160, 25, TFT_BLACK, TFT_BLACK, TFT_WHITE, remedios[i].nome, 2);
  }
  botoes[0].setPressHandler([]() {
    mostrarRemedio(0);
  });
  botoes[1].setPressHandler([]() {
    mostrarRemedio(1);
  });
  botoes[2].setPressHandler([]() {
    mostrarRemedio(2);
  });
  botoes[3].setPressHandler([]() {
    mostrarRemedio(3);
  });
  botoes[4].setPressHandler([]() {
    mostrarRemedio(4);
  });
  for (int i = totalRemedios; i < MAX_REMEDIOS; i++) {          //erro de botões não sumirem quando add + de 5
    botoes[i].init(&tela, &touch, 0, 0, 0, 0, 0, 0, 0, "", 0);  //apaga
    botoes[i].setPressHandler(nullptr);
  }
}
void volta() {
  Serial.println(">>> Chamou voltar()");
  carregarEEPROM();  // recarrega totalRemedios e os remedios da EEPROM
  estado = "menu";       // <- garante que voltamos pro menu
  telaInicial();
}
void tomarRemedio() {
  Serial.println(">>> Chamou tomar()");
  tela.fillScreen(TFT_BLACK);
  tela.setCursor(30, 80);
  tela.setTextColor(TFT_RED);
  tela.setTextSize(4);
  tela.print("Remedio");
  tela.setCursor(15, 125);
  tela.setTextSize(4);
  tela.print("liberado:");
  tela.setCursor(15, 180);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print("tomar agora!");
  /*
  delay(90000);  // espera 90.000 ms = 1min30s
  telaInicial(); */
 
  estado = "tomar";
  voltar.init(&tela, &touch, 120, 290, 100, 30, TFT_WHITE, TFT_RED, TFT_WHITE, "Voltar", 2);
  voltar.setPressHandler(volta);
}
// ========== COMANDOS ==========
void reset() {
  totalRemedios = 0;
  EEPROM.write(0, totalRemedios);
  Serial.println("Reset feito.");
}
void renderizar() {
  carregarEEPROM();
  /*
  Serial.print("Total carregado: ");
  Serial.println(totalRemedios);
  for (int i = 0; i < totalRemedios; i++) {
    Serial.print("Remedio ");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(remedios[i].nome);
    Serial.print(" / qtd: ");
    Serial.print(remedios[i].quantidade);
    Serial.print(" / horarios: ");
    for (int j = 0; j < remedios[i].numHorarios; j++) {
      Serial.print(minutosParaHorario(remedios[i].horarios[j]));
      Serial.print(" ");
    }
    Serial.println();
  }
*/
  telaInicial();
}
// =================== SERIAL =====================
/* ADD:NomeRemedio,Dosagem,NumHorarios,h1,h2,h3,...,dias
ADD:Paracetamol,500,2,480,1320,1111111 */
void lerSerial() {
  if (Serial1.available()) {
  String texto = Serial1.readStringUntil('\n');
  texto.trim();  // remove \r ou espaços no final
  Serial.println("Recebido: " + texto);
  /*
  if (Serial1.available() > 0) {
    String texto = Serial1.readString();
    Serial.println(texto);
    texto.trim();
    */
    if (texto.length() == 1 && zeroDefinido) {
      char cmd = texto.charAt(0);
      if (cmd >= '0' && cmd <= '4') {
        int tubo = cmd - '0';
        Serial.print(F("Selecionando tubo "));
        Serial.println(tubo);
        selecionaComprimido(tubo);
        Serial.println(F("Feito!\n"));
      }
    }
    if (texto.startsWith("ADD:")) {
      if (totalRemedios >= MAX_REMEDIOS) {
        return;
      }
      /*
      if (totalRemedios >= MAX_REMEDIOS) {
          // Desloca todos para a esquerda (remove o primeiro)
        for (int i = 1; i < MAX_REMEDIOS; i++) {
          remedios[i - 1] = remedios[i];
        }
        totalRemedios = MAX_REMEDIOS - 1;  // Vamos inserir o novo agora
      }
      */
      Remedio r;
      texto.remove(0, 4);                    // remove "ADD:"
      int p1 = texto.indexOf(',');           //retorna a posição do primeiro caractere ',' encontrado na string texto
      String nome = texto.substring(0, p1);  //0 até posição de ',' (1a palavra)
      nome.toCharArray(r.nome, 20);
      texto.remove(0, p1 + 1);  //remove
/*
      int p2 = texto.indexOf(',');
      r.dosagem = texto.substring(0, p2).toInt();
      texto.remove(0, p2 + 1);
*/
    int p2 = texto.indexOf(',');
    r.numHorarios = texto.substring(0, p2).toInt();
      texto.remove(0, p2 + 1);
     /*
      int p3 = texto.indexOf(',');
      r.numHorarios = texto.substring(0, p3).toInt();
      texto.remove(0, p3 + 1);
*/
      for (int i = 0; i < MAX_HORARIOS; i++) {
        r.horarios[i] = 0;  //zera os valores
      }
      for (int i = 0; i < r.numHorarios; i++) {
        int p = texto.indexOf(',');
        r.horarios[i] = texto.substring(0, p).toInt();
        texto.remove(0, p + 1);
      }
      for (int i = 0; i < 7; i++) {
        r.diasSemana[i] = (texto.charAt(i) == '1');  //se o caracter for igual a 1, true, se não false
      }
      remedios[totalRemedios++] = r;  //adiciona o novo remédio no vetor. aumenta o contador totalRemedios.
      Serial.println("Salvando na EEPROM...");
      salvarEEPROM();
      Serial.println("Salvo!");
      renderizar();
    }
    if (texto == "RESET") {
      reset();
    }
    //else if (texto == "OK") {
      //Serial.println(">> Chamando renderizar()");
      //renderizar();
    
   
    //else if (texto == "tomar") {
      //tomarRemedio();
  //}
    else if (texto == "zero") {
      define_zero();
    }
  }
}
void define_zero() {
  posicaoAtual = 0;
  zeroDefinido = true;
  motor.setSpeed(15);
  Serial.println(F(">> POSIÇÃO ZERO definida via Serial!"));
}
// ================ SETUP e LOOP =====================
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  tela.begin(tela.readID());
  tela.fillScreen(TFT_BLACK);
  pinMode(pinoSensor, INPUT);
  pinMode(47, OUTPUT);
  digitalWrite(47, LOW); //Terra do sensor
  //limparEEPROM();
  motor.setSpeed(10);
  //PIN_LIMIT_SWITCH.setReleaseHandler(define_zero);
  //pinMode(PIN_LIMIT_SWITCH, INPUT_PULLUP);  // leitura com resistor interno de pull-up
  Serial.println(F("\n== Inicialização =="));
  Serial.println(F("Motor girando até acionar fim de curso..."));
  renderizar();  // <== chama para desenhar menu na tela assim que inicia
}
void loop() {
  PIN_LIMIT_SWITCH.process();
  lerSerial();
  //botoes[0].process();
  if (estado == "menu") {
    for (int i = 0; i < totalRemedios; i++) botoes[i].process();
  }
  else if (estado == "detalhes" || estado == "tomar") {
  voltar.process();
  }
  if(!zeroDefinido){
  motor.step(1);  // continua girando
  posicaoAtual++;
  delay(2);
  }
  if (estado == "tomar") {
  int leitura = digitalRead(pinoSensor); // 0 ou 1
  //Serial.println(leitura);

  if (leitura ==  LOW) {
      renderizar();
    } 
  }
}
/*
PASSO 1 – Carregar dados antigos
PASSO 2 – Exibir menu principal
PASSO 3 – Esperar entrada Serial (vinda do computador)   //carregarEEPROM
PASSO 4 – Salvar novo remédio na EEPROM
*/