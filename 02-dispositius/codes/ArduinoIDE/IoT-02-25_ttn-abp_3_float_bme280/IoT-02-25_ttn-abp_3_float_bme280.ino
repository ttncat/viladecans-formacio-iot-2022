// https://github.com/jordibinefa/IoT-02/tree/master/codes
// things.cat
//
// Based on https://github.com/jordibinefa/arduino-IDE-codes/tree/master/esp32_lorawan_ttnEsp32_ttgo_multichannel_02_icm00_10_iot02
//
// Code written by Jordi Binefa. 20200502

#include "IoT-02_pinout.h"
#include "IoT-02_common.h"

#include "IoT-02_oled.h"
SSD1306  display(0x3c, I2C_SDA, I2C_SCL);

#include "IoT-02_modbus.h"
HardwareSerial modbusData(2);
#define MODBUS_BAUD_RATE 4800

#include "IoT-02_bme280.h"
Adafruit_BME280 bme;

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "IoT-02_lora_credentials.h"

#define TX_INTERVAL         300

// Requires this fork: https://github.com/jpmeijers/arduino-lmic
const lmic_pinmap lmic_pins = {
    .nss = NSS_GPIO,
    .rxtx = RXTX_GPIO,
    .rst = RESET_GPIO,
    .dio = {DIO0_GPIO, DIO1_GPIO, DIO2_GPIO},
};

#define ESP32

#ifndef CFG_eu868
#error "This script is meant to connect to TTN EU network at 868MHz"
#endif

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// Job
static osjob_t sendjob;

// Value
unsigned long autoincrement = 0;

// -----------------------------------------------------------------------------
// LMIC
// -----------------------------------------------------------------------------

void vSwap(byte* a, byte* b) {
  byte byAux = *b;

  *b = *a;
  *a = byAux;
}
void float2Bytes(float val, byte* bytes_array) {
  // Create union of shared memory space
  union {
    float float_variable;
    byte temp_array[4];
  } u;
  // Overite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
  vSwap(&bytes_array[0], &bytes_array[3]);
  vSwap(&bytes_array[1], &bytes_array[2]);
}

void ttnSend(osjob_t* j) {

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    //Serial.println(F("[RFM95] Pending message"));
    return;
  }

  // Prepare buffer
  unsigned char data[12];
  float fTc, fTf, fP, fRH;
  int nTx100, nPx100, nRHx100, nAx100, nGr;
  //data[0] = (autoincrement >> 24) & 0xFF;
  //data[1] = (autoincrement >> 16) & 0xFF;
  //data[2] = (autoincrement >>  8) & 0xFF;
  //data[3] = (autoincrement >>  0) & 0xFF;
  //float2Bytes(fT,data);
  //vReadBME280(&fTc, &fTf, &fP, &fRH);
  vReadingBME280(&nTx100,&nPx100,&nRHx100,&nGr,&nAx100);
  fTc = ((float)nTx100)/100; fP = ((float)nPx100)/100; fRH = ((float)nRHx100)/100;

  if (fTc > -10.f && fTc < 90.) {
    
    Serial.print("T:");
    Serial.print(fTc);
    Serial.print(" *C <--> ");
    
    float2Bytes(fTc, data);
    
    for (int i = 0; i < 4 ; i++) {
      Serial.print("[0x");
      Serial.print(data[i], HEX);
      Serial.print("]");
    }
    Serial.println();
    //
    Serial.print("P:");
    Serial.print(fP);
    Serial.print(" hPa <--> ");
    
    float2Bytes(fP, &data[4]);
    
    for (int i = 0; i < 4 ; i++) {
      Serial.print("[0x");
      Serial.print(data[i], HEX);
      Serial.print("]");
    }
    Serial.println();    
    //
    Serial.print("HR:");
    Serial.print(fRH);
    Serial.print(" %% <--> ");
    
    float2Bytes(fP, &data[8]);
    
    for (int i = 0; i < 4 ; i++) {
      Serial.print("[0x");
      Serial.print(data[i], HEX);
      Serial.print("]");
    }
    Serial.println();  
    // Prepare upstream data transmission at the next possible time.
    // Parameters are port, data, length, confirmed
    LMIC_setTxData2(1, data, 12, 0);

    Serial.println(F("[RFM95] Packet queued"));
  }

  // Next TX is scheduled after TX_COMPLETE event.
  autoincrement++;
}

