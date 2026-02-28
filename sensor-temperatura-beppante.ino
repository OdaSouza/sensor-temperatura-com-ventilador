// Definições dos pinos
#define PINO_SENSOR_TEMP A0     // Pino analógico para o sensor de temperatura
#define PINO_MOTOR 7             // Pino digital para controlar o motor
#define PINO_LED 8                // Pino para o LED de alerta
#define PINO_BUZZER 9             // Pino para o buzzer

// Limites de temperatura
#define TEMPERATURA_LIMITE_MOTOR 30.0  // Temperatura para ligar o motor
#define TEMPERATURA_CRITICA 50.0       // Temperatura crítica para alerta

// Constantes do sensor (para LM35)
#define TENSAO_REFERENCIA 5.0    // Tensão de referência do Arduino (5V)
#define RESOLUCAO_ADC 1023.0     // Resolução do ADC (10 bits = 1023)
#define FATOR_LM35 10.0           // LM35: 10mV por °C

// Variáveis de estado
bool motorLigado = false;
bool alertaAtivo = false;
unsigned long ultimoBeep = 0;
bool estadoLed = false;

void setup() {
  // Inicia comunicação serial
  Serial.begin(9600);
  Serial.println("Sistema com Sensor Analógico de Temperatura Iniciado!");
  Serial.println("==================================================");
  
  // Configura pinos de saída
  pinMode(PINO_MOTOR, OUTPUT);
  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  
  // Garante que tudo comece desligado
  digitalWrite(PINO_MOTOR, LOW);
  digitalWrite(PINO_LED, LOW);
  digitalWrite(PINO_BUZZER, LOW);
  
  delay(1000);
}

void loop() {
  // Faz a leitura do sensor analógico
  int valorSensor = analogRead(PINO_SENSOR_TEMP);
  
  // Converte para temperatura (para LM35)
  float tensao = (valorSensor * TENSAO_REFERENCIA) / RESOLUCAO_ADC;
  float temperatura = tensao * 100.0;  // LM35: 10mV/°C -> 1V = 100°C
  
  // Para TMP36 (comente a linha acima e use esta):
  // float temperatura = (tensao - 0.5) * 100.0;  // TMP36: 0.5V offset, 10mV/°C
  
  // Exibe os valores no Monitor Serial
  Serial.print("Valor ADC: ");
  Serial.print(valorSensor);
  Serial.print(" | Tensão: ");
  Serial.print(tensao, 3);
  Serial.print("V | Temperatura: ");
  Serial.print(temperatura, 1);
  Serial.println(" °C");
  
  // ===== VERIFICAÇÃO DE TEMPERATURA CRÍTICA (>50°C) =====
  if (temperatura > TEMPERATURA_CRITICA) {
    // Ativa alerta crítico
    if (!alertaAtivo) {
      Serial.println("!!! ALERTA CRÍTICO: Temperatura acima de 50°C !!!");
      alertaAtivo = true;
    }
    
    // Aciona LED e buzzer (padrão de alerta)
    acionarAlertaCritico();
    
  } else {
    // Se temperatura normalizou e estava em alerta, desliga tudo
    if (alertaAtivo) {
      Serial.println("--- Temperatura normalizada. Alertas desligados. ---");
      desligarAlertas();
    }
    
    // ===== CONTROLE NORMAL DO MOTOR =====
    if (temperatura > TEMPERATURA_LIMITE_MOTOR) {
      // Liga motor se necessário
      if (!motorLigado) {
        digitalWrite(PINO_MOTOR, HIGH);
        motorLigado = true;
        Serial.println(">> MOTOR LIGADO - Temperatura acima do limite (30°C)");
      }
    } else {
      // Desliga motor se necessário
      if (motorLigado) {
        digitalWrite(PINO_MOTOR, LOW);
        motorLigado = false;
        Serial.println(">> MOTOR DESLIGADO - Temperatura normalizada");
      }
    }
  }
  
  Serial.println("---");
  delay(1000);  // Aguarda 1 segundo (sensor analógico não precisa de delay longo)
}

// Função para acionar o alerta crítico
void acionarAlertaCritico() {
  unsigned long agora = millis();
  
  // Padrão de alerta: LED piscando rápido e beeps intermitentes
  if (agora - ultimoBeep > 300) {  // A cada 300ms
    ultimoBeep = agora;
    
    // Alterna estado do LED
    estadoLed = !estadoLed;
    digitalWrite(PINO_LED, estadoLed);
    
    // Se LED acendeu, dá beep
    if (estadoLed) {
      tone(PINO_BUZZER, 2000, 200);  // Beep de 2000Hz por 200ms
    }
  }
  
  // Mensagem de alerta a cada 5 segundos no serial
  static unsigned long ultimaMsg = 0;
  if (agora - ultimaMsg > 5000) {
    ultimaMsg = agora;
    Serial.println("!!! SISTEMA EM ALERTA CRÍTICO - TEMPERATURA > 50°C !!!");
  }
}

// Função para desligar todos os alertas
void desligarAlertas() {
  digitalWrite(PINO_LED, LOW);
  noTone(PINO_BUZZER);
  alertaAtivo = false;
  estadoLed = false;
}
