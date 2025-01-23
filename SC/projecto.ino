/*-------------------BIBLIOTECAS------------------------------ */
/*--------------------------------------------------------- */
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DHT11.h>
#include <Servo.h>

/*-------------------PINOS------------------------------ */
/*--------------------------------------------------------- */
Servo myservo;
#define SENSOR_NIVEL_PIN 7 // Pino onde está ligado o sensor de nível de água
#define DHT11_PIN 8        // Pino onde está ligado o sensor DHT11 de temperatura e humidade
int buzzer = 6;           // Pino onde está ligado o buzzer (alarme)
int angulo = 0;           // Variável para armazenar o ângulo do servo

DHT11 dht11(DHT11_PIN);              // Instancia o sensor DHT11
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Instancia o display LCD
SoftwareSerial BTSerial(2, 3);        // Instancia a comunicação serial com o módulo Bluetooth

bool ponteLevantada = false;  // Flag para indicar se a ponte está levantada

/*-------------------FUNÇÃO VOID SETUP------------------------------ */
/*--------------------------------------------------------- */
void setup() {
  myservo.attach(9); // Conectar o servo ao pino 9
  myservo.write(0);  // Configurar o servo na posição inicial (0 graus)
  
  Serial.begin(9600);    // Inicializar a comunicação serial
  BTSerial.begin(9600);  // Inicializar a comunicação com o Bluetooth
  lcd.begin(16, 2);      // Inicializar o LCD (16 colunas, 2 linhas)
  pinMode(SENSOR_NIVEL_PIN, INPUT);  // Definir o pino do sensor de nível de água como entrada
  pinMode(buzzer, OUTPUT);           // Definir o pino do buzzer como saída
  digitalWrite(buzzer, LOW);         // Desativar o buzzer inicialmente

  // Testar o servo no início para verificar sua funcionalidade
  myservo.write(90);  // Movimentar o servo para 90 graus
  delay(1000);         // Esperar 1 segundo
  myservo.write(0);    // Voltar o servo para a posição inicial (0 graus)
  delay(1000);         // Esperar 1 segundo
}

/*-------------------FUNÇÃO VOID LOOP------------------------------ */
/*--------------------------------------------------------- */
void loop() {
  int temperatura = 0;   // Variável para armazenar a temperatura
  int humidade = 0;      // Variável para armazenar a humidade
  int resultado = dht11.readTemperatureHumidity(temperatura, humidade); // Ler temperatura e humidade do sensor DHT11

  // Só processar os dados de temperatura e humidade se a ponte não estiver levantada
  if (!ponteLevantada) {
    if (resultado == 0) {  // Se a leitura for bem-sucedida
      lcd.clear();  // Limpar o LCD
      lcd.setCursor(0, 0);  // Definir o cursor na primeira linha
      lcd.print("Temperatura: ");
      lcd.print(temperatura);
      lcd.print("C");
      lcd.setCursor(0, 1);  // Definir o cursor na segunda linha
      lcd.print("Humidade: ");
      lcd.print(humidade);
      lcd.print("%");

      // Enviar os valores para o monitor serial e para o Bluetooth
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.print(" °C\tHumidade: ");
      Serial.print(humidade);
      Serial.println(" %");

      BTSerial.print("Temperatura: ");
      BTSerial.print(temperatura);
      BTSerial.print(" °C\tHumidade: ");
      BTSerial.println(humidade);

      // Se a temperatura ou humidade for muito alta, ativar o buzzer
      if (temperatura >= 55 || humidade >= 80) {
        digitalWrite(buzzer, HIGH);  // Ativar o buzzer
      } else {
        digitalWrite(buzzer, LOW);   // Desativar o buzzer
      }
    } else {  // Se houver um erro na leitura do DHT11
      String erro = DHT11::getErrorString(resultado);  // Obter mensagem de erro
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Erro DHT11");
      Serial.println(erro);  // Exibir erro no monitor serial
      BTSerial.println(erro); // Enviar erro para o Bluetooth
      digitalWrite(buzzer, LOW);  // Garantir que o buzzer está desligado
    }
  }

  delay(2000);  // Aguardar 2 segundos antes de continuar

  int nivelDeAgua = digitalRead(SENSOR_NIVEL_PIN);  // Ler o valor do sensor de nível de água

  lcd.clear();  // Limpar o LCD
  if (nivelDeAgua == LOW) {  // Se não houver água detectada
    lcd.setCursor(1, 0);   // Definir o cursor na primeira linha
    lcd.print("Nivel de agua:");
    lcd.setCursor(4, 1);   // Definir o cursor na segunda linha
    lcd.print("Sem agua");

    Serial.println("Não foi detectada água.");
    BTSerial.println("Não foi detectada água.");
    digitalWrite(buzzer, LOW);  // Garantir que o buzzer está desligado
  } else {  // Se houver água detectada
    lcd.setCursor(1, 0);   // Definir o cursor na primeira linha
    lcd.print("Nivel de agua:");
    lcd.setCursor(2, 1);   // Definir o cursor na segunda linha
    lcd.print("Existe agua");

    Serial.println("Água detectada.");
    BTSerial.println("Água detectada.");

    // Levantar a ponte (mover o servo para 90 graus)
    angulo = 90;
    myservo.write(angulo);

    // Marcar que a ponte está levantada
    ponteLevantada = true;

    // Exibir "Ponte" e "Interdita" no LCD
    delay(2000);  // Esperar um pouco antes de mostrar a mensagem
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Ponte");

    lcd.setCursor(3, 1);
    lcd.print("Interdita");

    Serial.println("Ponte interdita, a levantar cancela.");
    BTSerial.println("Ponte interdita, a levantar cancela.");

    // Ativar o buzzer enquanto exibe a mensagem
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);

    // Deixar a ponte levantada por mais tempo (aqui estamos deixando 5 segundos)
    delay(5000);  // Ponte levantada por 5 segundos

    // Agora, abaixar a ponte e marcar que a ponte não está mais levantada
    myservo.write(0);
    ponteLevantada = false;  // Resetar a flag da ponte levantada
  }

  delay(2000);  // Aguardar 2 segundos antes de continuar
}