// LMIC library will call this method when an event is fired
void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");

  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("[RFM95] EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("[RFM95] EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("[RFM95] EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("[RFM95] EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("[RFM95] EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("[RFM95] EV_JOINED"));
      break;
    case EV_RFU1:
      Serial.println(F("[RFM95] EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("[RFM95] EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("[RFM95] EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("[RFM95] EV_TXCOMPLETE (includes waiting for RX windows)"));

      if (TXRX_ACK) {
        Serial.println(F("TXRX_ACK confirmed UP frame was acked\n"));
      }
      Serial.println(F("TX complete ....................................."));
      Serial.print(LMIC.dataLen); Serial.print(" ");
      Serial.println(LMIC.frame[LMIC.dataBeg - 1]);
      for (int i = 0;  i < LMIC.dataLen;  i++) {
        Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
      }
      Serial.println(" ");

      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("[RFM95] ACK received"));
      }
      /*
      if (LMIC.dataLen == 1) {
        if (LMIC.frame[LMIC.dataBeg] == 0x71){
          Serial.println("Activació de relé");
          PCF_38.write8(0xF7);
        }
        if (LMIC.frame[LMIC.dataBeg] == 0x51){
          Serial.println("Desactivació de relé");
          PCF_38.write8(0xFF);
        }
      }
      */
      if (LMIC.dataLen > 0 && LMIC.dataLen <= 2) {
        char szHex[3] = {0}; 
        szHex[0] = char(LMIC.frame[LMIC.dataBeg]);
        if(LMIC.dataLen == 2)
          szHex[1] = char(LMIC.frame[LMIC.dataBeg+1]);
        //szHex[2] = 0;
        //PlcSerial.print(szHex);
        //vPresentaPantallaDemo(szHex);
        Serial.print("szHex: ");
        Serial.println(szHex);
      }
      
      if (LMIC.dataLen) {
        Serial.print(F("[RFM95] Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));

        for (int i = 0; i < LMIC.dataLen; i++) {
          if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
            Serial.print(F("0"));
          }
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
      
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), ttnSend);
      break;

    case EV_LOST_TSYNC:
      //Serial.println(F("[RFM95] EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      //Serial.println(F("[RFM95] EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      //Serial.println(F("[RFM95] EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      //Serial.println(F("[RFM95] EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      //Serial.println(F("[RFM95] EV_LINK_ALIVE"));
      break;
    default:
      //Serial.println(F("[RFM95] Unknown event"));
      break;

  }

}

void ttnSetup() {
  // LMIC init
  os_init();

  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100); // A treure

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);

  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7, 14);

  // Start job
  ttnSend(&sendjob);
}



void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);
  vSetupIO();
  vSetupScreen();
  vSetupModBus(MODBUS_BAUD_RATE);
  Serial.begin(115200);
  Serial.println(__FILE__);
  vSetupBME280();

  // SPI interface
#ifdef ESP32
  SPI.begin(SCK_GPIO, MISO_GPIO, MOSI_GPIO, NSS_GPIO);
#endif

  // Init LMIC library to work with TTN EU
  ttnSetup();  
}

void loop() {
  static boolean bIO0wasPressed = false;
  static boolean bI35wasPressed = false;
  static boolean bI34wasPressed = false;
  static int nCmptIO0 = 0;
  boolean bLatchingRelayState = digitalRead(STATE_LATCHING_RELAY);
  boolean bIO0currentState = bPressedButton(BT_IO0);
  boolean bI34currentState = bPressedButton(BT_I34);
  boolean bI35currentState = bPressedButton(BT_I35);
  boolean bLedG, bLedY, bLedR, bLedW;

  // Keeps track of the scheduled jobs
  os_runloop_once();
  
  if (bIO0wasPressed != bIO0currentState) {
    delay(2);
    if (bIO0currentState) {
      bLatchingRelayState = !bLatchingRelayState;
      vLatchingRelay(bLatchingRelayState);
      Serial.print("bLatchingRelayState: ");
      (bLatchingRelayState) ? Serial.println("ON") : Serial.println("OFF");
      ttnSend(NULL); // <--------------------------------------------- Sending LoRaWAN message
      nCmptIO0++;
    }
    bIO0wasPressed = bIO0currentState;
  }
  if (bI34wasPressed != bI34currentState) {
    delay(2);
    if (bI34currentState) {
      bLatchingRelayState = HIGH;
      vLatchingRelay(bLatchingRelayState);
      Serial.print("bLatchingRelayState: ");
      (bLatchingRelayState) ? Serial.println("ON") : Serial.println("OFF");
    }
    bI34wasPressed = bI34currentState;
  }
  if (bI35wasPressed != bI35currentState) {
    delay(2);
    if (bI35currentState) {
      bLatchingRelayState = LOW;
      vLatchingRelay(bLatchingRelayState);
      Serial.print("bLatchingRelayState: ");
      (bLatchingRelayState) ? Serial.println("ON") : Serial.println("OFF");
    }
    bI35wasPressed = bI35currentState;
  }
  bLedG = bPressedButton(BT_I35);
  bLedY = bLatchingRelayState;
  bLedR = bPressedButton(BT_I34);
  bLedW = bPressedButton(BT_IO0);
  digitalWrite(LED_G, bLedG);
  digitalWrite(LED_Y, bLedY);
  digitalWrite(LED_R, bLedR);
  digitalWrite(LED_W, bLedW);
  vScreenDemo(bLedG, bLedY, bLedR, bLedW,nCmptIO0);
  vModBusReading();
  
  delay(10);
}
