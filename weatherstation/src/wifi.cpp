//
// using ideas from:
// https://github.com/sstaub/NTP
// https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino
//
// When calling configtime with NTP server name (like "pool.ntp.org"), make sure you have a valid DNS settings
// A valid DNS will be available if you started the network connection with default parameters (like Wifi.begin(ssid,sspass)), 
// but will not be available in case you started your network connection with WiFi.config(IP,gateway,subnet) (used to get fix IP)
// without giving it valid DNS -> WiFi.config(IP,GW,SUB,DNS1,DNS2)!!!
//
// MUST use version 1.0: arduino-libraries/Arduino_JSON @ ^0.1.0
//

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_Json.h>
#include <TinyXML.h>

const char ssid[]      = "<your ssid>";
const char password[]  = "<pwd>";
const char ntpServer[] = "pool.ntp.org";
const char hostname[]  = "ESP32-Nimo";
const char tzinfo[]    = "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";   // Western European Time

String openWeatherMapApiKey = "<api key";            // your own id, has to be requested from openweathermap
String openWeatherMapcityid = "";
//String openWeatherMapcityids[] = {"2911288", "524894", "8224785"};    //  "Hamburg: 2911288";  // Moscow: 524894 Mumbai: 1275339, Waterloo: 8224785
// some city id's from openWeatherMap website

