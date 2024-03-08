const int numPinosAnalogicos = 8;
const int pinosAnalogicos[] = {A0, A1, A2, A3, A4, A5, A6, A7};
const int pinosDigitaisEntrada[] = {47, 48, 49};
const int pinosDigitaisSaida[] = {31, 33, 35, 37, 39, 41, 43, 45};
const int pinoPWMEntradaCaudal = 3;

bool alarmeA16Ativado = false;
unsigned long tempoInicioAlarmeA16 = 0;

void setup() {
Serial.begin(9600);

// Configura os pinos digitais de entrada como entradas digitais flutuantes
for (int i = 0; i < sizeof(pinosDigitaisEntrada) / sizeof(pinosDigitaisEntrada[0]); i++) {
pinMode(pinosDigitaisEntrada[i], INPUT_PULLUP);
}

for (int i = 0; i < sizeof(pinosDigitaisSaida) / sizeof(pinosDigitaisSaida[0]); i++) {
pinMode(pinosDigitaisSaida[i], OUTPUT);
}

pinMode(pinoPWMEntradaCaudal, INPUT);
}

void loop() {
int valorPWMCaudal = pulseIn(pinoPWMEntradaCaudal, HIGH);
int estadoPino49 = digitalRead(49);

Serial.print("Valor PWM Caudal: ");
Serial.println(valorPWMCaudal);

// Proteção por falta de caudal
if (valorPWMCaudal < -1 || valorPWMCaudal > 5) {
Serial.println("A15");
desligarSaidas();
aguardarCaudalNormal();
}

int estadoPino48 = digitalRead(48);

if (estadoPino48 != LOW) {
Serial.println("A14");
desligarSaidas();
aguardarPino48Low();
}

int valorA7 = analogRead(A7);

if (valorA7 < 60 || valorA7 > 600) {
Serial.println("A11");
desligarSaidas();
aguardarA7Normal();
} else {
for (int i = 0; i < numPinosAnalogicos; i++) {
int valor = analogRead(pinosAnalogicos[i]);

  Serial.print("Pino ");
  Serial.print(i);
  Serial.print(" (Analogico): ");
  Serial.println(valor);

  delay(1000);
}

}

// Verifica se nenhum alarme foi detectado e configura o pino 31 para HIGH
int valorA5 = analogRead(A5);
int valorA6 = analogRead(A6);

if (valorPWMCaudal >= -1 && valorPWMCaudal <= 5 && estadoPino48 == LOW && (valorA7 >= 60 && valorA7 <= 600) && (valorA5 >= 60 && valorA5 <= 600) && (valorA6 >= 60 && valorA6 <= 600)) {
digitalWrite(31, HIGH);
// Se pino 31 está em HIGH, verifica o pino 49 após 10 segundos
if (millis() - tempoInicioAlarmeA16 > 10000 && estadoPino49 != LOW && !alarmeA16Ativado) {
Serial.println("A16");
desligarSaidas();
alarmeA16Ativado = true;
// Desativa a leitura de outros pinos
while (true) {
delay(100);  // Aguarda até que o reset seja dado
}
}
} else {
digitalWrite(31, LOW);
tempoInicioAlarmeA16 = millis();  // Reinicia a contagem do tempo ao desligar o pino 31
alarmeA16Ativado = false;

// Adiciona a verificação e ação para A5
if (valorA5 < 60 || valorA5 > 600) {
  Serial.println("A12");
  desligarSaidas();
  aguardarA5Normal();
}

// Adiciona a verificação e ação para A6
if (valorA6 < 60 || valorA6 > 600) {
  Serial.println("A13");
  desligarSaidas();
  aguardarA6Normal();
}

}
}

void desligarSaidas() {
// Define todas as saídas para LOW
for (int i = 0; i < sizeof(pinosDigitaisSaida) / sizeof(pinosDigitaisSaida[0]); i++) {
digitalWrite(pinosDigitaisSaida[i], LOW);
}
}

void aguardarCaudalNormal() {
// Aguarda até que o valor de caudal esteja dentro do intervalo
while (pulseIn(pinoPWMEntradaCaudal, HIGH) < -1 || pulseIn(pinoPWMEntradaCaudal, HIGH) > 5) {
delay(100); // Ajuste conforme necessário
}
}

void aguardarPino48Low() {
// Aguarda até que o pino 48 esteja em LOW
while (digitalRead(48) != LOW) {
delay(100);
}
}

void aguardarA7Normal() {
// Aguarda até que o valor de A7 esteja dentro do intervalo
while (analogRead(A7) < 60 || analogRead(A7) > 600) {
delay(100);
}
}

void aguardarA5Normal() {
// Aguarda até que o valor de A5 esteja dentro do intervalo
while (analogRead(A5) < 60 || analogRead(A5) > 600) {
delay(100);
}
}

void aguardarA6Normal() {
// Aguarda até que o valor de A6 esteja dentro do intervalo
while (analogRead(A6) < 60 || analogRead(A6) > 600) {
delay(100);
}
