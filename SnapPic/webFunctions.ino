//
// (c) mozgy
//
// MIT Licence
//

// In Progress ..
String fnOptionVGA( char *str1, char *str2 ) {

  String webText;

  webText = "  <option value='";
  webText += String( str1 ) + "'";
  if( String( foo[picSnapSize] ) == String ( str1 ) ) {
    webText += "selected";
  }
  webText += ">" + String( str2 ) + "</option>";

  return webText;

}

String getHTMLRootText( void ) {

  String webText;

  webText = "<!DOCTYPE html><html>";
  webText += "<head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'>";
  webText += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<style>body { font-family: 'Comic Sans MS', cursive, sans-serif; }</style>";
  webText += "</head><body>";
  webText += "AI-Cam-" + String( AI_CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  webText += "<p><a href=/stats>Statistics</a>";
  webText += "<p><a href=/setup>Setup</a>";
  webText += "<p><a href=/snaps>Pictures</a>";
  webText += "</body>";
  webText += "</html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLStatisticsText( void ) {

  String webText;
  char tmpStr[20];

  webText = "<!DOCTYPE html><html>";
  webText += "<head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'>";
  webText += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<style>body { font-family: 'Comic Sans MS', cursive, sans-serif; }</style>";
  webText += "</head><body>";
  webText += "AI-Cam-" + String( AI_CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  fnElapsedStr( elapsedTimeString );
  webText += String( elapsedTimeString ) + "<br>";
  sprintf( tmpStr, "Used space %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );
  webText += String( tmpStr );
  webText += "<br>Time Period " + String( waitTime );
  webText += "</body>";
  webText += "</html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLSetupText( void ) {

  String webText;

  webText = "<!DOCTYPE html><html>";
  webText += "<head><link rel='stylesheet' type='text/css' href='onoffswitch.css'></head>";
  webText += "<body>Camera Setup";
  webText += "<p><a href=/startap>TestAP</a>";
  webText += "<p><form action='/set'><div class='onoffswitch'>";
  webText += "<input type='checkbox' name='onoffswitch' class='onoffswitch-checkbox' id='myonoffswitch'";
  if( flashEnable )
    webText += " checked";
  webText += ">";
  webText += "<label class='onoffswitch-label' for='myonoffswitch'>";
  webText += "<span class='onoffswitch-inner'></span><span class='onoffswitch-switch'></span>";
  webText += "</label>";
  webText += "</div>";
  webText += "<p>Picture Size - <select name='picSize'>";
  webText += " <option value='FRAMESIZE_QVGA'>320x240</option>";
  webText += " <option value='FRAMESIZE_VGA'>640x480</option>";
  webText += " <option value='FRAMESIZE_SVGA'>800x600</option>";
  webText += " <option value='FRAMESIZE_XGA' selected>1024x768</option>";
  webText += " <option value='FRAMESIZE_SXGA'>1280x1024</option>";
  webText += " <option value='FRAMESIZE_UXGA'>1600x1200</option>";
  webText += " <option value='FRAMESIZE_QXGA'>2048x1536</option>";
  webText += "</select>";
  webText += "<p><div>";
  webText += "<label for='timePeriod'>Time Period - </label>";
  webText += "<input id='timePeriod' type='number' name='timePeriod' min='10' max='600' step='10' value='";
  webText += String( waitTime );
  webText += "'>";
  webText += "<input type='submit' value='Set'>";
  webText += "</div>";
  webText += "</form>";
  webText += "</body>";
  webText += "</html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLTFootText( int numPic ) {

  String webText;

  webText = "<div class='tableCam-foot'><table><tfoot><tr><th colspan='2'>Number of entries - " + String( numPic ) + "</th></tr>";
  webText += "<tr><th colspan='2'>Back to <a href='/'>TOP</a></th></tr></tfoot></table></div>";

  return webText;

}

/*
<!DOCTYPE html>
<html>
 <head>
  <title>Cam " + String( AI_CAM_SERIAL ) + "</title>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <link rel='stylesheet' type='text/css' href='mozz.css'>
 </head>
 <body>
  <div class='limiter'>
   <div class='container-tableCam'>
    <div class='wrap-tableCam'>
     <div class='tableCam'>
      <div class='tableCam-body'>
       <table>
        <tbody>

         <tr>
          <td class='co1'><a href='" + linkName + "'>" + linkName + "</a></td>
          <td class='co2'><a href='/delete?FILENAME=" + linkName + "'>X</a></td>
          or
          <td class='co2'>DIR</td>
         </tr>

        </tbody>
       </table>
      </div>
      <div class='tableCam-foot'>
       <table>
        <tfoot>
         <tr>
          <th colspan='2'>Number of entries - " + String( numPic ) + "</th>
         </tr>
        </tfoot>
       </table>
      </div> // tableCam-foot
     </div>  // tableCam
    </div>   // wrap-tableCam
   </div>    // container-tableCam
  </div>     // limiter
 </body>
</html>
 */

String listDirectoryAsString( File path ) {

  String linkName;
  String webText;
  int numPic = 0;

  webText = "<!DOCTYPE html><html><head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<link rel='stylesheet' type='text/css' href='mozz.css'></head>";
  webText += "<body><div class='limiter'><div class='container-tableCam'><div class='wrap-tableCam'>";
  webText += "<div class='tableCam'><div class='tableCam-body'><table><tbody>";
  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      webText += "<tr><td class='co1'><a href='" + linkName + "'>" + linkName + "</a></td>";
      webText += "<td class='co2'>";
      if( linkName.endsWith( ".jpg" ) ) {
        webText += "<a href='/delete?FILENAME=" + linkName + "'>X</a>";
      } else {
        webText += "DIR";
      }
      webText += "</td></tr>";
      file.close();
      file = path.openNextFile();
//      DBG_OUTPUT_PORT.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPic++;
    }
  }
  webText += "</tbody></table></div>";
  webText += getHTMLTFootText( numPic );
  webText += "</div></div></div></div></body></html>";
  return webText;

}

String listDirectoryAsJSON( File path ) {

  String linkName;
  int numPic = 0;
  String strJSON = "[";
//  listjson_t table;

  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      if( strJSON != "[" ) {
        strJSON += ',';
      }
      strJSON += "{\"type\":\"";
      strJSON += ( file.isDirectory() ) ? "dir" : "file";
      strJSON += "\",\"name\":\"";
      strJSON += String( file.name() ).substring(1);
      strJSON += "\"}";
      file.close();
      file = path.openNextFile();
//      DBG_OUTPUT_PORT.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPic++;
    }
  }
  strJSON += "]";
  return strJSON;

//  table = fnJSONList( path );

}
