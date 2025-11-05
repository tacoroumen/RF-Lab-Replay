#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "00001";
const int lampPin = 2;

unsigned long messageCount = 0; // Counter for received messages

void setup() {
  Serial.begin(115200);
  pinMode(lampPin, OUTPUT);
  radio.begin();

  if (!radio.isChipConnected()) {
    Serial.println("NRF24 not found! Check wiring and power.");
    while (true);
  }

  radio.setChannel(108); // 108 = 2.508 GHz
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  Serial.println("Lamp node ready and listening...");
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    
    messageCount++;
    unsigned long now = millis();

    Serial.print("#");
    Serial.print(messageCount);
    Serial.print(" [");
    Serial.print(now);
    Serial.print(" ms] Received: ");
    Serial.println(text);

    // Act on received command
    if (strcmp(text, "LAMP1ON") == 0) {
      digitalWrite(lampPin, HIGH);
      Serial.println("Lamp1 turned ON");
    } 
    else if (strcmp(text, "LAMP1OFF") == 0) {
      digitalWrite(lampPin, LOW);
      Serial.println("Lamp turned OFF");
    } 
    else {
      Serial.print("Unknown command received: ");
      Serial.println(text);
    }

    Serial.println("-----------------------------");
  }
}
