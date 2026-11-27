# Simple RC transmitter
Simple RC transmitter with frequency hopping (FHSS) and telemetry (Auto-ACK Payload), built on the Arduino platform.

Support for DIY RC receivers from this repository [**RX_nRF24_FHSS**](https://github.com/stanekTM/RX_nRF24_FHSS)

## Features and functions
* **FHSS** Rapid hopping between 4 strong RF channels (12, 40, 76, 88) outside the main WiFi frequencies.
* **Transmission rate** 1 Mbps (for support of the newer nRF52 generation).
* **Constant latency / period** 4.5 ms (222 Hz).
* **RC channels** from 2 to 7 with 16-bit resolution (1000–2000 us).
* **Telemetry / Status LED mode**
  * Permanently lights: Normal operation, the signal is fine, and the batteries are charged.
  * Rapid flashing (100 ms): Signal loss.
  * Medium flashing (300 ms): Low receivers battery voltage A1 or A2.
  * Slow flashing (800 ms): Low transmitter battery voltage.

### Calibration
* Hold calibration button, switch RC transmitter on, still holding calibration button move all controls to extremes.
* Center all controls.
* Release calibration button (saved to eeprom).
### Reversing the stick
* To reverse the desired channel, hold the joystick in the end position and turn on the RC transmitter (saved to eeprom).
### Fail-Safe
* By pressing the calibration button, we saves the Fail-Safe positions for the servos into the receiver.

## Supported hardware
* Transceiver Nordic Semiconductor nRF24L01+.
* AVR ATmega328P microcontroller.

## Transmitter configuration description
```cpp
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
const uint8_t pot_pins[] = {A0, A1, A2, A3, /*A4, A5, A6*/};

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
```