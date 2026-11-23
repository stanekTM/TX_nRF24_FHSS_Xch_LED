/*
*****************************************************************************************************
Simple RC transmitter with frequency hopping (FHSS) and telemetry (Auto-ACK Payload)
*****************************************************************************************************
Code from my repository: https://github.com/stanekTM/TX_nRF24_FHSS_Xch_LED

Rapid hopping between 4 strong RF channels (12, 40, 76, 88) outside the main WiFi frequencies.
Transmission rate of 1 Mbps (increased for full support of the newer nRF52 generation).
Constant latency / period of 4.5 ms (222 Hz).
RC channels from 2 to 7 with 16-bit resolution (1000–2000 us).
Received telemetry indicates low receiver battery voltage A1(A2) and signal loss by flashing an LED.

Support for DIY RC receivers from this repository: https://github.com/stanekTM/RX_nRF24_FHSS
*****************************************************************************************************
*/

#include <RF24.h>   // Ovladač bezdrátového čipu nRF24L01+ (v1.4.11 nebo novější)
#include <SPI.h>    // Standardní knihovna Arduino
#include <EEPROM.h> // Standardní knihovna Arduino

// Setting the transmitter and receiver battery alarm for the status LED
#define TX_BATTERY_VOLTAGE    4.2  // Set the maximum rated voltage for the transmitter battery
#define TX_MONITORED_VOLTAGE  3.45 // Set the minimum voltage for the transmitter battery alarm

#define RX_BATTERY_VOLTAGE    4.2  // Set the maximum rated voltage for the receiver battery
#define RX_MONITORED_VOLTAGE  3.45 // Set the minimum voltage for the receiver battery alarm

// Control range (1000 to 2000 us)
#define MIN_CONTROL_VAL  1000
#define MID_CONTROL_VAL  1500
#define MAX_CONTROL_VAL  2000

// Select analog pins for reading the potentiometers (max. 7)
const uint8_t pot_pins[] = {A0, A1, /*A2, A3, A4, A5, A6*/};

// Status LED pin
#define PIN_LED           6

// Calibration or Fail-Safe button pin
#define PIN_BTN_CALIB_FS  4

// Analog pin for battery reading
#define PIN_BATTERY       A7

// Pins for nRF24L01+
#define PIN_CE            9
#define PIN_CSN           10

// Hardware SPI pins
//          MOSI          11
//          MISO          12
//          SCK           13

// Inicializace rádiového čipu nRF24L01+
RF24 radio(PIN_CE, PIN_CSN);

// Unikátní RF adresa max. 5 bajtů (číslo nebo znak). Musí být stejná i v přijímači
const uint8_t rf_address[] = "jirka";

// 4 silné RF kanály (mimo hlavní WiFi frekvence)
const uint8_t rf_channels[] = {12, 40, 76, 88};
uint8_t fhss_index = 0; // Ukazatel na aktuálně používaný RF kanál z tabulky

// Při změně počtu pinů automaticky spočítá potenciometry
const uint8_t NUM_RC_CHANNELS = sizeof(pot_pins);

// Struktura odesílaných dat
struct tx_packet
{
  uint8_t fail_safe_flag = 0; // Příznak Fail-Safe
  uint16_t rc_data[NUM_RC_CHANNELS];
}
tx_data;

// Struktura přijímaných telemetrických dat (ACK Payload)
struct rx_packet
{
  uint8_t rssi = 0;
  uint8_t batt_A1 = 255;
  uint8_t batt_A2 = 255;
}
rx_telemetry_data;

// Interval skoku (4500 us / 222 Hz)
const uint32_t HOP_INTERVAL = 4500;
uint32_t hop_timer = 0; // Časovač FHSS skoků

// Hlídač totální ztráty signálu
uint32_t signal_loss_timeout = 0;

// Časovač příznaku Fail-Safe
uint32_t fail_safe_timer = 0;

// Časovač pro neblokující tisk do PC
uint32_t serial_print_timer = 0;

// Měření baterií
bool low_batt_flag_tx = false; // Příznak vybité baterie vysílače
bool low_batt_flag_rx = false; // Příznak vybité baterie A1 a A2 přijímače

// Příznak kalibrace
bool calibrate = true;

// Čtení potenciometrů
uint16_t pots_value[NUM_RC_CHANNELS];
uint16_t min_pots_calib[NUM_RC_CHANNELS];
uint16_t mid_pots_calib[NUM_RC_CHANNELS];
uint16_t max_pots_calib[NUM_RC_CHANNELS];
uint8_t reverse[NUM_RC_CHANNELS];

// Prototypy všech funkcí na začátku, aby je smyčka správně viděla
void radio_init();
void calibrate_reverse_pots();
void manage_rf_link();
void read_pots();
void read_batt();
void blink(const uint8_t pin, uint32_t interval);

//***************************************
// Hlavní konfigurace systému při startu
//***************************************
void setup()
{
  Serial.begin(115200);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BATTERY, INPUT);
  pinMode(PIN_BTN_CALIB_FS, INPUT_PULLUP);
  
  // Inicializace dat
  for (uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    tx_data.rc_data[i] = 1500;
    pots_value[i] = 1500;
    min_pots_calib[i] = 1023;
    mid_pots_calib[i] = 512;
    max_pots_calib[i] = 0;
  }
  
  // Inicializace ovladače rádia
  radio_init();
  
  // Kalibrovat nebo reverzovat potenciometry
  calibrate_reverse_pots();
  
  // Inicializace časovačů
  hop_timer = micros();
  signal_loss_timeout = millis();
  fail_safe_timer = millis();
}

//**********************************
// Hlavní opakovací smyčka programu
//**********************************
void loop()
{
  // Vysílaní a příjem telemetrických dat
  manage_rf_link();
  
  // Čtení potenciometrů
  read_pots();
  
  // Měření napětí baterií
  read_batt();
  
  // Po stisku tlačítka aktivujeme příznak Fail-Safe a uložíme aktuální čas
  if (digitalRead(PIN_BTN_CALIB_FS) == LOW)
  {
    tx_data.fail_safe_flag = 1;
    fail_safe_timer = millis();
  }
  // Pokud příznak Fail-Safe běží a uplynuly 2 sekundy (2000 ms), vypneme ho
  if (millis() - fail_safe_timer >= 2000) tx_data.fail_safe_flag = 0;
  
  /*
  // Výpis dat na sériový monitor
  if (millis() - serial_print_timer >= 200)
  {
    serial_print_timer = millis();
    
    if (millis() - signal_loss_timeout <= 1000) // Tiskneme data pouze pokud je model online
    {
      //Serial.print("RSSI: "); Serial.print(rx_telemetry_data.rssi);
      Serial.print(" | A1: "); Serial.print(rx_telemetry_data.batt_A1);
      Serial.print(" | A2: "); Serial.println(rx_telemetry_data.batt_A2);
    }
  }
  */
}

//*****************************************************
// Funkce pro blikání LED diody bez zpoždění procesoru
//*****************************************************
void blink(const uint8_t pin, uint32_t interval)
{
  digitalWrite(pin, (millis() / interval) % 2); // Zbytek po dělení časem (millis) určuje stav zapnuto/vypnuto
}
