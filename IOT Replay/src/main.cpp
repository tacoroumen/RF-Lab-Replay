#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "00001";

// ---- function prototypes ----
void storeLog(const char *s);
void printLog();

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
    Serial.println("NRF24 niet gevonden! Controleer bedrading en voeding.");
    while (true);
  }

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.startListening();

  Serial.println("Receiver logger ready.");
  Serial.println("Type 'LIST' in serial to show captured messages.");
}

void loop() {
  // if a packet arrives, read and store
  if (radio.available()) {
    // read up to 31 chars + null
    char buf[32];
    memset(buf, 0, sizeof(buf));
    // RF24 read will fill the buffer; ensure we don't overflow and terminate
    radio.read(&buf, sizeof(buf));
    // ensure null-terminated
    buf[sizeof(buf)-1] = '\0';

    storeLog(buf);
    Serial.print("Received and logged: ");
    Serial.println(buf);
  }

  // check Serial commands (only LIST supported)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    if (cmd == "LIST") {
      printLog();
    } else {
      Serial.println("Unknown command. Type LIST to view log.");
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
    unsigned long t = logBuf[idx].t;
    Serial.print(i);
    Serial.print(": ");
    Serial.print(logBuf[idx].msg);
    Serial.print("  (millis=");
    Serial.print(t);
    Serial.println(")");
  }
  if (logCount == 0) Serial.println("(no messages)");
  Serial.println("---------------------------");
}
