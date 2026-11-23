//*********************
// Čtení potenciometrů
//*********************
void read_pots()
{
  for (uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    uint16_t raw_pots = analogRead(pot_pins[i]);
    
    // Výpočet mapování s ohledem na fyzický střed potenciometru páčky
    if (raw_pots < mid_pots_calib[i])
    {
      pots_value[i] = map(raw_pots, min_pots_calib[i], mid_pots_calib[i], MIN_CONTROL_VAL, MID_CONTROL_VAL);
    }
    else
    {
      pots_value[i] = map(raw_pots, mid_pots_calib[i], max_pots_calib[i], MID_CONTROL_VAL, MAX_CONTROL_VAL);
    }
    
    // Aplikace reverzace kanálu
    if (reverse[i] == 1) pots_value[i] = (MIN_CONTROL_VAL + MAX_CONTROL_VAL) - pots_value[i];
    
    // Ochrana proti šumu a hodnotám mimo rozsah 1000 až 2000
    tx_data.rc_data[i] = constrain(pots_value[i], MIN_CONTROL_VAL, MAX_CONTROL_VAL);
  }
}

//******************************************
// Kalibrovat nebo reverzovat potenciometry
//******************************************
void calibrate_reverse_pots()
{
  // Kalibrační smyčka při držení tlačítka
  while (digitalRead(PIN_BTN_CALIB_FS) == LOW)
  {
    calibrate = false;
    
    for (uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
    {
      uint16_t raw_pots = analogRead(pot_pins[i]);
      
      if (raw_pots < min_pots_calib[i]) min_pots_calib[i] = raw_pots;
      
      if (raw_pots > max_pots_calib[i]) max_pots_calib[i] = raw_pots;
      
      mid_pots_calib[i] = raw_pots;
    }
  }
  
  if (!calibrate)
  {
    for (uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
    {
      EEPROM.put(i * 6,     min_pots_calib[i]); // EEPROM locations 0,  6, 12, 18, 24, 30, 36
      EEPROM.put(i * 6 + 2, mid_pots_calib[i]); // EEPROM locations 2,  8, 14, 20, 26, 32, 38
      EEPROM.put(i * 6 + 4, max_pots_calib[i]); // EEPROM locations 4, 10, 16, 22, 28, 34, 40
    }
    calibrate = true;
  }
  
  // Načíst data z EEPROM do operační paměti RAM
  for (uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    EEPROM.get(i * 6,     min_pots_calib[i]); // EEPROM locations 0,  6, 12, 18, 24, 30, 36
    EEPROM.get(i * 6 + 2, mid_pots_calib[i]); // EEPROM locations 2,  8, 14, 20, 26, 32, 38
    EEPROM.get(i * 6 + 4, max_pots_calib[i]); // EEPROM locations 4, 10, 16, 22, 28, 34, 40
    
    reverse[i] = EEPROM.read(i + 42) & 1;     // EEPROM locations 42, 43, 44, 45, 46, 47, 48
  }
  
  // Kontrola reverzace pák při startu (krajní poloha)
  for (uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    pots_value[i] = map(analogRead(pot_pins[i]), min_pots_calib[i], max_pots_calib[i], MIN_CONTROL_VAL, MAX_CONTROL_VAL);
    
    if (pots_value[i] < MIN_CONTROL_VAL + 50 || pots_value[i] > MAX_CONTROL_VAL - 50)
    {
      reverse[i] ^= B00000001;
      EEPROM.update(i + 42, reverse[i]); // EEPROM locations 42, 43, 44, 45, 46, 47, 48
    }
  }
}
