#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

// Define os pinos dos sensores de temperatura
#define TEMP_COMPRESS_ENTRADA A0
#define TEMP_COMPRESS_SAIDA A1
#define TEMP_PERMUT_ENTRADA A2
#define TEMP_PERMUT_SAIDA A3
#define TEMP_VALVEX_ENTRADA A4
#define TEMP_EVAP_ENTRADA A5
#define TEMP_RUA A6
#define TEMP_AC_SAIDA A7
// Define os pinos do motor de passo da válvula de expansão
#define VALVEX_PIN1 4
#define VALVEX_PIN2 5
#define VALVEX_PIN3 6
#define VALVEX_PIN4 7

// Define os pinos do inversor de frequência do compressor
#define INV_COMPRESS_PIN 8

// Define os parâmetros da bomba de calor
#define TEMP_COMPRESS_MAX 90 // Temperatura máxima do circuito de alta (°C)
#define TEMP_COMPRESS_MIN 70 // Temperatura mínima do circuito de alta (°C)
#define FREQ_INV_COMPRESS_MAX 120 // Frequência máxima do compressor (Hz)
#define FREQ_INV_COMPRESS_MIN 0 // Frequência mínima do compressor (Hz)
#define PASSOS_VALVEX 200 // Número de passos do motor de passo da válvula
#define PASSO_VALVEX 1.8 // Ângulo de cada passo do motor de passo da válvula (°)

// Define os pinos dos botões
#define ANTERIOR_PIN 25
#define PROXIMO_PIN 23
#define ENTER_PIN 27

// Cria os objetos das classes LiquidCrystal_I2C, Stepper e Servo
LiquidCrystal_I2C lcd1(0x27, 20, 4);
Stepper valvex(200, VALVEX_PIN1, VALVEX_PIN2, VALVEX_PIN3, VALVEX_PIN4);

// Declara as variáveis globais
float temp_compress_entrada;
float temp_compress_saida;
float temp_permut_entrada;
float temp_permut_saida;
float temp_valvex_entrada;
float temp_evap_entrada;
float temp_rua;
float temp_ac_saida;
float freq_compress;
int angulo_valvex;
int displayMode = 0;
int setUpMenuMode = 0;  // Modo do menu "SET-UP"
unsigned long enterButtonPressTime = 0;
bool inSetUpMenu = false;
int lastButtonState25 = HIGH;
int lastButtonState23 = HIGH;
int lastButtonState27 = HIGH;
int setPointValue = false; // Valor inicial do SET POINT (pode ser ajustado conforme necessário)

void showSetUpMenu() {
  lcd1.clear();
  lcd1.setCursor(0, 0);
  switch (setUpMenuMode) {
    case 0:
      lcd1.print("SET-POINT");
      break;
    case 1:
      lcd1.print("DEFROST");
      break;
    case 2:
      lcd1.print("DATA e HORA");
      break;
    case 3:
      lcd1.print("LOG de ERROS");
      break;
  }
}

void setup() {
  // Inicializa o LCD
  lcd1.begin();
  lcd1.backlight();

  // Inicializa o motor de passo da válvula
  valvex.setSpeed(60);
  angulo_valvex = 0;

  // Inicializa o inversor de frequência do compressor
  pinMode(INV_COMPRESS_PIN, OUTPUT); // Define o pino como saída
  freq_compress = FREQ_INV_COMPRESS_MIN; // Inicializa a frequência do compressor como mínima

  // Configuração do pino para o botão "Enter"
  pinMode(ENTER_PIN, INPUT_PULLUP);

  // Mostra uma mensagem inicial no LCD
  lcd1.setCursor(0, 0);
  lcd1.print("Heat Pump");
  lcd1.setCursor(0, 1);
  lcd1.print("EnerPlural");
  delay(2000);
  lcd1.setCursor(0, 2);
  delay(2000);
  lcd1.print("Loading...");
  delay(2000);
}

