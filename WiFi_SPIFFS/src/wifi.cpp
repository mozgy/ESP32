//
// (c) mozgy
//
// MIT Licence
//

#include "mywifi.h"
#include "credentials.h"

unsigned long wifiWaitTime;
int wifiSTATries;
bool wifi5G = false;

/*
void waitForConnect( unsigned long timeout ) {

  unsigned long timeWiFi = millis();

  while( WiFi.status() != WL_CONNECTED ) {
    if( ( millis() - timeWiFi ) > timeout )
      break;
  }

}
  */

String get_wifi_status( int status ) {
  switch( status ) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
  }
  return "WL_NONE";
}

void WiFiStationConnected( WiFiEvent_t event, WiFiEventInfo_t info ) {

  Serial.println( "Connected to AP successfully!" );

}

void WiFiGotIP( WiFiEvent_t event, WiFiEventInfo_t info ) {

  Serial.print( "WiFi connected with IP address: " );
  // Serial.println( WiFi.localIP() );
  Serial.println( IPAddress( info.got_ip.ip_info.ip.addr ) );

}

void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info ) {

  Serial.println( "Disconnected from WiFi access point" );
  Serial.print( "WiFi lost connection. Reason: " );
  Serial.println( info.wifi_sta_disconnected.reason );
  Serial.println( "Reconnecting .." );
  // WiFi.disconnect( );
  // vTaskDelay( 4000 );
  WiFi.begin( ssid, password );

}

void initWiFi( void ) {

  wifiSTATries = 1;
  bool wifiNoSTA = false;

  WiFi.softAPdisconnect( true );
  WiFi.disconnect( true );
  WiFi.setMinSecurity( WIFI_AUTH_WPA_PSK );
  esp_wifi_set_storage( WIFI_STORAGE_RAM );
  // esp_wifi_set_storage( WIFI_STORAGE_FLASH );
  uint32_t brown_reg_tmp = READ_PERI_REG( RTC_CNTL_BROWN_OUT_REG );
  WRITE_PERI_REG( RTC_CNTL_BROWN_OUT_REG, 0 );
  WiFi.mode( WIFI_STA );
  WRITE_PERI_REG( RTC_CNTL_BROWN_OUT_REG, brown_reg_tmp );
  WiFi.onEvent( WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED );
  WiFi.onEvent( WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP );
  WiFi.onEvent( WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED );
  Serial.println( WiFi.macAddress() );
  // WiFi.setHostname( ESP_HOSTNAME );
  WiFi.begin( ssid, password );
  WiFi.setSleep( false );

  WiFi.scanNetworks( true );

}

// SDK definitions
// typedef enum {
//     WIFI_AUTH_OPEN = 0,         /**< authenticate mode : open */
//      WIFI_AUTH_WEP,              /**< authenticate mode : WEP */
//      WIFI_AUTH_WPA_PSK,          /**< authenticate mode : WPA_PSK */
//      WIFI_AUTH_WPA2_PSK,         /**< authenticate mode : WPA2_PSK */
//      WIFI_AUTH_WPA_WPA2_PSK,     /**< authenticate mode : WPA_WPA2_PSK */
//      WIFI_AUTH_WPA2_ENTERPRISE,  /**< authenticate mode : WPA2_ENTERPRISE */
//      WIFI_AUTH_WPA3_PSK,         /**< authenticate mode : WPA3_PSK */
//      WIFI_AUTH_WPA2_WPA3_PSK,    /**< authenticate mode : WPA2_WPA3_PSK */
//      WIFI_AUTH_WAPI_PSK,         /**< authenticate mode : WAPI_PSK */
//      WIFI_AUTH_MAX
// } wifi_auth_mode_t;

/*
  IPAddress camIP(192,168,1,199);
  IPAddress camDNS(192,168,1,1);
  IPAddress camGW(192,168,1,1);
  IPAddress camMask(255,255,255,0);
  WiFi.config( camIP, camGW, camMask, camDNS );
  WiFi.begin( ssid, password );
  */

/* OLD code 
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
  */
