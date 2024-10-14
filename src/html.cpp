//
// html.cpp - all html code for bird cam
//
// Ben Slaghekke, 23 sep 2023 
// 07 05 2024 BSla add extra user note line
#include "html.h"

// The sequence of a web page is:
//  index_head
//  style
//  index_body1
//  site name
//  date and comment
//  index_body2
//  endHtml
//
// To see how this sequence is used, look at module httpsupp.cpp, function sendPage
//
// $KEY$ sequences are replaced by their values.
//


const char PROGMEM styleHead[] =
R"rawliteral(
<style>
   body    { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
   table   { margin-left: auto; margin-right: auto; }
   td      { padding: 8 px; }
 
   img {width: auto; max-width: 60%; height: auto; }
   h1    {font-family:verdana;}
   h2    {font-family:verdana;}
   h3    {font-family:verdana;}
   h4    {font-family:verdana;}
   h5    {font-family:verdana;}
   p     {font-family:verdana;}
   label {font-size:100%;font-family:verdana;}
   input {font-size:150%;font-family:verdana;}
   a     {font-size:200%;font-family:verdana;}
</style>
)rawliteral";



const char PROGMEM theHead[] = R"rawliteral(
<html>
<head>
<meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
<meta http-equiv="Pragma" content="no-cache" />
<meta http-equiv="Expires" content="0" />
<meta name="viewport" content="width=device-width, initial-scale=1">
$REFRESH$
)rawliteral";

const char PROGMEM startBody[] = R"rawliteral(
</head>
<body>
)rawliteral";

const char PROGMEM indexBody[] = R"rawliteral(
   <h2> Vergeet niet na gebruik de sluiter te sluiten </h2>
   <a href="page2"> Open de sluiter </a><br><br><br>
   <a href="siteinfo"> Stel site info in </a><br>
)rawliteral";

const char PROGMEM shutterOpenBody[] = R"rawliteral(
   <h2>Sluiter stond open</h2>   <p> Bij het vorige bezoek is de sluiter niet gesloten. Hierdoor vervuilt de cameralens. </p>
   <h2> Vergeet alstublieft niet, na gebruik de sluiter te sluiten! </h2>
   <a href="page2"> Bekijk camerabeeld </a><br><br><br><br>
   <a href="siteinfo"> Stel site info in </a><br>
)rawliteral";

const char PROGMEM page3Body[] = R"rawliteral(
   <h2>De sluiter is gesloten. U kunt de batterij veilig afkoppelen.</h2>
   <a href="page2"> Terug naar het camerabeeld </a><br><br><br>
   <a href="siteinfo"> Stel site info in </a><br>
)rawliteral";


const char PROGMEM siteInfoBody[] = R"rawliteral(
<form action="/siteinfo2" action=GET>
<br>
  <label for="sitename">Site naam:</label><br>
  <input type="text" id="sitename" name="sitename" value="$site$"><br>
<br>
<br>
  <label for="Comment">Commentaar:</label><br>
  <input type="text" id="comment" name="comment" value="$comment$"><br>
<br>
<h3>Als de camera moet inloggen op een Wifi netwerk, specificeer het volgende:</h3>
<br>
  <label for="SSID">SSID:</label><br>
  <input type="text" id="SSID" name="SSID" value="$SSID$">
<br> <br>
  <label for="Password">Wachtwoord:</label><br>
  <input type="text" id="Password" name="Password" value="$pass$"><br>
<br>
  <input type="submit" id="Exit" name="Exit" value="OK"> <sp> <sp> <sp> <sp> 
  <input type="submit" id="Exit" name="Exit" value="Cancel">
  <!--input type="submit" value="Servo scherm"-->
</form> 
)rawliteral";

const char PROGMEM rebootBody[] = R"rawliteral(
   <h2>De SSID of het wachtwoord is gewijzigd.</h2>
   <p style="text-align:left">De camera reboot nu en probeert in te loggen op het netwerk. 
       Controleer dat het WiFi access point is verdwenen. Als het access point na 1 minuut nog is verdwenen 
       log dan in op het ingestelde WiFi netwerk en zoek de camera. </p>
   <p style="text-align:left"> Als het WiFi access point nog bestaat, ga dan weer naar de configure page en probeer het opnieuw. </p>
)rawliteral";

//
// NOTE: this page is called http://1.2.3.4/page2/
// ****
// The script changes this URI into text   http://1.2.3.4:81/stream
// So the (0,-6) refers to the length of the page name excluding the /page2/
// Beware if you change the page name!
// 
const char PROGMEM page2Body[] = R"rawliteral(
    <img src="" id="photo" >
    <br>
    <a href="page3"> Sluit de sluiter </a>

   <script>
      window.onload = document.getElementById("photo").src = window.location.href.slice(0, -6) + ":81/stream";
   </script>
)rawliteral";

const char PROGMEM endHtml[] = R"rawliteral(
</body>
</html>
)rawliteral";

const char PROGMEM adjustHtml[] = R"rawliteral(
<!--adjust.html -->
<!DOCTYPE html>
<html>
<head>
<title>Birdcam servo adjust</title>
<link rel="stylesheet" href="styles.css">
</head>
<body>
<form action="/adjust2">
  <h3> Sluiter afstelling </h3>
  <h5>$SHUTTERPOS$</h5>
  <br>

  <label for="opos">Open positie (0..180):</label>
  <input type="number" id="openpos" name="openpos" min="0" max="180" value="$OPENPOS$" />
  <label for="opos">graden</label>
  <br>
  
  <label for="cpos">Sluit positie&nbsp (0..180):</label>
  <input type="number" id="clpos" name="clpos" min="0" max="180" value="$CLOSEDPOS$" />
  <label unit="cpos">graden</label>
  <br>  <br> 
  
  <input type="submit" id="Open"  name="Open"   value="Open">&nbsp&nbsp&nbsp&nbsp
  <input type="submit" id="Sluit" name ="Sluit" value="Sluit">
  <br> <br>  <br>
  
  <label for="spd">Snelheid (1..400):</label>
  <input type="number" id="speed" name="speed" min="1" max="400" value="$SPEED$" />
  <label unit="spd">gr/sec</label>
  <br><br>
  <p>Totaal aantal sluiter bewegingen totnutoe: $TOTALMOVES$</p>  <br>
  <br>
  <label for="ntimes">Beweeg de sluiter</label>
  <input type="number" id="ntimes" name="ntimes" min="0" max="1000" value="$MOVESLEFT$" />
  <label unit="ntimes">keer</label>
  <br><br>
  <input type="submit" id="Exit" name="Exit" value="Start bewegingen">&nbsp&nbsp&nbsp&nbsp
  <input type="submit" id="Exit" name="Exit" value="OK">&nbsp&nbsp&nbsp&nbsp
  <input type="submit" id="Exit" name="Exit" value="Cancel">
</form> 
</body>
)rawliteral";