void loop() {
  // Lê as temperaturas dos sensores e converte para °C
  temp_compress_entrada = 1024 - (log((analogRead(TEMP_COMPRESS_ENTRADA)) + 360) / log(1024 + 1)) * 1024;
  temp_compress_saida = 1024 - (log((analogRead(TEMP_COMPRESS_SAIDA)) + 360) / log(1024 + 1)) * 1024;
  temp_permut_entrada = 1024 - (log((analogRead(TEMP_PERMUT_ENTRADA)) + 360) / log(1024 + 1)) * 1024;
  temp_permut_saida = 1024 - (log((analogRead(TEMP_PERMUT_SAIDA)) + 360) / log(1024 + 1)) * 1024;
  temp_valvex_entrada = 1024 - (log((analogRead(TEMP_VALVEX_ENTRADA)) + 360) / log(1024 + 1)) * 1024;
  temp_evap_entrada = 1024 - (log((analogRead(TEMP_EVAP_ENTRADA)) + 360) / log(1024 + 1)) * 1024;
  temp_rua = 1024 - (log((analogRead(TEMP_RUA)) + 360) / log(1024 + 1)) * 1024;
  temp_ac_saida = 1024 - (log((analogRead(TEMP_AC_SAIDA)) + 360) / log(1024 + 1)) * 1024;

  // Controla a válvula de expansão para manter a temperatura de saída do circuito de alta entre os limites definidos
  if (temp_compress_saida > TEMP_COMPRESS_MAX) {
    // Fecha a válvula um pouco para reduzir a vazão do gás e diminuir a temperatura
    angulo_valvex -= PASSO_VALVEX;
    if (angulo_valvex < -90) angulo_valvex = -90; // Limita o ângulo mínimo da válvula em -90°
    valvex.step(-PASSOS_VALVEX / (360 / PASSO_VALVEX)); // Move o motor de passo na direção anti-horária um passo
  }

  if (temp_compress_saida < TEMP_COMPRESS_MIN) {
    // Abre a válvula um pouco para aumentar a vazão do gás e aumentar a temperatura
    angulo_valvex += PASSO_VALVEX;
    if (angulo_valvex > 90) angulo_valvex = 90; // Limita o ângulo máximo da válvula em +90°
    valvex.step(PASSOS_VALVEX / (360 / PASSO_VALVEX)); // Move o motor de passo na direção horária um passo
  }

  // Controla a frequência do compressor para manter a temperatura de entrada do circuito de alta entre os limites definidos
  if (temp_compress_entrada > TEMP_COMPRESS_MAX) {
    // Reduz a frequência do compressor para diminuir a velocidade do gás e diminuir a temperatura
    freq_compress -= FREQ_INV_COMPRESS_MAX / PASSOS_VALVEX;
    if (freq_compress < FREQ_INV_COMPRESS_MIN) freq_compress = FREQ_INV_COMPRESS_MIN; // Limita a frequência mínima do compressor em FREQ_MIN Hz
    analogWrite(INV_COMPRESS_PIN, map(freq_compress, FREQ_INV_COMPRESS_MIN, FREQ_INV_COMPRESS_MAX, 0, 255)); // Envia um sinal PWM proporcional à frequência desejada para o inversor
  }

  if (temp_compress_entrada < TEMP_COMPRESS_MIN) {
    // Aumenta a frequência do compressor para aumentar a velocidade do gás e aumentar a temperatura
    freq_compress += FREQ_INV_COMPRESS_MAX / PASSOS_VALVEX;
    if (freq_compress > FREQ_INV_COMPRESS_MAX) freq_compress = FREQ_INV_COMPRESS_MAX; // Limita a frequência máxima do compressor em FREQ_MAX Hz
    analogWrite(INV_COMPRESS_PIN, map(freq_compress, FREQ_INV_COMPRESS_MIN, FREQ_INV_COMPRESS_MAX, 0, 255)); // Envia um sinal PWM proporcional à frequência desejada para o inversor
  }

  // Verifica o estado do botão "Anterior" e alterna a direção da transição no LCD1
  if (digitalRead(ANTERIOR_PIN) == LOW && lastButtonState25 == HIGH) {
    if (inSetUpMenu) {
      setUpMenuMode = (setUpMenuMode - 1 + 4) % 4;  // Alterna para a opção anterior no menu "SET-UP"
      showSetUpMenu();
    } else {
      displayMode = (displayMode - 1 + 5) % 5;  // Alterna para o modo anterior
    }
  }
  lastButtonState25 = digitalRead(ANTERIOR_PIN);

  // Verifica o estado do botão "Próximo" e alterna a exibição no LCD1
  if (digitalRead(PROXIMO_PIN) == LOW && lastButtonState23 == HIGH) {
    if (inSetUpMenu) {
      setUpMenuMode = (setUpMenuMode + 1) % 4;  // Alterna para a próxima opção no menu "SET-UP"
      showSetUpMenu();
    } else {
      displayMode = (displayMode + 1) % 5;  // Alterna para o próximo modo
    }
  }
  lastButtonState23 = digitalRead(PROXIMO_PIN);

  // Verifica o estado do botão "Enter" e gerencia o menu "SET-UP"
  int buttonState27 = digitalRead(ENTER_PIN);

  if (buttonState27 == LOW && lastButtonState27 == HIGH) {
    // Botão "Enter" foi pressionado
    enterButtonPressTime = millis();  // Registra o tempo atual
  }

  if (buttonState27 == HIGH && lastButtonState27 == LOW) {
    // Botão "Enter" foi solto
    if (millis() - enterButtonPressTime >= 100) {
      // Botão "Enter" foi pressionado por 3 segundos ou mais
      if (!inSetUpMenu) {
        // Se não estiver no menu "SET-UP", entra no menu
        inSetUpMenu = true;
        showSetUpMenu();
      } else {
        // Se estiver no menu "SET-UP", realiza a ação associada à opção selecionada
        switch (setUpMenuMode) {
          case 0:
            // Ação para a opção "SET-POINT"
            // Implemente aqui as ações desejadas para esta opção
            // Neste exemplo, vamos ajustar o SET POINT
            adjustSetPoint();
            break;
          case 1:
            // Ação para a opção "DEFROST"
            // Implemente aqui as ações desejadas para esta opção
            break;
          case 2:
            // Ação para a opção "DATA e HORA"
            // Implemente aqui as ações desejadas para esta opção
            break;
          case 3:
            // Ação para a opção "LOG de ERROS"
            // Implemente aqui as ações desejadas para esta opção
            break;
        }

        // Sai do menu "SET-UP"
        inSetUpMenu = false;
        lcd1.clear();
      }
    }
  }

  // Mostra as temperaturas e a frequência no LCD1 apenas se não estiver no menu "SET-UP"
  if (!inSetUpMenu) {
    lcd1.clear();
    lcd1.setCursor(0, 0);
    switch (displayMode) {
      case 0:
        lcd1.setCursor(0, 0);
        lcd1.print("S1-Temp.Ent.Comp:");
        lcd1.setCursor(0, 1);
        lcd1.print(temp_compress_entrada);
        lcd1.print(" C");
        lcd1.setCursor(0, 2);
        lcd1.print("S2-Temp.Saida.Comp:");
        lcd1.setCursor(0, 3);
        lcd1.print(temp_compress_saida);
        lcd1.print(" C");
        break;
      case 1:
        lcd1.setCursor(0, 0);
        lcd1.print("S3-Temp.Ent.Perm:");
        lcd1.setCursor(0, 1);
        lcd1.print(temp_permut_entrada);
        lcd1.print(" C");
        lcd1.setCursor(0, 2);
        lcd1.print("S4-Temp.Saida.Perm:");
        lcd1.setCursor(0, 3);
        lcd1.print(temp_permut_saida);
        lcd1.print(" C");
        break;
      case 2:
        lcd1.setCursor(0, 0);
        lcd1.print("S5-Temp.Ent.EEV:");
        lcd1.setCursor(0, 1);
        lcd1.print(temp_valvex_entrada);
        lcd1.print(" C");
        lcd1.setCursor(0, 2);
        lcd1.print("S6-Temp.Ent.Evap:");
        lcd1.setCursor(0, 3);
        lcd1.print(temp_evap_entrada);
        lcd1.print(" C");
        break;
      case 3:
        lcd1.setCursor(0, 0);
        lcd1.print("S7-Temp.Ar.Rua:");
        lcd1.setCursor(0, 1);
        lcd1.print(temp_rua);
        lcd1.print(" C");
        lcd1.setCursor(0, 2);
        lcd1.print("S8-Temp.AC.Saida:");
        lcd1.setCursor(0, 3);
        lcd1.print(temp_ac_saida);
        lcd1.print(" C");
        break;
      case 4:
        lcd1.setCursor(0, 0);
        lcd1.print("A1-Freq.Inv.Comp:");
        lcd1.setCursor(0, 1);
        lcd1.print(freq_compress);
        lcd1.print(" Hz");
        lcd1.setCursor(0, 2);
        lcd1.print("A2-Abertura EEV");
        lcd1.setCursor(0, 3);
        lcd1.print(angulo_valvex);
        lcd1.print("'");
        break;
    }
  }

  // Atualiza o estado do botão "Enter"
  lastButtonState27 = buttonState27;
}

