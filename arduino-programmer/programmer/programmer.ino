#define DATA_START 52
#define ADDR_START 30
#define LOAD 2
#define BAUD_RATE 115200


void setupPorts()
{
  pinMode(LOAD, OUTPUT);
  digitalWrite(LOAD, 1);
  for (int i = 0, p = DATA_START; i < 8; ++i, p -= 2) 
  {
    pinMode(p, OUTPUT);
  }
  for (int i = 0, p = ADDR_START; i < 5; ++i, p-=2)
  {
    pinMode(p, OUTPUT);
  }
}

void sendDataByte(uint8_t byte)
{
  for (uint8_t mask = 0x80, i = 0; mask != 0; mask >>= 1, ++i) {
    if (byte & mask) digitalWrite(DATA_START - (2 * i), 1);
    else digitalWrite(DATA_START - (2 * i), 0);
  }
}

void sendAddrByte(uint8_t byte)
{
  for (uint8_t mask = 0b00010000, i = 0; mask != 0; mask >>= 1, ++i) {
    if (byte & mask) digitalWrite(ADDR_START - (2 * i), 1);
    else digitalWrite(ADDR_START - (2 * i), 0);
  }

}


void load() 
{
  delay(20);
  digitalWrite(LOAD, 0);
  delay(50);
  digitalWrite(LOAD, 1);
  delay(10);
}


void writeByteToMemory(uint8_t addr, uint8_t value) {
  sendAddrByte(addr);
  sendDataByte(value);
  load();
}

void uploadProgram() {
  // Protocol:
  // Host sends: 'U' + len_lo + len_hi + <len bytes of program>
  // Arduino replies: "OK"
  while (Serial.available() < 2) { }
  uint16_t len = (uint16_t)Serial.read();
  len |= (uint16_t)Serial.read() << 8;

  for (uint16_t i = 0; i < len; ++i) {
    while (Serial.available() == 0) { }
    uint8_t b = (uint8_t)Serial.read();
    writeByteToMemory((uint8_t)i, b);
  }

  Serial.println("OK");
}

void setup() {
  delay(1500);
  Serial.begin(BAUD_RATE);
  setupPorts();
  sendAddrByte(0x00);
  delay(100);
  Serial.println("Ready. Send 'U' to upload or 'A<number>' to select address.");
  delay(100);
}

void loop() {
  if (Serial.available() == 0) {
    return;
  }

  char cmd = (char)Serial.read();
  if (cmd == 'U') {
    uploadProgram();
    return;
  }

  if (cmd == 'A') {
    int num = Serial.parseInt();
    Serial.print("Changing to addr ");
    Serial.print(num);
    Serial.println("...");
    sendAddrByte((uint8_t)num);
    return;
  }
}
