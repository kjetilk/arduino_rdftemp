#include <OneWire.h>
#include <Time.h>

#define LED 13
#define TEMPPIN 2
#define SERIAL_BAUD   9600
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 


void setup(void) {
  Serial.begin(SERIAL_BAUD);
  setSyncProvider( requestSync);  // Syncing for timestamp
}

void loop(void) {
  
  if (Serial.available()) {
    processSyncMessage();
  } else {
    Serial.println("No time sync");
  }

  for (int i=9;i<13;i++){
  handleOWIO(TEMPPIN,i);
  Serial.println();
}

  delay(1000);
  Blink(LED,3);
}

void handleOWIO(byte pin, byte resolution) {
  int owpin = pin;

  // Device identifier
  byte dsaddr[8];
  OneWire myds(owpin);
  getfirstdsadd(myds,dsaddr);

  Serial.print(now());
  Serial.print(F("dsaddress:"));
  int j;
  for (j=0;j<8;j++) {
    if (dsaddr[j] < 16) {
    Serial.print('0');
  }
  Serial.print(dsaddr[j], HEX);
}
// Data

Serial.println(getdstemp(myds, dsaddr, resolution));

} // run OW sequence

void getfirstdsadd(OneWire myds, byte firstadd[]){
  byte i;
  byte present = 0;
  byte addr[8];
  float celsius, fahrenheit;

  int length = 8;

  //Serial.print("Looking for 1-Wire devices...\n\r");
  while(myds.search(addr)) {
    //Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
    for( i = 0; i < 8; i++) {
      firstadd[i]=addr[i];
      //Serial.print("0x");
      if (addr[i] < 16) {
          //Serial.print('0');
      }
      //Serial.print(addr[i], HEX);
      if (i < 7) {
        //Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      //Serial.print("CRC is not valid!\n");
      return;
    }
    // the first ROM byte indicates which chip

    //Serial.print("\n\raddress:");
    //Serial.print(addr[0]);

    return;
  }
}

float getdstemp(OneWire myds, byte addr[8], byte resolution) {
  byte present = 0;
  int i;
  byte data[12];
  byte type_s;
  float celsius;
  float fahrenheit;

  switch (addr[0]) {
    case 0x10:
      //Serial.println(F("  Chip = DS18S20"));  // or old DS1820
      type_s = 1;
    break;
    case 0x28:
      //Serial.println(F("  Chip = DS18B20"));
      type_s = 0;
    break;
    case 0x22:
      //Serial.println(F("  Chip = DS1822"));
      type_s = 0;
    break;
    default:
      Serial.println(F("Device is not a DS18x20 family device."));
  }

  // Get byte for desired resolution
  byte resbyte = 0x1F;
  if (resolution == 12){
    resbyte = 0x7F;
  }
  else if (resolution == 11) {
    resbyte = 0x5F;
  }
  else if (resolution == 10) {
    resbyte = 0x3F;
  }

  // Set configuration
  myds.reset();
  myds.select(addr);
  myds.write(0x4E);         // Write scratchpad
  myds.write(0);            // TL
  myds.write(0);            // TH
  myds.write(resbyte);         // Configuration Register

  myds.write(0x48);         // Copy Scratchpad

  myds.reset();
  myds.select(addr);

  long starttime = millis();
  myds.write(0x44,1);         // start conversion, with parasite power on at the end
  while (!myds.read()) {
    // do nothing
  }
  Serial.print("Conversion took: ");
  Serial.print(millis() - starttime);
  Serial.println(" ms");

  //delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = myds.reset();
  myds.select(addr);
  myds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present,HEX);
  Serial.println("Raw Scratchpad Data: ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = myds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // convert the data to actual temperature

  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    } else {
      byte cfg = (data[4] & 0x60);
      if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
// default is 12 bit resolution, 750 ms conversion time
    }
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("Temp (C): ");
  //Serial.println(celsius);
  return celsius;
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}


void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