//String openWeatherMapcityids[] = {"258543","258569","258575","282342","288131","283180","2911288","1275339","281997"};
String openWeatherMapcityids[] = {"2911288","3169272","2967096","2909313","2909325","2909335","2909344","2909347","2909350","2909356","2909364","2909386","2909402","2909418","2909430","2909433","2909437","2909444","2909445","2909460","2909461","2909489","2909491","2909497","2909544","2909587","2909601","2909615","2909618","2909623","2909636","2909695","2909696","2909755","2909756","2909761","2909762","2909764","2909781","2909821","2909865","2909873","2909877","2909880","2909884","2909886","2909892","2909908","2909912","2909915","2909916","2909926","2909941","2909944","2909968","2909969","2909973","2910037","2910046","2910047","2910048","2910049","2910051","2910055","2910068","2910099","2910108","2910121","2910135","2910140","2910141","2910146","2910167","2910168","2910171","2910172","2910178","2910181","2910231","2910269","2910271","2910273","2910275","2910278","2910280","2910282","2910283","2910289","2910292","2910302","2910310","2910324","2910342","2910344","2910363","2910381","2910384","2910385","2910390","2910391","2910403","2910407","2910410","2910419","2910451","2910456","2910463","2910469","2910474","2910483","2910484","2910488","2910493","2910514","2910552","2910560","2910575","2910624","2910633","2910638","2910639","2910648","2910649","2910652","2910663","2910667","2910675","2910684","2910685","2910686","2910694","2910706","2910709","2910711","2910720","2910724","2910748","2910760","2910761","2910762","2910776","2910777","2910791","2910814","2910831","2910863","2910875","2910881","2910898","2910900","2910902","2910921","2910935","2910949","2910966","2910971","2910973","2910985","2910986","2910992","2911007","2911010","2911013","2911044","2911051","2911072","2911073","2911074","2911077","2911082","2911087","2911099","2911144","2911145","2911167","2911173","2911189","2911214","2911218","2911230","2911233","2911240","2911242","2911243","2911246","2911250","2911251","2911257","2911259","2911260","2911266","2911271","2911275","2911276","2911282","2911283","2911285","2911287","2911288","2911293","2911296","2911297","2911298","2911300","2911301","2911306","2911308","2911314","2911318","2911322","2911325","2911360","2911362","2911364","2911369","2911379","2911384","2911389","2911395","2911404","2911408","2911419","2911423","2911426","2911443","2911461","2911463","2911464","2911483","2911491","2911514","2911520","2911521","2911522","2911530","2911533","2911544","2911556","2911561","2911566","2911575","2911579","2911580","2911584","2911631","2911634","2911640","2911641","2911648","2911664","2911665","2911667","2911682","2911683","2911691","2911695","2911700","2911725","2911729","2911739","2911760","2911765","2911767","2911769","2911771","2911775","2911794","2911795","2911813","2911825","2911831","2911833","2911834","2911855","2911909","2911910","2911911","2911916","2911917","2911935","2911938","2911940","2911953","2911955","2911960","2911964","2911965","2911983","2911987","2912018","2912053","2912106","2912120","2912122","2912158","2912159","2912160","2912167","2912168","2912177","2912184","2912195","2912236","2912266","2912272","2912309","2912312","2912319","2912320","2912347","2912351","2912357","2912367","2912368","2912386","2912392","2912400","2912405","2912417","2912448","2912457","2912481","2912495","2912496","2912509","2912533","2912534","2912539","2912541","2912547","2912573","2912577","2912594","2912604","2912605","2912618","2912621","2912646","2912649","2912681","2912711","2912744","2912745","2912753","2912758","2912782","2912790","2912801","2912807","2912810","2912811","2912820","2912826","2912829","2912834","2912850","2912858","2912863","2912921","2912932","2912938","2912943","2912948","2912961","2912966","2912983","2913000","2913025","2913046","2913050","2913064","2913069","2913153","2913177","2913192","2913195","2913200","2913210","2913221","2913224","2913225","2913229","2913233","2913244","2913274","2913275","2913279","2913280","2913286","2913290","2913292","2913296","2913297","2913303","2913328","2913335","2913357","2913366","2913375","2913376","2913381","2913384","2913385","2913387","2913394","2913395","2913405","2913406","2913408","2913414","2913417","2913421","2913425","2913428","2913431","2913433","2913437","2913445","2913446","2913449","2913452","2913453","2913461","2913466","2913472","2913478","2913480","2913492","2913525","2913532","2913535","2913537","2913543","2913555","2913558","2913565","2913571","2913587","2913592","2913598","2913600","2913609","2913625","2913626","2913627","2913631","2913657","2913660","2913661","2913673","2913675","2913676","2913677","2913679","2913681","2913687","2913689","2913692","2913693","2913694","2913701","2913708","2913711","2913713","2913719","2913721","2913731","2913737","2913738","2913743","2913745","2913752","2913761","2913773","2913782","2913783","2913788","2913797","2913803","2913814","2913818","2913825","2913838","2913841","2913847","2913848","2913855","2913880","2913884","2913888","2913890","2913891","2913892","2913905","2913906","2913913","2913915","2913918","2913922","2913942","2913972","2914035","2914057","2914064","2914101","2914105","2914121","2914122","2914125","2914137","2914144","2914146","2914159","2914169","2914177","2914186","2914188","2914191","2914208","2914215","2914252","2914263","2914275","2914279","2914292","2914304","2914309","2914316","2914347","2914426","2914428","2914432","2914457","2914465","2914522","2914543","2914548","2914560","2914562","2914642","2914655","2914657","2914660","2914661","2914689","2914741","2914746","2914756","2914769","2914789","2914795","2914796","2914800","2914801","2914806","2914809","2914816","2914828","2914833","2914836","2914848","2914856","2914867","2914872","2914875","2914876","2914878","2914882","2914890","2914902","2914923","2914925","2914929","2914932","2914934","2914935","2914940","2914949","2914950","2914974","2914975","2914999","2915004","2915006","2915013","2915024","2915028","2915032","2915035","2915044","2915045","2915046","2915048","2915049","2915051","2915052","2915055","2915067","2915075","2915076","2915081","2915087","2915090","2915093","2915094","2915095","2915107","2915111","2915116","2915118","2915121","2915124","2915128","2915129","2915130","2915131","2915134","2915137","2915143","2915157","2915163","2915164","2915166","2915168","2915172","2915173","2915176","2915178","2915187","2915191","2915192","2915196","2915201","2915202","2915203","2915209","2915210","2915212","2915213","2915219","2915223","2915224","2915226","2915231","2915233","2915236","2915238","2915240","2915241","2915252","2915254","2915255","2915256","2915277","2915283","2915294","2915298","2915303","2915306","2915317","2915318","2915321","2915337","2915343","2915349","2915353","2915354","2915357","2915358","2915362","2915364","2915367","2915372","2915375","2915376","2915382","2915385","2915388","2915391","2915392","2915398","2915410","2915417","2915427","2915430","2915432","2915434","2915438","2915439","2915443","2915447","2915459","2915493","2915499","2915511","2915513","2915515","2915520","2915523","2915529","2915536","2915540","2915550","2915552","2915554","2915556","2915563","2915570","2915574","2915578","2915579","2915581","2915592","2915595","2915597","2915601","2915604","2915608","2915613","2915620","2915641","2915653","2915673","2916224","2916290","2916567","2916570","2916581","2916584","2916585","2916586","2916593","2916596","2916597","2916604","2916606","2916611","2916627","2916630","2916631","2916636","2916643","2916644","2916653","2916655","2916657","2916659","2916666","2916676","2916689","2916696","2916728","2916731","2916839","2916840","2916846","2916851","2916859","2916860","2916861","2916862","2916864","2916866","2916879","2916889","2916893","2916896","2916903","2916915","2916923","2916926","2916936","2916937","2916942","2916949","2916950","2916954","2916959","2916963","2916966","2916972","2916974","2916979","2916986","2916996","2917002","2917006","2917011","2917013","2917014","2917019","2917046","2917047","2917053","2917060","2917067","2917084","2917098","2917102","2917106","2917107","2917126"};
String URL = "http://api.openweathermap.org/data/2.5/weather?appid=" + openWeatherMapApiKey + "&id=";

