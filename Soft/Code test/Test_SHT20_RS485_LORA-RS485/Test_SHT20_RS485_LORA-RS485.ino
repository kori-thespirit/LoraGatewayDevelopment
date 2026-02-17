#include <ModbusMaster.h>

#define MAX485_DE 15
#define MAX485_RE 16
#define RS485_EN  12

HardwareSerial RS485Serial(2);
ModbusMaster node;

void preTransmission()
{
  digitalWrite(MAX485_DE, HIGH);
  digitalWrite(MAX485_RE, HIGH);
}

void postTransmission()
{
  digitalWrite(MAX485_DE, LOW);
  digitalWrite(MAX485_RE, LOW);
}

void setup()
{
  Serial.begin(115200);

  pinMode(MAX485_DE, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  pinMode(RS485_EN, OUTPUT);

  digitalWrite(RS485_EN, HIGH);
  delay(100);

  digitalWrite(MAX485_DE, LOW);
  digitalWrite(MAX485_RE, LOW);

  RS485Serial.begin(9600, SERIAL_8N1, 14, 17);

  node.begin(1, RS485Serial);   // ID mặc định = 1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop()
{
  // Function 0x04 → đọc input register
  uint8_t result = node.readInputRegisters(0x0001, 2);

  if (result == node.ku8MBSuccess)
  {
    int16_t tempRaw = node.getResponseBuffer(0);
    int16_t humRaw  = node.getResponseBuffer(1);

    float temp = tempRaw / 10.0;
    float hum  = humRaw / 10.0;

    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.println(" %");
  }
  else
  {
    Serial.print("Error: ");
    Serial.println(result);
  }

  delay(2000);
}