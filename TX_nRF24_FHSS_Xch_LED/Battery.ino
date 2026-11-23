//***********************
// Měření napětí baterií
//***********************
void read_batt()
{
  // Pokud napětí vysílače klesne, uzamkneme stav vybité baterie
  if (analogRead(PIN_BATTERY) <= (1024.0 / TX_BATTERY_VOLTAGE) * TX_MONITORED_VOLTAGE)
  {
    low_batt_flag_tx = true;
  }
  
  /*
  // Pokud napětí přijímače A1 nebo A2 klesne, uzamkneme stav vybité baterie
  if (rx_telemetry_data.batt_A1 <= (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE)
  {
    low_batt_flag_rx = true;
  }
  
  if (rx_telemetry_data.batt_A2 <= (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE)
  {
    low_batt_flag_rx = true;
  }
  */

  // Pokud napětí přijímače A1 nebo A2 klesne, uzamkneme stav vybité baterie
  uint8_t rx_batt_limit = (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE;

  if (rx_telemetry_data.batt_A1 <= rx_batt_limit || rx_telemetry_data.batt_A2 <= rx_batt_limit)
  {
    low_batt_flag_rx = true;
  }
  
  //Serial.println(low_batt_flag_rx);
}