String httpGETRequest(const char* uri);
String jsonBuffer;
String sNtpMessage;

// weather data structure
struct weatherData {
  double tempMin;
  double tempMax;
  double temp;
  double feelslike;
  int    humidity;
  int    pressure;
  double windspeed;
  int    winddeg;
  String cloudsdesc;
  long   sunrise;
  long   sunset;
  String station;
};

void  NTP_printLocalTime(int offset, int mode);
char* asc_time(const struct tm *timeptr, char mode);
void  WeatherReport(void);
char* degree2cardinal(int degree);
String align(String in);

extern char SERDEBUG;
extern const char ROWSIZE  = 16;
extern char MessageBuffer[160];
extern void DisplayMessage(String Message, int startPos);
extern void DisplayClear();
extern String utf8toIso1(String utf8string);



void WiFi_init() {
  char tmpbuff[40];
  char retry=0;

  sprintf(tmpbuff, "     MC6205      Weatherstation");
  DisplayMessage(tmpbuff, 0);
  sprintf(tmpbuff, " Connecting to:");
  DisplayMessage(tmpbuff, 48);
  sprintf(tmpbuff, " %s", ssid);
  DisplayMessage(tmpbuff, 80+int((15-sizeof(ssid))/2));

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // workaround hostname setup (https://github.com/espressif/arduino-esp32/issues/2537)
  WiFi.begin(ssid, password);
  delay(2000);
  
  while (WiFi.status() != WL_CONNECTED) {
    if(retry++ > 8) {
      DisplayClear();
      delay(200);
      sprintf(tmpbuff, "retry connecting     to:            %s", ssid);
      DisplayMessage(tmpbuff, ROWSIZE);
      delay(800);
      WiFi.begin(ssid, password);
      retry=0;
    }
    delay(500);
    //DisplayClear();
  }

  DisplayClear();
  WiFi.setHostname(hostname);
  sprintf(tmpbuff, "-WiFi connected-");
  DisplayMessage(tmpbuff, 0);
  
  sprintf(tmpbuff, "   Hostname: ");
  DisplayMessage(tmpbuff, 32);

  sprintf(tmpbuff, "%s", WiFi.getHostname());
  DisplayMessage(tmpbuff, 51);

  sNtpMessage = "IP:";
  sNtpMessage+= WiFi.localIP().toString();
  sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
  DisplayMessage(MessageBuffer, 80);

  // init NTP and get the time
  configTzTime(tzinfo, ntpServer);
  NTP_printLocalTime(113,0);

  //disconnect WiFi
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);

  delay(3000);
  DisplayClear();
}


