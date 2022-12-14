// Inclou deep sleep i emmagatzemament dels paràmetres OTAA

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "IoT-02_pinout.h"
#include "IoT-02_common.h"

#include "IoT-02_bme280.h"
Adafruit_BME280 bme;

#include "IoT-02_oled.h"
SSD1306  display(0x3c, I2C_SDA, I2C_SCL);

float fT,fP,fRH;

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0xA5, 0x7B, 0x9E, 0x50, 0xDA, 0xCB, 0x2F, 0x00 };
void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0xD3, 0xBD, 0x04, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = { 0x1C, 0x27, 0x8E, 0x6A, 0x8C, 0xEE, 0xD8, 0xCB, 0x05, 0xFD, 0x06, 0x92, 0x05, 0x54, 0x4F, 0x25 };
void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

RTC_DATA_ATTR u4_t RTC_LORAWAN_netid = 0;
RTC_DATA_ATTR devaddr_t RTC_LORAWAN_devaddr = 0;
RTC_DATA_ATTR u1_t RTC_LORAWAN_nwkKey[16];
RTC_DATA_ATTR u1_t RTC_LORAWAN_artKey[16];
RTC_DATA_ATTR u1_t RTC_LORAWAN_dn2Dr;
RTC_DATA_ATTR u1_t RTC_LORAWAN_dnConf;
RTC_DATA_ATTR u4_t RTC_LORAWAN_seqnoDn;
RTC_DATA_ATTR u4_t RTC_LORAWAN_seqnoUp = 0;
RTC_DATA_ATTR s1_t RTC_LORAWAN_adrTxPow;
RTC_DATA_ATTR s1_t RTC_LORAWAN_datarate;
RTC_DATA_ATTR u1_t RTC_LORAWAN_txChnl;
RTC_DATA_ATTR s2_t RTC_LORAWAN_adrAckReq;
RTC_DATA_ATTR u1_t RTC_LORAWAN_rx1DrOffset;
RTC_DATA_ATTR u1_t RTC_LORAWAN_rxDelay;
RTC_DATA_ATTR u4_t RTC_LORAWAN_channelFreq[MAX_CHANNELS];
RTC_DATA_ATTR u2_t RTC_LORAWAN_channelDrMap[MAX_CHANNELS];
RTC_DATA_ATTR u4_t RTC_LORAWAN_channelDlFreq[MAX_CHANNELS];
RTC_DATA_ATTR band_t RTC_LORAWAN_bands[MAX_BANDS];
RTC_DATA_ATTR u2_t RTC_LORAWAN_channelMap;
RTC_DATA_ATTR int bootCount = 0;

#define N_DATA_BYTES 12

union uFloat {
 float f;
 unsigned char uc[4];
};

union uInt {
  int n;
  unsigned char uc[4];
};

// static uint8_t mydata[] = "Hello, world!";
static uint8_t mydata[N_DATA_BYTES];
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;
bool GOTO_DEEPSLEEP = false;

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 10*60 /* Time ESP32 will go to sleep (in seconds) */

// Pin mapping
#define SCK_GPIO     5
#define MISO_GPIO   15
#define MOSI_GPIO   25
#define NSS_GPIO    18 /*16*/
#define RXTX_GPIO   LMIC_UNUSED_PIN
#define RESET_GPIO  14
#define DIO0_GPIO   26
#define DIO1_GPIO   13
#define DIO2_GPIO   12

const lmic_pinmap lmic_pins = {
  .nss = NSS_GPIO,
  .rxtx = RXTX_GPIO,
  .rst = RESET_GPIO,
  .dio = {DIO0_GPIO, DIO1_GPIO, DIO2_GPIO},
};
/*
  const lmic_pinmap lmic_pins = {
  .nss = 6,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 5,
  .dio = {2, 3, 4},
  };
*/

