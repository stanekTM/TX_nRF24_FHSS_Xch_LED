//***********************
// Měření napětí baterií
//***********************
void read_batt()
{
  // Pokud napětí vysílače klesne, uzamkneme stav vybité baterie
  if ((analogRead(PIN_BATTERY) <= (1023.0 / TX_BATTERY_VOLTAGE) * TX_MONITORED_VOLTAGE) || low_batt_flag_tx)
  {
    low_batt_flag_tx = true;
  }
  
  // Pokud napětí přijímače A1 nebo A2 klesne, uzamkneme stav vybité baterie
  if ((rx_telemetry_data.batt_A1 <= (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE) || low_batt_flag_rx)
  {
    low_batt_flag_rx = true;
  }
  
  if ((rx_telemetry_data.batt_A2 <= (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE) || low_batt_flag_rx)
  {
    low_batt_flag_rx = true;
  }
  
  /*
  // Kontrola napětí se provádí POUZE pokud máme aktivní signál (signal_loss_timeout je menší než 1 s)
  if (millis() - signal_loss_timeout <= 1000)
  {
    // Pokud napětí přijímače A1 nebo A2 klesne, uzamkneme stav vybité baterie
    low_batt_flag_rx = rx_telemetry_data.batt_A1 <= (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE || low_batt_flag_rx;
    low_batt_flag_rx = rx_telemetry_data.batt_A2 <= (255.0 / RX_BATTERY_VOLTAGE) * RX_MONITORED_VOLTAGE || low_batt_flag_rx;
  }
  else
  {
    low_batt_flag_rx = false; // Při úplné ztrátě signálu vypneme alarm baterie, protože začne blikat hlavní alarm signálu
  }
  */
  
  //Serial.println(low_batt_flag_rx);
}
