#ifndef _MYWEBSERVER_H_
#define _MYWEBSERVER_H_

#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer asyncWebServer;

void asyncHandleRoot( AsyncWebServerRequest *request );
void asyncHandleStatistics( AsyncWebServerRequest *request );
void asyncHandleSetup( AsyncWebServerRequest *request );
void asyncHandleFullSetup( AsyncWebServerRequest *request );
void asyncHandleLogin( AsyncWebServerRequest *request );
void asyncHandleNotFound( AsyncWebServerRequest *request );
void listDirectory( String path, AsyncWebServerRequest *request );
bool loadFromSDCard( AsyncWebServerRequest *request );
void initAsyncWebServer( void );
void doSnapSavePhoto( void );

String getHTMLRootText( void );
String getHTMLStatisticsText( void );
String getHTMLSetupText( void );
String getHTMLFullSetupText( void );
String getCameraStatus( void );

void fnElapsedStr( char *str );
void initSDCard( void );
extern bool SDCardOK;

extern void reconfigureCamera( void );
extern void fnSetFrameSize( String frameSize );

#endif
