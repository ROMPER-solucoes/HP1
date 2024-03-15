#include <Wire.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3);
bool lcdCleared = false; // Flag para indicar se o LCD foi limpo
void receiveEvent(int bytes); // Protótipo da função receiveEvent

void setup() {
  pinMode(25, OUTPUT); // Configura o pino 25 como saída
  pinMode(14, INPUT_PULLUP); // Configura o pino 14 como entrada com resistor de pull-up
  pinMode(15, INPUT_PULLUP); // Configura o pino 15 como entrada com resistor de pull-up
  pinMode(16, INPUT_PULLUP); // Configura o pino 16 como entrada com resistor de pull-up
  
  lcd.begin(20, 4); // Inicializa o LCD com 20 colunas e 4 linhas
  Wire.begin(8); // Endereço deste Transeptor (Transceptor 2) na rede I2C
  Wire.onReceive(receiveEvent); // Configura a função para lidar com a recepção de dados I2C
  Wire.beginTransmission(9); // Endereço do Transeptor 1 na rede I2C
}

void loop() {
  // Aqui você pode adicionar qualquer lógica adicional que precise ser executada continuamente
  // Verifica o estado dos pinos 14, 15 e 16 e envia os comandos correspondentes se estiverem LOW
  if (digitalRead(14) == LOW) {
    sendCommand('E'); // Envia o comando "E" para o t ransmissor
  }
  if (digitalRead(15) == LOW) {
    sendCommand('P'); // Envia o comando "P" para o transmissor
  }
  if (digitalRead(16) == LOW) {
    sendCommand('A'); // Envia o comando "A" para o transmissor
  }  
}

void sendCommand(char command) {
  Wire.beginTransmission(9); // Endereço do Transeptor 1 
  Wire.write('c'); // Indica que os dados seguintes são comandos
  Wire.write(command); // Envia o comando pela I2C
  Wire.endTransmission();
  delay(10); // Pequeno atraso para garantir a transmissão correta
}   


void receiveEvent(int bytes) {
  while (Wire.available()) { // Enquanto houver dados disponíveis para leitura
    char dataType = Wire.read(); // Lê o tipo de dado (comando ou texto)
    if (dataType == 't') { // Se o tipo de dado for texto
      byte col = Wire.read(); // Lê a coluna
      byte row = Wire.read(); // Lê a linha
      byte length = Wire.read(); // Lê o comprimento do texto
      char text[length + 1]; // Cria um array para armazenar o texto
      for (int i = 0; i < length; i++) { // Lê o texto caractere por caractere
        text[i] = Wire.read();
      }
      text[length] = '\0'; // Adiciona o caractere nulo ao final do texto
      if (!lcdCleared) {
        lcd.clear(); // Limpa o LCD apenas se ainda não estiver limpo
        lcdCleared = true; // Define a flag para indicar que o LCD foi limpo
      }
      lcd.setCursor(col, row); // Define o cursor do LCD para a posição especificada
      lcd.print(text); // Imprime o texto no LCD
    } else if (dataType == 'c') { // Se o tipo de dado for comando
      char command = Wire.read(); // Lê o comando
      if (command == 'l') { // Se for o comando para limpar o LCD
        lcd.clear(); // Limpa o LCD
        lcdCleared = true; // Define a flag para indicar que o LCD foi limpo
      } else if (command == '*') { // Se for o comando para ativar o pino 25
        digitalWrite(25, HIGH); // Define o pino 25 como HIGH
      }
    }
  } 
}