//
// the weather-report 
//
void WeatherReport(void)
{
  struct weatherData Weather;
  char tmpbuff[32];
  static char idc;
  int ID;
  
  if(idc == 0)
    ID=0;  // first entry in array (used for hometown)
  else 
    ID = random(200)+random(100);
  
  if(idc++ >= 3)
    idc=0;
  
  String URI = URL+openWeatherMapcityids[ID];

  if(WiFi.status() == WL_CONNECTED) 
  {
    struct tm timeinfo;

    getLocalTime(&timeinfo);
  
    jsonBuffer = httpGETRequest(URI.c_str());
    jsonBuffer.replace('[', ' ');   // remove unwanted json content
    jsonBuffer.replace(']', ' ');

    if(SERDEBUG==1) {
      Serial.print("URI:");
      Serial.println(URI);
      Serial.println();
      Serial.println(jsonBuffer);
    }

    JSONVar root = JSON.parse(jsonBuffer);
    if (JSON.typeof(root) == "undefined") {
      if(SERDEBUG==1) 
        Serial.println("Parsing input failed!");
      return;
    }

    Weather.tempMin    = root["main"]["temp_min"];
    Weather.tempMax    = root["main"]["temp_max"];
    Weather.temp       = root["main"]["temp"];
    Weather.feelslike  = root["main"]["feels_like"];
    Weather.pressure   = root["main"]["pressure"];
    Weather.humidity   = root["main"]["humidity"];
    Weather.cloudsdesc = root["weather"]["description"];
    Weather.windspeed  = root["wind"]["speed"];
    Weather.winddeg    = root["wind"]["deg"];
    Weather.station    = root["name"];

    sNtpMessage = Weather.station;
    sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
    int mlen = (ROWSIZE-sNtpMessage.length())/2;
    if(mlen > ROWSIZE) mlen = 0;
    DisplayMessage(MessageBuffer, mlen);

    sprintf(tmpbuff, "Temp: %.1f %.1f%c", Weather.temp-273.15, Weather.feelslike-273.15, 152);
    DisplayMessage(tmpbuff, 32);

    sprintf(tmpbuff, "Humidity: %d%%", Weather.humidity);
    DisplayMessage(align(String(tmpbuff)), 48);

    sprintf(tmpbuff, "Air: %d hPa", Weather.pressure);
    DisplayMessage(align(String(tmpbuff)), 64);

    sprintf(tmpbuff, "Wind: %d%c %s", Weather.winddeg, 152, degree2cardinal(Weather.winddeg));  // 152 == °
    DisplayMessage(align(String(tmpbuff)), 80);

    sprintf(tmpbuff, "Speed: %.1f m/s", Weather.windspeed);
    DisplayMessage(align(String(tmpbuff)), 96);

    // weather description
    char wlen=0;
    sNtpMessage = Weather.cloudsdesc;
    sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
    sNtpMessage.length() > ROWSIZE ? wlen=0 : wlen = int((ROWSIZE-sNtpMessage.length())/2);
    DisplayMessage(MessageBuffer, 112+wlen);

    // display time and date
    strcpy(tmpbuff,asc_time(&timeinfo,3));  // time
    strcat(tmpbuff, " ");
    strcat(tmpbuff, asc_time(&timeinfo,2)); // shortdate
    DisplayMessage(tmpbuff, 144);


    if(SERDEBUG==1) {
      Serial.print("JSON: ");
      Serial.println(root);
      Serial.print("Temperature: ");
      Serial.println(root["main"]["temp"]);
      Serial.print("Pressure: ");
      Serial.println(root["main"]["pressure"]);
      Serial.print("Humidity: ");
      Serial.println(root["main"]["humidity"]);
      Serial.print("Wind Speed: ");
      Serial.println(root["wind"]["speed"]);
      Serial.print("Weather: ");
      Serial.println(root["weather"]["description"]);
      Serial.print("Station: ");
      Serial.println(root["name"]);
      Serial.print("WeatherId: ");
      Serial.println(root["weather"]["id"]);
    }
  }
}


