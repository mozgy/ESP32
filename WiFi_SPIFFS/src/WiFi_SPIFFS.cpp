/*
 * 
 * Copyright (c) 2023. Mario Mikočević <mozgy>
 * 
 * MIT Licence
 *
 */

#define SW_VERSION "0.02.01"

#include <Arduino.h>
// #include "driver/rtc_io.h"
#include <LittleFS.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif
// #include <FS.h>
// #include <SPIFFS.h>

#include "credentials.h"
#include "mywifi.h"

// Serial printing ON/OFF
#define DEBUG true
// #define Serial if(DEBUG)Serial // this does NOT play nice with ArduinoJson
// #define DEBUG_OUTPUT Serial
#define DBG_OUTPUT_PORT Serial

// ADC_MODE(ADC_VCC); Darned Arduino IDE BUG

#define OTA_HOSTNAME "wifiscan-esp"

Ticker tickerWiFiScan;
#define WAITTIME 300
boolean tickerFired;

char elapsedTimeString[40];
char currentDateTime[17];

File fh_netdata;
String line;

//    {\"ssid\":\"ssid\",\"bssid\":\"bssid\",\"rssi\":0,\"ch\":1,\"enc\":\"*\"}
struct stationData {
  char stationSSID[32];
  char stationBSSID[18];
  int stationRSSI;
  int stationChannel;
  bool stationEncription;
};

const char* host     = "mozgy.t-com.hr";
const char* urlHost  = "192.168.44.10";

// #define WIFI_DISC_DELAY 30000L
// unsigned long wifiWaitTime;
// int wifiSTATries;

//#define WIFIDEBUG
#undef WIFIDEBUG
void flagWiFiScan( void ) {
  tickerFired = true;
}

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

String bssidToString( uint8_t *bssid ) {

  char mac[18] = {0};

  sprintf( mac,"%02X:%02X:%02X:%02X:%02X:%02X", bssid[0],  bssid[1],  bssid[2], bssid[3], bssid[4], bssid[5] );
  return String( mac );

}

void debugPrintJson( const char *msg, const JsonDocument& jsonData ) {

  DBG_OUTPUT_PORT.print( msg );
  serializeJson( jsonData, Serial );
  DBG_OUTPUT_PORT.println();

}

bool update_netdata( int netNum ) {

  int netId;
  int netFound = 0;

  DynamicJsonDocument WiFiDataFile(4200);

//    fh_netdata.println( "{\"count\":0,\"max\":0}" );
//    fh_netdata.println( "{\"count\":0,\"max\":0,\"networks\":[{\"ssid\":\"ssid\",\"bssid\":\"bssid\",\"rssi\":0,\"ch\":1,\"enc\":\"*\"}]}" );

// create new data from network list
  DynamicJsonDocument WiFiData(4000);
  WiFiData[ "count" ] = netNum;
  WiFiData[ "max" ] = netNum;

  JsonArray WiFiDataArray  = WiFiData.createNestedArray( "networks" );

  if ( !LittleFS.exists( "/netdata.txt" ) ) {
    DBG_OUTPUT_PORT.println( "Data file doesn't exist yet." );

    fh_netdata = LittleFS.open( "/netdata.txt", FILE_WRITE );
    if ( !fh_netdata ) {
      DBG_OUTPUT_PORT.println( "Data file creation failed" );
      return false;
    }
    for ( int i = 0; i < netNum; i++ ) {

      DynamicJsonDocument tmpObj(120);

      tmpObj[ "id" ] = i;
      tmpObj[ "ssid" ] = WiFi.SSID(i);
      tmpObj[ "bssid" ] = bssidToString( WiFi.BSSID(i) );
      tmpObj[ "rssi" ] = WiFi.RSSI(i);
      tmpObj[ "ch" ] = WiFi.channel(i);
#ifdef ESP8266
      tmpObj[ "enc" ] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
#endif
#ifdef ESP32
      tmpObj[ "enc" ] = ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
#endif

      debugPrintJson( "Add - ", tmpObj );

      WiFiDataArray.add( tmpObj );

    }

  } else {

    fh_netdata = LittleFS.open( "/netdata.txt", FILE_READ );
    // DBG_OUTPUT_PORT.println( "Reading saved wifi data .." );
    line = fh_netdata.readStringUntil('\n');
    DeserializationError error = deserializeJson( WiFiDataFile, line );
    // DBG_OUTPUT_PORT.print( "Line (read) " );DBG_OUTPUT_PORT.println( line );
    DBG_OUTPUT_PORT.println( "Previous scan -" );serializeJsonPretty( WiFiDataFile, Serial );DBG_OUTPUT_PORT.println();

    if ( error ) {
      DBG_OUTPUT_PORT.print( "parsing failed: " );
      DBG_OUTPUT_PORT.println(error.c_str());
      // parsing failed, removing old data
      fh_netdata.close();
      LittleFS.remove( "/netdata.txt" );
      return false;
    }

    int netNumFile = WiFiDataFile[ "count" ];
    int netMaxFile = WiFiDataFile[ "max" ];
    netId = netMaxFile;

    JsonArray tmpArray = WiFiDataFile[ "networks" ];

    for ( JsonVariant item : tmpArray ) {
      WiFiDataArray.add( item );
      // debugPrintJson( "Copy - ", item.as<JsonObject>() );
    }

    for ( int i = 0; i < netNum; i++ ) {
      bool wifiNetFound = false;
      bool strongerRSSI = false;
      String ssid1 = WiFi.SSID(i);
      String bssid1 = bssidToString( WiFi.BSSID(i) );
      int rssi1 = WiFi.RSSI(i);
      for ( int j = 0; j < netNumFile; j++ ) {
        String ssid2 = WiFiDataArray[j]["ssid"];
        if ( ssid1 == ssid2 ) {
          String bssid2 = WiFiDataArray[j]["bssid"];
          if ( bssid1 == bssid2 ) {
            wifiNetFound = true;
            String rssi2 = WiFiDataArray[j]["rssi"];
            DBG_OUTPUT_PORT.print( "Station - " );DBG_OUTPUT_PORT.print(ssid1);
            DBG_OUTPUT_PORT.print( ", scanned RSSI - " );DBG_OUTPUT_PORT.print(rssi1);
            DBG_OUTPUT_PORT.print( ", saved RSSI - " );DBG_OUTPUT_PORT.println(rssi2);
            if ( rssi1 > rssi2.toInt() ) {
              strongerRSSI = true;
              DBG_OUTPUT_PORT.println( "Station signal stronger - saving new data" );
              WiFiDataArray[j]["rssi"] = String( rssi1 );
            }
          }
        }
      }
      if ( !wifiNetFound ) {
        DynamicJsonDocument tmpObj(120);
        tmpObj[ "id" ] = netId;
        tmpObj[ "ssid" ] = WiFi.SSID(i);
        tmpObj[ "bssid" ] = bssidToString( WiFi.BSSID(i) );
        tmpObj[ "rssi" ] = WiFi.RSSI(i);
        tmpObj[ "ch" ] = WiFi.channel(i);
#ifdef ESP8266
        tmpObj[ "enc" ] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
#endif
#ifdef ESP32
        tmpObj[ "enc" ] = ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
#endif
        WiFiDataArray.add( tmpObj );
        debugPrintJson( "Found new - ", tmpObj );

        netFound++;
        netId++;
      }
    }

    WiFiData[ "count" ] = netNumFile + netFound;
    WiFiData[ "max" ] = netId;

//    Serial.println("Computed wifi data ->");
//    WiFiData.prettyPrintTo( Serial );

    fh_netdata.close();
    // LittleFS.remove( "/netdata.txt" );

    fh_netdata = LittleFS.open( "/netdata.txt", FILE_WRITE );
    if ( !fh_netdata ) {
      DBG_OUTPUT_PORT.println( "Data file creation failed" );
      return false;
    }

  }
  serializeJson( WiFiData, fh_netdata );
  fh_netdata.println( "\n" );
  fh_netdata.close();

  return true;

}

