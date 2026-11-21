//*****************************
// Inicializace ovladače rádia
//*****************************
void radio_init()
{
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); // Pro testy na stole stačí minimální výkon
  radio.setDataRate(RF24_1MBPS);
  
  radio.setAutoAck(true); // Zapnutí automatického potvrzování paketů
  radio.setRetries(0, 0); // Vypnutí hardwarového opakování
  
  radio.setPayloadSize(sizeof(tx_data)); // Nastavuje základní / maximální délku struktury
  
  radio.enableDynamicPayloads(); // Povolení automatického přizpůsobení délky paketů
  radio.enableAckPayload(); // Zapnutí funkce přibalování telemetrie do potvrzovacích ACK paketů
  
  radio.setCRCLength(RF24_CRC_16);
  
  radio.openWritingPipe(rf_address); // Otevření komunikačního kanálu pro zápis na dané adrese
  
  radio.setChannel(rf_channels[fhss_index]); // Nastavení startovního rádiového kanálu z FHSS tabulky
  radio.stopListening(); // Přepnutí do režimu vysílání
}

//**************************************
// Vysílaní a příjem telemetrických dat
//**************************************
void manage_rf_link()
{
  // Pevná FHSS časová smyčka odesílání (každých 4.5 ms)
  if (micros() - hop_timer >= HOP_INTERVAL)
  {
    //hop_timer = micros(); // Reset časovače
    
    //========================
    hop_timer += HOP_INTERVAL; // Přičteme přesný interval místo čtení aktuálního času
    
    // Pojistka: Pokud se program někde zasekl déle než 2 intervaly, srovnáme časovač
    if (micros() - hop_timer >= HOP_INTERVAL) hop_timer = micros();
    //========================
    
    // Odeslání paketu (přenos dat trvá přibližně 0.1 až 0.3 ms)
    if (radio.write(&tx_data, sizeof(tx_data)))
    {
      signal_loss_timeout = millis(); // Reset času ztráty signálu (spojení žije)
      
      // Pokud dorazil ACK paket, přečteme telemetrii
      if (radio.available())
      {
        radio.read(&rx_telemetry_data, sizeof(rx_telemetry_data));
      }
    }
    
    // Okamžitý synchronní skok vysílače na další kanál
    fhss_index = (fhss_index + 1) % 4; // Posun na další index v tabulce
    radio.setChannel(rf_channels[fhss_index]); // Nastavení rádiového kanálu na novou frekvenci
  }
  
  // Pokud z přijímače nepřišla žádná odpověď delší než 1 sekunda (1000 ms)
  if (millis() - signal_loss_timeout >= 1000) blink(PIN_LED, 100); // Krizový stav: LED bliká rychle (0.1 s)
  else if (low_batt_flag_rx) blink(PIN_LED, 300); // Vybitá baterie A1 nebo A2: LED bliká (0.3 s)
  //else if (low_batt_flag_tx) blink(PIN_LED, 800); // Vybitá baterie vysílače: LED bliká (0.8 s)
  else digitalWrite(PIN_LED, HIGH); // Normální provoz: LED trvale svítí
}
