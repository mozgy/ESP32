/*
 * 
 * Copyright (c) 2020. Mario Mikočević <mozgy>
 * 
 * MIT Licence
 *
 */

void prnEspStats( void ) {

  uint64_t chipid;

  DBG_OUTPUT_PORT.println();
  DBG_OUTPUT_PORT.printf( "Sketch SW version: %s\n", SW_VERSION );
  DBG_OUTPUT_PORT.print( "Last Reset Reason: " );
//  DBG_OUTPUT_PORT.println( ESP.getResetReason() );
//  DBG_OUTPUT_PORT.printf( "Boot Mode / Vers: %u / %u\n", ESP.getBootMode(), ESP.getBootVersion() );
  DBG_OUTPUT_PORT.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  DBG_OUTPUT_PORT.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  DBG_OUTPUT_PORT.printf( "Heap: %u\n", ESP.getFreeHeap() );
  DBG_OUTPUT_PORT.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
#ifdef ESP8266
  DBG_OUTPUT_PORT.printf( "Chip ID: %u\n", ESP.getChipId() );
#endif
#ifdef ESP32
  chipid=ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  DBG_OUTPUT_PORT.printf( "ESP32 Chip ID = %04X", (uint16_t)(chipid>>32) ); //print High 2 bytes
  DBG_OUTPUT_PORT.printf( "%08X\n", (uint32_t)chipid );                     //print Low 4bytes.
  DBG_OUTPUT_PORT.printf( "Chip Revision: %u\n", ESP.getChipRevision() );
#endif
  DBG_OUTPUT_PORT.printf( "SDK: %s\n", ESP.getSdkVersion() );
  DBG_OUTPUT_PORT.printf( "Arduino: %d\n", ARDUINO );
//  DBG_OUTPUT_PORT.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  DBG_OUTPUT_PORT.printf( "Flash Size: %u\n", ESP.getFlashChipSize() );
  DBG_OUTPUT_PORT.printf( "Flash Speed: %u\n", ESP.getFlashChipSpeed() );
//  DBG_OUTPUT_PORT.printf( "PSRAM Size: %u\n", ESP.getPsramSize() );
#ifdef ESP8266
  DBG_OUTPUT_PORT.printf( "Vcc: %u\n", ESP.getVcc() );
#endif
  DBG_OUTPUT_PORT.println();

/*
    Serial.print("ESP32 SDK: "); Serial.println(ESP.getSdkVersion());
    Serial.print("ESP32 CPU FREQ: "); Serial.print(getCpuFrequencyMhz()); Serial.println("MHz");
    Serial.print("ESP32 APB FREQ: "); Serial.print(getApbFrequency() / 1000000.0, 1); Serial.println("MHz");
    Serial.print("ESP32 FLASH SIZE: "); Serial.print(ESP.getFlashChipSize() / (1024.0 * 1024), 2); Serial.println("MB");
    Serial.print("ESP32 RAM SIZE: "); Serial.print(ESP.getHeapSize() / 1024.0, 2); Serial.println("KB");
    Serial.print("ESP32 FREE RAM: "); Serial.print(ESP.getFreeHeap() / 1024.0, 2); Serial.println("KB");
    Serial.print("ESP32 MAX RAM ALLOC: "); Serial.print(ESP.getMaxAllocHeap() / 1024.0, 2); Serial.println("KB");
    Serial.print("ESP32 FREE PSRAM: "); Serial.print(ESP.getFreePsram() / 1024.0, 2); Serial.println("KB");
 */

}

void fnElapsedStr( char *str ) {

  unsigned long sec, minute, hour;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  sprintf( str, "Elapsed " );
  if ( hour == 0 ) {
    sprintf( str, "%s   ", str );
  } else {
    sprintf( str, "%s%2d:", str, hour );
  }
  if ( minute >= 10 ) {
    sprintf( str, "%s%2d:", str, minute );
  } else {
    if ( hour != 0 ) {
      sprintf( str, "%s0%1d:", str, minute );
    } else {
      sprintf( str, "%s ", str );
      if ( minute == 0 ) {
        sprintf( str, "%s  ", str );
      } else {
        sprintf( str, "%s%1d:", str, minute );
      }
    }
  }
  if ( ( sec % 60 ) < 10 ) {
    sprintf( str, "%s0%1d", str, ( sec % 60 ) );
  } else {
    sprintf( str, "%s%2d", str, ( sec % 60 ) );
  }

}

void initWiFi( void ) {

  uint8_t connAttempts = 0;
  uint8_t connAttemptsMAX = 5;

  DBG_OUTPUT_PORT.print( "Connecting to " );
  DBG_OUTPUT_PORT.println( ssid );
  WiFi.mode( WIFI_STA );
  WiFi.begin( );
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ){
    WiFi.begin( ssid, password );
    delay(500);
    connAttempts++;
//    DBG_OUTPUT_PORT.println( "Retrying connection..." ); // if connAttempts > 1
    if ( connAttempts > connAttemptsMAX ) {
#ifdef DEEPSLEEP
      DBG_OUTPUT_PORT.println( "Connection Failed! Gonna ..zzZZ" );
      ESP.deepSleep( sleepTimeSec * 1000000, RF_NO_CAL );
#endif
      // if deepsleep is not defined we just reboot
      // TODO - after MAX attempts create AP for ESP Setup over OTA
      delay(5000);
      ESP.restart();
    }
  }
  DBG_OUTPUT_PORT.println( "WiFi connected" );
  DBG_OUTPUT_PORT.print( "IP address: " );
  DBG_OUTPUT_PORT.println( WiFi.localIP() );

}

#ifdef OTA
void initOTA( void ) {

  // Port defaults to 8266
  // ArduinoOTA.setPort( OTA_PORT );

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname( OTA_HOSTNAME );

  // ArduinoOTA.setPassword((const char *)"xxx");
  // ArduinoOTA.setPassword( OTA_PASSWORD );

  ArduinoOTA.onStart([]() {
    DBG_OUTPUT_PORT.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    DBG_OUTPUT_PORT.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//    DBG_OUTPUT_PORT.printf( "Progress: %u%%\n", (progress / (total / 100)) );
    static uint8_t done = 0;
    uint8_t percent = (progress / (total / 100) );
    if ( percent % 2 == 0  && percent != done ) {
      DBG_OUTPUT_PORT.print("#");
      done = percent;
    }
  if ( percent == 100 ) {
      DBG_OUTPUT_PORT.println();
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DBG_OUTPUT_PORT.printf( "Error[%u]: ", error );
    switch ( error ) {
      case OTA_AUTH_ERROR:
        DBG_OUTPUT_PORT.println("Auth Failed");
        break;
      case OTA_BEGIN_ERROR:
        DBG_OUTPUT_PORT.println("Begin Failed");
        break;
      case OTA_CONNECT_ERROR:
        DBG_OUTPUT_PORT.println("Connect Failed");
        break;
      case OTA_RECEIVE_ERROR:
        DBG_OUTPUT_PORT.println("Receive Failed");
        break;
      case OTA_END_ERROR:
        DBG_OUTPUT_PORT.println("End Failed");
        break;
      default:
        DBG_OUTPUT_PORT.println("OTA Error");
    }
  });
  ArduinoOTA.begin();

}
#endif
