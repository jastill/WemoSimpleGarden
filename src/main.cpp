#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN D4
#define NUMPIXELS 12

#define BLUE pixels.Color(0, 0, 255)
#define GREEN pixels.Color(0, 255, 0)
#define RED pixels.Color(255, 0, 0)
#define OFF pixels.Color(0, 0, 0)

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Wifi credentials
const char *ssid = "15033402";
const char *password = "CapsCapsCaps";

int WiFiStrength = 0;

WiFiServer server(80);

void setLedsOff()
{
  Serial.println("Setting LEDs OF");
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, OFF);
  }
  pixels.show();
}

void setLedsRed()
{
  Serial.println("Setting LEDs RED");
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, RED);
  }
  pixels.show();
}

void setLedsGreen()
{
  Serial.println("Setting LEDs GREEN");
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, GREEN);
  }
  pixels.show();
}

void setLedsBlue()
{
  Serial.println("Setting LEDs BLUE");
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, BLUE);
  }
  pixels.show();
}

void setupLeds()
{
  pixels.begin();
  pixels.setBrightness(255);
  setLedsRed();
}

void setup()
{

  Serial.begin(115200);
  setupLeds();
  delay(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Set the ip address of the webserver
  // WiFi.config(WebServerIP, Gatway, Subnet)
  // or comment out the line below and DHCP will be used to obtain an IP address

  // which will be displayed via the serial console
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  delay(7500);
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  delay(500);
}

double analogValue = 0.0;
double analogVolts = 0.0;
unsigned long timeHolder = 0;

void loop()
{
  WiFiStrength = WiFi.RSSI();   // get dBm from the ESP8266
  analogValue = analogRead(A0); // read the analog signal

  // convert the analog signal to voltage
  // the ESP2866 A0 reads between 0 and ~3 volts, producing a corresponding value
  // between 0 and 1024. The equation below will convert the value to a voltage value.

  analogVolts = (analogValue * 3.08) / 1024;

  // now get our chart value by converting the analog (0-1024) value to a value between 0 and 100.
  // the value of 400 was determined by using a dry moisture sensor (not in soil, just in air).
  // When dry, the moisture sensor value was approximately 400. This value might need adjustment
  // for fine tuning of the chartValue.

  int chartValue = analogValue / 10;

  if (millis() - 15000 > timeHolder)
  {
    timeHolder = millis();

    //Section customize

    if (analogValue >= 800)
    {
      setLedsRed();
    }
    else if (analogValue < 799 && analogValue > 400)
    {
      setLedsGreen();
    }
    else if (analogValue < 399) // 76-100 is blue
    {
      setLedsBlue();
    }
    //Section customize end

    delay(3000); // this is the duration the LED will stay ON
  }

  // Serial data
  Serial.print("Analog raw: ");
  Serial.println(analogValue);
  Serial.print("Analog V: ");
  Serial.println(analogVolts);
  Serial.print("ChartValue: ");
  Serial.println(chartValue);
  Serial.print("TimeHolder: ");
  Serial.println(timeHolder);
  Serial.print("millis(): ");
  Serial.println(millis());
  Serial.print("WiFi Strength: ");
  Serial.print(WiFiStrength);
  Serial.println("dBm");
  Serial.print(WiFi.localIP());
  Serial.println(" ");
  Serial.println("------------------------------------------------------------------------------------------------------------");
  delay(1000); // slows amount of data sent via serial

  // check to for any web server requests. ie - browser requesting a page from the webserver
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println(" <head>");
  client.println("<meta http-equiv=\"refresh\" content=\"60\">");
  client.println(" <script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>");
  client.println("  <script type=\"text/javascript\">");
  client.println("    google.charts.load('current', {'packages':['gauge']});");
  client.println("    google.charts.setOnLoadCallback(drawChart);");
  client.println("   function drawChart() {");
  client.println("      var data = google.visualization.arrayToDataTable([ ");
  client.println("        ['Label', 'Value'], ");
  client.print("        ['Resistance',  ");
  client.print(chartValue);
  client.println(" ], ");
  client.println("       ]); ");
  // setup the google chart options here

  //Section customize
  client.println("    var options = {");
  client.println("      width: 600, height: 220,");
  client.println("      redFrom: 80, redTo: 100,");
  client.println("      greenFrom: 40, greenTo: 79,");
  client.println("      yellowFrom: 0, yellowTo: 39,");
  client.println("   yellowColor: '#006bff',   ");
  client.println("       minorTicks: 5");
  client.println("    };");

  //Section customize end

  client.println("   var chart = new google.visualization.Gauge(document.getElementById('chart_div'));");
  client.println("  chart.draw(data, options);");
  client.println("  setInterval(function() {");
  client.print("  data.setValue(0, 1, ");
  client.print(chartValue);
  client.println("    );");
  client.println("    chart.draw(data, options);");
  client.println("    }, 13000);");
  client.println("  }");
  client.println(" </script>");
  client.println("  </head>");
  client.println("  <body>");
  client.print("<h1 style=\"size:14px;\">dshop Plant Watering Sensor</h1>");
  // show some data on the webpage and the gauge
  client.println("<table>");
  client.print("WiFi Signal Strength: ");
  client.println(WiFiStrength);
  client.println("dBm<br>");
  client.print("Analog Raw: ");
  client.println(analogValue);
  client.print("<br>Analog Volts: ");
  client.println(analogVolts);
  client.print("<br><br>Legend: ");
  client.print("<br>Blue means the soil is flooded, ");
  client.print("<br>Green means that the soil is at optimal moisture");
  client.print("<br>and red means that the soil is too dry.");
  // below is the google chart html
  client.println("<div id=\"chart_div\" style=\"width: 500px; height: 234px;\"></div>");
  client.println("<br><a href=\"/REFRESH\"\"><button>Refresh</button></a>");
  client.println("</table>");
  client.println("<body>");
  client.println("</html>");
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}