//
// http get
//
String httpGETRequest(const char *uri)
{
  HTTPClient http;
    
  // request the URI
  http.begin(uri);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}";
  
  if (httpResponseCode>0) {
    payload = http.getString();
  }
  else {
    DisplayClear();
    sNtpMessage = "   HTTP Error      Code: ";
    sNtpMessage += httpResponseCode;
    sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
    DisplayMessage(MessageBuffer, 80);
    delay(3000);
    DisplayClear();
  }
  // Free resources
  http.end();

  return utf8toIso1(payload);
}


void NTP_printLocalTime(int offset, int mode)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    sNtpMessage = "Failed to obtain ntptime!";
    sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
    DisplayMessage(MessageBuffer, offset);
    delay(1000);
    return;
  }

  sNtpMessage = asc_time(&timeinfo, 0);
  sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
  DisplayMessage(MessageBuffer, offset);

  if(mode == 0) {
    sNtpMessage = asc_time(&timeinfo, 1);
    sNtpMessage.toCharArray(MessageBuffer, sizeof(MessageBuffer));
    DisplayMessage(MessageBuffer, 19+offset);
  }
}


//
// date & time routine
//
char *asc_time(const struct tm *timeptr, char mode)
{
  const char wday_name[][4] = {
    "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"
  };
  const char mon_name[][4] = {
    "Jan", "Feb", "Mar", "Apr", "Mai", "Jun",
    "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"
  };

  static char time_buff[20] = {};

  // date full dayname
  if(mode == 0) {
    sprintf(time_buff, "%.2s %2d %.3s %d",
      wday_name[timeptr->tm_wday],
      timeptr->tm_mday, 
      mon_name[timeptr->tm_mon],
      1900 + timeptr->tm_year);
    return time_buff;
  }

  // time
  if(mode == 1) {
    sprintf(time_buff, "%.2d:%.2d:%.2d",
      timeptr->tm_hour,
      timeptr->tm_min, 
      timeptr->tm_sec);
    return time_buff;
  }

  // shortdate
  if(mode == 2) {
    sprintf(time_buff, "%.2d-%.2d-%.4d",
      timeptr->tm_mday,
      timeptr->tm_mon, 
      1900 + timeptr->tm_year);
    return time_buff;
  }

  // shorttime (no seconds) (default)
  sprintf(time_buff, "%.2d:%.2d",
  timeptr->tm_hour,
  timeptr->tm_min);

  return time_buff;
}


//
// convert degree to cardinal
//
char* degree2cardinal(int degree) 
{
  char cardinals[18][4] = {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW","N"};
  static char cardinal[4] = {};

  int idx = (int)floor((degree/22.5) + 0.5) % 16;
  sprintf(cardinal, "%s",cardinals[idx]);
 
  return cardinal;
}


//
// right align text
//
String align(String in) 
{
  String out;
  String fill;
  char inlen = 0;
  char pos = 0;

  inlen = in.length();
  
  if(inlen < ROWSIZE) {
    pos = in.indexOf(":")+2;
    for(int i=1; i!=ROWSIZE-inlen+1; i++)
      fill+=" ";
    out=in.substring(0, pos) + fill + in.substring(pos, inlen);
  }
  return out;
}
