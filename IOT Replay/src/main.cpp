#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN

// Addresses for RX and TX pipes
const byte rxAddress[6] = "00001";
const byte txAddress[6] = "00001";

// ---- function prototypes ----
void storeLog(const char *s);
void printLog();
void sendLogEntry(int index);

// ---- log buffer settings ----
struct LogEntry {
  unsigned long t;   // millis() when received
  char msg[32];      // received message (null-terminated)
};

const int MAX_LOG = 50;         // keep last 50 messages
LogEntry logBuf[MAX_LOG];
int logStart = 0;               // circular buffer start index
int logCount = 0;               // number stored

void setup() {
  Serial.begin(115200);
  radio.begin();

  if (!radio.isChipConnected()) {
    Serial.println("NRF24 not found! Check wiring and power.");
    while (true);
  }

  // --- Common radio configuration ---
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);

  // --- Open pipes ---
  radio.openReadingPipe(0, rxAddress); // listen on RX address
  //radio.setAutoAck(0, true);  // Disable ACKs only on reading pipe 0

  radio.openWritingPipe(txAddress);    // send on TX address
  

  radio.startListening();

  Serial.println("Receiver logger ready.");
  Serial.println("Type 'LIST' to show messages or 'SEND <n>' to replay one.");
}

void loop() {
  // if a packet arrives, read and store
  if (radio.available()) {
    char buf[32];
    memset(buf, 0, sizeof(buf));
    radio.read(&buf, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';

    storeLog(buf);
    Serial.print("Received and logged: ");
    Serial.println(buf);
  }

  // check Serial commands (LIST or SEND n)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("LIST")) {
      printLog();
    } else if (cmd.startsWith("SEND")) {
      cmd.remove(0, 4);
      cmd.trim();
      int idx = cmd.toInt();
      if (idx >= 0 && idx < logCount) {
        sendLogEntry(idx);
      } else {
        Serial.println("Invalid index. Use LIST to see available messages.");
      }
    } else {
      Serial.println("Unknown command. Use LIST or SEND <n>.");
    }
  }
}

// ---- function definitions ----
void storeLog(const char *s) {
  if (!s) return;

  if (logCount < MAX_LOG) {
    int idx = (logStart + logCount) % MAX_LOG;
    strncpy(logBuf[idx].msg, s, sizeof(logBuf[idx].msg) - 1);
    logBuf[idx].msg[sizeof(logBuf[idx].msg) - 1] = '\0';
    logBuf[idx].t = millis();
    logCount++;
  } else {
    // overwrite oldest
    strncpy(logBuf[logStart].msg, s, sizeof(logBuf[logStart].msg) - 1);
    logBuf[logStart].msg[sizeof(logBuf[logStart].msg) - 1] = '\0';
    logBuf[logStart].t = millis();
    logStart = (logStart + 1) % MAX_LOG;
  }
}

void printLog() {
  Serial.println("---- Captured messages ----");
  for (int i = 0; i < logCount; ++i) {
    int idx = (logStart + i) % MAX_LOG;
    Serial.print(i);
    Serial.print(": ");
    Serial.print(logBuf[idx].msg);
    Serial.print("  (millis=");
    Serial.print(logBuf[idx].t);
    Serial.println(")");
  }
  if (logCount == 0) Serial.println("(no messages)");
  Serial.println("---------------------------");
}

void sendLogEntry(int index) {
  int idx = (logStart + index) % MAX_LOG;
  const char *msg = logBuf[idx].msg;

  Serial.print("Replaying message #");
  Serial.print(index);
  Serial.print(": ");
  Serial.println(msg);

  // Switch to TX mode
  radio.stopListening();
  //radio.setAutoAck(false);   // <— disable ACK requirement
  radio.openWritingPipe(txAddress);
  radio.write(msg, strlen(msg) + 1);


  // Return to RX mode
  radio.openReadingPipe(0, rxAddress);
  radio.startListening();

}