// Função para ajustar o SET POINT
void adjustSetPoint() {
  lcd1.clear();
  lcd1.setCursor(1, 0);
  lcd1.print("SET POINT: ");
  lcd1.print(setPointValue);
  lcd1.setCursor(14, 0);
  lcd1.print("C");
  lcd1.setCursor(0, 2);
  lcd1.print("Pressione 'Enter'");
  lcd1.setCursor(0, 3);
  lcd1.print("para confirmar");

  unsigned long enterButtonPressTime = 0;
  bool confirmationPending = false;
  bool exitSetUpMenu = false;

  while (inSetUpMenu && !exitSetUpMenu) {
    int buttonState27 = digitalRead(ENTER_PIN);

    if (buttonState27 == LOW && lastButtonState27 == HIGH) {
      enterButtonPressTime = millis();
      confirmationPending = true;
    }

    if (buttonState27 == HIGH && lastButtonState27 == LOW) {
      if (confirmationPending && (millis() - enterButtonPressTime >= 100)) {
        // Confirmação bem-sucedida
        exitSetUpMenu = true;
        lcd1.clear();
        // Salva o novo SET POINT ou executa outras ações de confirmação aqui
        setPointValue = constrain(setPointValue, 30, 85);  // Limita o SET POINT entre 30 e 85 graus
      }
      confirmationPending = false;
    }

    if (digitalRead(ANTERIOR_PIN) == LOW && lastButtonState25 == HIGH) {
      setPointValue = max(30, setPointValue - 1);  // Garante que o valor não seja inferior a 30
    }
    lastButtonState25 = digitalRead(ANTERIOR_PIN);

    if (digitalRead(PROXIMO_PIN) == LOW && lastButtonState23 == HIGH) {
      setPointValue = min(85, setPointValue + 1);  // Garante que o valor não ultrapasse 85
    }
    lastButtonState23 = digitalRead(PROXIMO_PIN);

    lcd1.setCursor(12, 0);
    lcd1.print(setPointValue);

    // Adicionado código para sair e retornar às leituras dos sensores
    if (digitalRead(ENTER_PIN) == LOW) {
      exitSetUpMenu = true;
      lcd1.clear();
      inSetUpMenu = false;  // Garante que inSetUpMenu seja definido como falso
    }

    delay(50);
  }
}