void LMICSaveVarsToRTC()
{
  Serial.println(F("Save LMIC to RTC ..."));
  RTC_LORAWAN_netid = LMIC.netid;
  RTC_LORAWAN_devaddr = LMIC.devaddr;
  memcpy(RTC_LORAWAN_nwkKey, LMIC.nwkKey, 16);
  memcpy(RTC_LORAWAN_artKey, LMIC.artKey, 16);
  RTC_LORAWAN_dn2Dr = LMIC.dn2Dr;
  RTC_LORAWAN_dnConf = LMIC.dnConf;
  RTC_LORAWAN_seqnoDn = LMIC.seqnoDn;
  RTC_LORAWAN_seqnoUp = LMIC.seqnoUp;
  RTC_LORAWAN_adrTxPow = LMIC.adrTxPow;
  RTC_LORAWAN_datarate = LMIC.datarate;
  RTC_LORAWAN_txChnl = LMIC.txChnl;
  RTC_LORAWAN_adrAckReq = LMIC.adrAckReq;
  //RTC_LORAWAN_rx1DrOffset = LMIC.rx1DrOffset;
  RTC_LORAWAN_rxDelay = LMIC.rxDelay;
  memcpy(RTC_LORAWAN_channelFreq, LMIC.channelFreq, MAX_CHANNELS * sizeof(u4_t));
  memcpy(RTC_LORAWAN_channelDrMap, LMIC.channelDrMap, MAX_CHANNELS * sizeof(u2_t));
  //memcpy(RTC_LORAWAN_channelDlFreq, LMIC.channelDlFreq, MAX_CHANNELS * sizeof(u4_t));
  memcpy(RTC_LORAWAN_bands, LMIC.bands, MAX_BANDS * sizeof(band_t));
  RTC_LORAWAN_channelMap = LMIC.channelMap;
}

void LMICLoadVarsFromRTC()
{
  Serial.println(F("Load LMIC vars from RTC ..."));
  LMIC_setSession(RTC_LORAWAN_netid, RTC_LORAWAN_devaddr, RTC_LORAWAN_nwkKey, RTC_LORAWAN_artKey);
  LMIC.dn2Dr = RTC_LORAWAN_dn2Dr;
  LMIC.dnConf = RTC_LORAWAN_dnConf;
  LMIC.seqnoDn = RTC_LORAWAN_seqnoDn;
  //LMIC_setSeqnoUp(RTC_LORAWAN_seqnoUp);
  LMIC.seqnoUp = RTC_LORAWAN_seqnoUp; //// !!!!
  LMIC_setDrTxpow(RTC_LORAWAN_datarate, RTC_LORAWAN_adrTxPow);
  LMIC.txChnl = RTC_LORAWAN_txChnl;
  LMIC.adrAckReq = RTC_LORAWAN_adrAckReq;
  //LMIC.rx1DrOffset = RTC_LORAWAN_rx1DrOffset;
  LMIC.rxDelay = RTC_LORAWAN_rxDelay;
  memcpy(LMIC.channelFreq, RTC_LORAWAN_channelFreq, MAX_CHANNELS * sizeof(u4_t));
  memcpy(LMIC.channelDrMap, RTC_LORAWAN_channelDrMap, MAX_CHANNELS * sizeof(u2_t));
  //memcpy(LMIC.channelDlFreq, RTC_LORAWAN_channelDlFreq, MAX_CHANNELS * sizeof(u4_t));
  memcpy(LMIC.bands, RTC_LORAWAN_bands, MAX_BANDS * sizeof(band_t));
  LMIC.channelMap = RTC_LORAWAN_channelMap;
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case 1 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3 : Serial.println("Wakeup caused by timer"); break;
    case 4 : Serial.println("Wakeup caused by touchpad"); break;
    case 5 : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));

      // Disable link check validation (automatically enabled
      // during join, but not supported by TTN at this time).
      LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
        for (int i = 0; i < LMIC.dataLen; i++) {
          if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
            Serial.print(F("0"));
          }
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
          Serial.print(" ");
        }
        if(LMIC.frame[LMIC.dataBeg] == 0x0A){
          Serial.println("Ordre d'engegar el relé i el led blanc");
          digitalWrite(LED_W,HIGH);
          vLatchingRelay(HIGH);
          delay(5000); // Per a que doni temps de veure el led blanc encès
        }else{
          if(LMIC.frame[LMIC.dataBeg] == 0x05){
            Serial.println("Ordre d'apagar el relé i el led blanc");
            digitalWrite(LED_W,LOW);
            vLatchingRelay(LOW);
          }
        }
        Serial.println();
      }
      GOTO_DEEPSLEEP = true;
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void vSetFloatAtFrame(float fValue, unsigned char* ucFrame, int nPos) {
 union uFloat ufA;
 int i, k;

 ufA.f = fValue;
 Serial.print("Vector de bytes: ");
 for (i = 3 , k = 0 ; i >= 0 ; i-- , k++ ) {
   ucFrame[k+nPos] = ufA.uc[i];
   // Serial.print(ucFrame[k+nPos], DEC); Serial.print(" ");
   Serial.print(ucFrame[k+nPos], HEX); Serial.print(" ");
 }
 Serial.println();
}

