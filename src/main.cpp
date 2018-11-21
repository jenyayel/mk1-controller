//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

AsyncWebServer server(80);
Servo bottom, right, left, front;

const char *PARAM_POSITION = "position";
const char *ssid = "Robot";
const char *password = "55555555";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

void setupNetwork()
{
    Serial.print("Setting WiFi...");
    // WiFi.begin(ssid, password);
    // Serial.print("Address: ");
    // Serial.println(WiFi.localIP());
    Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting soft-AP ... ");
    Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");
    Serial.println(WiFi.softAPIP());
}

boolean isValidNumber(String str)
{
    boolean isNum = false;
    for (byte i = 0; i < str.length(); i++)
    {
        isNum = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
        if (!isNum)
            return false;
    }
    return isNum;
}

void logRequest(AsyncWebServerRequest *request)
{
    Serial.printf("Url [%s]\n", request->url().c_str());
    int args = request->args();
    for (int i = 0; i < args; i++)
    {
        Serial.printf("ARG[%s]:\t%s\n", request->argName(i).c_str(), request->arg(i).c_str());
    }
    Serial.print("--------------------------------------------\n");
}

int validateAndGetNewPosition(AsyncWebServerRequest *request)
{
    if (!request->hasParam(PARAM_POSITION))
        return -1;

    String rawPosition = request->getParam(PARAM_POSITION)->value();
    if (!isValidNumber(rawPosition))
        return -1;

    int position = atoi(rawPosition.c_str());
    if (position < 0 || position > 180)
        return -1;
    return position;
}

void setupServer()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.onNotFound([](AsyncWebServerRequest *request) {
        logRequest(request);
        request->send(404, "text/plain", "Not found");
    });

    server.on("/right", HTTP_POST, [](AsyncWebServerRequest *request) {
        logRequest(request);
        int position;
        if ((position = validateAndGetNewPosition(request)) == -1)
        {
            request->send(400, "text/plain", "Missing or invalid parameters 'position'");
            return;
        }

        right.write(position);
        request->send(200, "text/plain", "OK");
    });
    server.on("/left", HTTP_POST, [](AsyncWebServerRequest *request) {
        logRequest(request);
        int position;
        if ((position = validateAndGetNewPosition(request)) == -1)
        {
            request->send(400, "text/plain", "Missing or invalid parameters 'position'");
            return;
        }

        left.write(position);
        request->send(200, "text/plain", "OK");
    });
    server.on("/front", HTTP_POST, [](AsyncWebServerRequest *request) {
        logRequest(request);
        int position;
        if ((position = validateAndGetNewPosition(request)) == -1)
        {
            request->send(400, "text/plain", "Missing or invalid parameters 'position'");
            return;
        }

        front.write(position);
        request->send(200, "text/plain", "OK");
    });
    server.on("/bottom", HTTP_POST, [](AsyncWebServerRequest *request) {
        logRequest(request);
        int position;
        if ((position = validateAndGetNewPosition(request)) == -1)
        {
            request->send(400, "text/plain", "Missing or invalid parameters 'position'");
            return;
        }

        bottom.write(position);
        request->send(200, "text/plain", "OK");
    });
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
        logRequest(request);
        request->send(200, "text/plain", String(right.read()));
    });
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
        logRequest(request);
        request->send(200, "text/plain", String(left.read()));
    });
    server.on("/bottom", HTTP_GET, [](AsyncWebServerRequest *request) {
        logRequest(request);
        request->send(200, "text/plain", String(bottom.read()));
    });
    server.on("/front", HTTP_GET, [](AsyncWebServerRequest *request) {
        logRequest(request);
        request->send(200, "text/plain", String(front.read()));
    });
    server.begin();
}

void setupServos()
{
    bottom.attach(16);
    right.attach(4);

    left.attach(5);
    left.write(113);

    front.attach(0);
    front.write(115);
}

void setup()
{
    Serial.begin(9600);
    delay(3000);
    SPIFFS.begin();

    setupNetwork();
    setupServer();
    setupServos();
}

void loop()
{
    Serial.printf("Stations connected to soft-AP = %d\n", WiFi.softAPgetStationNum());
    delay(5000);
}