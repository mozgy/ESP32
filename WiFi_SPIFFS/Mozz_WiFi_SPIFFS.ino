/*
 * 
 * Copyright (c) 2020. Mario Mikočević <mozgy>
 * 
 * MIT Licence
 *
 */

#define SW_VERSION "0.01.04"

#include <Arduino.h>

// Serial printing ON/OFF
#define DEBUG true
// #define Serial if(DEBUG)Serial // this does NOT play nice with ArduinoJson
// #define DEBUG_OUTPUT Serial
#define DBG_OUTPUT_PORT Serial

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif
#include <FS.h>
#include <SPIFFS.h>

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

// ADC_MODE(ADC_VCC); Darned Arduino IDE BUG

#include <ArduinoOTA.h>
#define OTA_HOSTNAME "wifiscan-esp"

#include <Ticker.h>
Ticker tickerWiFiScan;
#define WAITTIME 60
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

//#define WIFIDEBUG
#undef WIFIDEBUG
void flagWiFiScan( void ) {
  tickerFired = true;
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

  DynamicJsonDocument WiFiDataFile(1200);

//    fh_netdata.println( "{\"count\":0,\"max\":0}" );
//    fh_netdata.println( "{\"count\":0,\"max\":0,\"networks\":[{\"ssid\":\"ssid\",\"bssid\":\"bssid\",\"rssi\":0,\"ch\":1,\"enc\":\"*\"}]}" );

// create new data from network list
  DynamicJsonDocument WiFiData(400);
  WiFiData[ "count" ] = netNum;
  WiFiData[ "max" ] = netNum;

  JsonArray WiFiDataArray  = WiFiData.createNestedArray( "networks" );

  fh_netdata = SPIFFS.open( "/netdata.txt", "r" );

  if ( !fh_netdata ) {

// no last data
    DBG_OUTPUT_PORT.println( "Data file doesn't exist yet." );

    fh_netdata = SPIFFS.open( "/netdata.txt", "w" );
    if ( !fh_netdata ) {
      DBG_OUTPUT_PORT.println( "Data file creation failed" );
      return false;
    }
    for ( int i = 0; i < netNum; ++i ) {

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

      debugPrintJson( "Add - ", WiFiData );

      WiFiDataArray.add( tmpObj );

    }

  } else {

// read last WiFi data from file
    DBG_OUTPUT_PORT.println( "Reading saved wifi data .." );
    line = fh_netdata.readStringUntil('\n');
    DBG_OUTPUT_PORT.print( "Line (read) " );DBG_OUTPUT_PORT.println( line );

    DeserializationError error = deserializeJson( WiFiDataFile, line );
    if ( error ) {
      DBG_OUTPUT_PORT.print( "parsing failed: " );
      DBG_OUTPUT_PORT.println(error.c_str());
      // parsing failed, removing old data
      SPIFFS.remove( "/netdata.txt" );
      return false;
    }

    int netNumFile = WiFiDataFile[ "count" ];
    int netMaxFile = WiFiDataFile[ "max" ];
    netId = netMaxFile;

    JsonArray tmpArray = WiFiDataFile[ "networks" ];
//    for ( JsonArray::iterator it = tmpArray.begin(); it != tmpArray.end(); ++it ) {
//      JsonObject tmpObj = *it;
//      WiFiDataArray.add( tmpObj );
//      debugPrintJson( "Copy - ", &tmpObj );
//    }
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
              DBG_OUTPUT_PORT.println( "Station signal stronger - TODO - save new data" );
//              WiFiDataArray[j]["rssi"] = String( rssi1 );
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
    // SPIFFS.remove( "/netdata.txt" );

    fh_netdata = SPIFFS.open( "/netdata.txt", "w" );
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

bool update_netdata_test( int netNum ) {

  DynamicJsonDocument WiFiData(1000);
  JsonArray data = WiFiData.createNestedArray( "data" );

  for ( int i = 0; i < netNum; ++i ) {
    DynamicJsonDocument tmpObj(120);
 
    tmpObj["id"] = i;
    tmpObj["ssid"] = WiFi.SSID(i);
    tmpObj["bssid"] = bssidToString( WiFi.BSSID(i) );
    tmpObj["rssi"] = WiFi.RSSI(i);
    tmpObj["ch"] = WiFi.channel(i);
#ifdef ESP8266
    tmpObj["enc"] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
#endif
#ifdef ESP32
    tmpObj["enc"] = ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
#endif

//    serializeJson( tmpObj, Serial );
//    Serial.println();
//    serializeJsonPretty( tmpObj, Serial );
//    Serial.println();

    data.add( tmpObj );

  }

  serializeJson( WiFiData, Serial );
  Serial.println();

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

  DBG_OUTPUT_PORT.println( "scan start" );

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

void setup() {

  DBG_OUTPUT_PORT.begin( 115200 );
  delay( 10 );
  DBG_OUTPUT_PORT.setDebugOutput( true );
//  WiFi.printDiag(Serial); // research this
  DBG_OUTPUT_PORT.println();

  prnEspStats();

#ifdef OTA
  initOTA();
#endif

  bool result = SPIFFS.begin();
  if( !result ) {
    DBG_OUTPUT_PORT.println( "SPIFFS begin failed!" );
  }

//// comment format section after DEBUGING done
//  result = SPIFFS.format();
//  if( !result ) {
//    DBG_OUTPUT_PORT.println("SPIFFS format failed!");
//  }

  SPIFFS.remove( "/netdata.txt" );

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

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