void vSetIntAtFrame(int nValue, unsigned char* ucFrame, int nPos) {
 union uInt unA;
 int i, k;

 unA.n = nValue;
 Serial.print("Vector de bytes: ");
 for (i = 3 , k = 0 ; i >= 0 ; i-- , k++ ) {
   ucFrame[k+nPos] = unA.uc[i];
   // Serial.print(ucFrame[k+nPos], DEC); Serial.print(" ");
   Serial.print(ucFrame[k+nPos], HEX); Serial.print(" ");
 }
 Serial.println();
}


void do_send(osjob_t* j) {
  int nTx100, nPx100, nRHx100, nGr, nAx100;
  float fT,fP,fRH,fA;

  vReadingBME280(&nTx100, &nPx100, &nRHx100, &nGr, &nAx100);
  fT = ((float)nTx100)/100; fP = ((float)nPx100)/100; fRH = ((float)nRHx100)/100; fA = ((float)nAx100)/100;
  vScreenBME280(fT, fP, fRH, fA);
 
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    // LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    // vSetFloatAtFrame(fT,mydata,0);
    // vSetFloatAtFrame(fP,mydata,4);
    // vSetFloatAtFrame(fRH,mydata,8);
    vSetIntAtFrame(nTx100,mydata,0);
    vSetIntAtFrame(nPx100,mydata,4);
    vSetIntAtFrame(nRHx100,mydata,8);    
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, N_DATA_BYTES, 0);    
    Serial.println(F("Packet queued"));
    // Serial.print("T:  "); Serial.print(fT); Serial.println("ºC");
    // Serial.print("P:  "); Serial.print(fP); Serial.println(" hPa");
    // Serial.print("HR: "); Serial.print(fRH); Serial.println(" %");
    Serial.print("T:  "); Serial.print(float(nTx100)/100,2); Serial.println("ºC");
    Serial.print("P:  "); Serial.print(float(nPx100)/100,2); Serial.println(" hPa");
    Serial.print("HR: "); Serial.print(float(nRHx100)/100,2); Serial.println(" %");

  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.println(F("Starting"));

  Wire.begin(I2C_SDA, I2C_SCL);
  vSetupIO();
  vSetupScreen();
  vSetupBME280();
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  // LMIC init
  SPI.begin(SCK_GPIO, MISO_GPIO, MOSI_GPIO, NSS_GPIO);
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  // Load the LoRa information from RTC
  if (RTC_LORAWAN_seqnoUp != 0) {
    LMICLoadVarsFromRTC();
  }

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
  if (GOTO_DEEPSLEEP == true)  {
    //esp_sleep_enable_timer_wakeup(seconds * 1000000);
    LMICSaveVarsToRTC();
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
  }
}
