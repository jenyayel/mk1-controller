/* 
 * 
 * Wifi accesspoint for controlling mk1 robot arm
 * 
 *
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

AsyncWebServer server(80);
Servo bottom, right, left, front;

int SRV_R = 4;
int SRV_L = 5;
int SRV_B = 16;
int SRV_F = 0;

const char *PARAM_POSITION = "position";
const char *ssid = "Robot";
const char *password = "55555555";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Configures soft access point
void setupNetwork()
{
    Serial.print("Setting soft-AP configuration... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "ready" : "failed");

    Serial.print("Setting soft-AP... ");
    Serial.println(WiFi.softAP(ssid, password) ? "ready" : "failed");
    Serial.println(WiFi.softAPIP());
}

// validates that a given string is valid integer number
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

// logs to serial HTTP request
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

// validates parameter for servo positioning
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

// hanlder for requests for moving specified servo
void servoMoveHandler(AsyncWebServerRequest *request, Servo *servo)
{
    logRequest(request);
    int position;
    if ((position = validateAndGetNewPosition(request)) == -1)
    {
        request->send(400, "text/plain", "Missing or invalid parameters 'position'");
        return;
    }
    servo->write(position);
    request->send(200, "text/plain", "OK");
}

// hanlder for requests for getting servo position
void servoReadHandler(AsyncWebServerRequest *request, Servo *servo)
{
    logRequest(request);
    request->send(200, "text/plain", String(servo->read()));
}

// setup endpoint routes for web server
void setupServer()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.onNotFound([](AsyncWebServerRequest *request) {
        logRequest(request);
        request->send(404, "text/plain", "Not found");
    });

    server.on("/right", HTTP_POST, [](AsyncWebServerRequest *request) {
        servoMoveHandler(request, &right);
    });
    server.on("/left", HTTP_POST, [](AsyncWebServerRequest *request) {
        servoMoveHandler(request, &left);
    });
    server.on("/front", HTTP_POST, [](AsyncWebServerRequest *request) {
        servoMoveHandler(request, &front);
    });
    server.on("/bottom", HTTP_POST, [](AsyncWebServerRequest *request) {
        servoMoveHandler(request, &bottom);
    });
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
        servoReadHandler(request, &right);
    });
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
        servoReadHandler(request, &left);
    });
    server.on("/bottom", HTTP_GET, [](AsyncWebServerRequest *request) {
        servoReadHandler(request, &bottom);
    });
    server.on("/front", HTTP_GET, [](AsyncWebServerRequest *request) {
        servoReadHandler(request, &front);
    });
    server.begin();
}

// setup servos
void setupServos()
{
    bottom.attach(SRV_B);
    right.attach(SRV_R);
    left.attach(SRV_L);
    front.attach(SRV_F);
    
    // adjust inital positioning of servos
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
    Serial.printf("Clients connected [%d]\n", WiFi.softAPgetStationNum());
    delay(5000);
}