#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);
  radio.begin();

  if (!radio.isChipConnected()) {
    Serial.println("NRF24 niet gevonden! Controleer bedrading en voeding.");
    while (true);
  }

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108); // 2.508 GHz
  radio.stopListening();

  Serial.println("Server klaar... Typ bijv: LAMP1ON of LAMP2OFF");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();  // maakt hoofdletters, zodat lamp1on ook werkt

    if (isValidCommand(input)) {
      radio.write(input.c_str(), input.length() + 1);
      Serial.print("Verzonden: ");
      Serial.println(input);
    } else {
      Serial.println("Ongeldig commando. Gebruik: LAMP1ON, LAMP1OFF, LAMP2ON of LAMP2OFF");
    }
  }
}

bool isValidCommand(const String &cmd) {
  return (cmd == "LAMP1ON" || cmd == "LAMP1OFF" ||
          cmd == "LAMP2ON" || cmd == "LAMP2OFF");
}
