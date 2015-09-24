int EEPROMgetInt(int start) {
  Serial.print("Reading int from EEPROM, start: ");
  Serial.print(start);

  int value = 0;

  EEPROM.get(start, value);

  Serial.print(", value: ");
  Serial.println(value);

  return value;
}

void EEPROMputString(uint8_t start, String string) {
  uint8_t length = 0;
  if(string.length() > 255)
    length = 255;
  else
    length = string.length();

  Serial.print("Writing string to EEPROM, length: ");
  Serial.print(length);
  Serial.print(", start: ");
  Serial.print(start);
  Serial.print(", value: ");

  // write the length of the string first
  EEPROM.write(start, length);

  for (uint8_t i = 0; i < length; i++)
  {
    Serial.print(string[i]);
    EEPROM.write(start + 1 + i, string[i]);
  }

  Serial.println();
}

String EEPROMgetString(int start) {
  uint8_t length = EEPROM.read(start);

  String string;
  string.reserve(length);

  Serial.print("Reading string from EEPROM, start: ");
  Serial.print(start);
  Serial.print(", length: ");
  Serial.print(length);
  Serial.print(", chars: ");

  for (uint8_t i = 0; i < length; i++)
  {
    char c = EEPROM.read(start + 1 + i);
    Serial.print(c);
    string += c;
  }

  Serial.print(", string: ");
  Serial.print(string);

  Serial.println();

  return string;
}

void readSettings()
{
  int address = 0;

  // EEPROM seems to get erased when you upload new code
  // read EEPROM validity flag
  uint8_t check = EEPROM.read(address);

  if(check != 42)
    return;

  address += sizeof(check);

  setModeIndex(EEPROMgetInt(address));
  address += sizeof(modeIndex);

  setTimezone(EEPROMgetInt(address));
  address += sizeof(timezone);

  setZip(EEPROMgetInt(address));
  address += sizeof(zip);

  setAmPm(EEPROMgetInt(address));
  address += sizeof(ampm);

  setGmailUserAndPassword(EEPROMgetString(address));
  address += sizeof(gmailUserAndPassword) + 1;
}

void writeSettings()
{
  int address = 0;

  uint8_t check = 42;

  // write EEPROM validity flag
  EEPROM.write(address, check);
  address += sizeof(check);

  EEPROM.put(address, modeIndex);
  address += sizeof(modeIndex);

  EEPROM.put(address, timezone);
  address += sizeof(timezone);

  EEPROM.put(address, zip);
  address += sizeof(zip);

  EEPROM.put(address, ampm);
  address += sizeof(ampm);

  EEPROMputString(address, gmailUserAndPassword);
  address += sizeof(gmailUserAndPassword) + 1;
}