void parse_networks( int netNum ) {

#ifdef WIFIDEBUG
  for (int i = 0; i < netNum; ++i)
  {
    // Print SSID and RSSI for each network found
    DBG_OUTPUT_PORT.print(i + 1);
    DBG_OUTPUT_PORT.print(": ");
    DBG_OUTPUT_PORT.print(WiFi.SSID(i));
    DBG_OUTPUT_PORT.print(" (");
    DBG_OUTPUT_PORT.print(WiFi.RSSI(i));
    DBG_OUTPUT_PORT.print(")");
#ifdef ESP8266
    DBG_OUTPUT_PORT.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
#endif
#ifdef ESP32
    DBG_OUTPUT_PORT.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
#endif
    delay(10);
  }
#endif

 if ( !update_netdata( netNum ) ) {
   DBG_OUTPUT_PORT.println( "Something went WRONG!" );
 }

}

void do_wifiscan( void ) {
  int netCount;

  DBG_OUTPUT_PORT.print( "Scan start - " );

  // WiFi.scanNetworks will return the number of networks found
  netCount = WiFi.scanNetworks();
//  DBG_OUTPUT_PORT.println( "scan done" ); // no need if Serial.setDebugOutput(true)
  if ( netCount == 0 ) {
    DBG_OUTPUT_PORT.println( "no network found" );
  } else {
    DBG_OUTPUT_PORT.print(netCount);
    DBG_OUTPUT_PORT.println( " network(s) found" );
    parse_networks( netCount );
  }
  DBG_OUTPUT_PORT.println();

}

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

void setup() {

  DBG_OUTPUT_PORT.begin( 115200 );
  delay( 10 );
  DBG_OUTPUT_PORT.setDebugOutput( true );
//  WiFi.printDiag(Serial); // research this
  DBG_OUTPUT_PORT.println();

  prnEspStats();

  if( !LittleFS.begin( true ) ) {
    Serial.println( "Formatting LittleFS" );
    if( !LittleFS.begin( ) ) {
      Serial.println( "An Error has occurred while mounting LittleFS" );
    }
  }

  delay( 10 );
  initWiFi();
  wifiWaitTime = millis();

  initOTA();

  tickerWiFiScan.attach( WAITTIME, flagWiFiScan );
  tickerFired = true;

}

void loop() {

  ArduinoOTA.handle();

  if( tickerFired ) {
    tickerFired = false;
    fnElapsedStr( elapsedTimeString );
    DBG_OUTPUT_PORT.println( elapsedTimeString );
    do_wifiscan();
  }

}
