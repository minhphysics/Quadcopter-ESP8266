#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#ifndef APSSID
#define APSSID "ESP"
#define APPSK  "ffffffff"
#endif
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);
bool clearData(const char* filename) {
  SPIFFS.begin();
  File f = SPIFFS.open(String("/") + filename, "w");
  if (!f) {
    f.close();
    return false;
  }
  else {
    f.close();
    return true;
  }
}
bool saveData(const char* filename, const char* content, uint16_t len) {
  SPIFFS.begin();
  File f = SPIFFS.open(String("/") + filename, "a");
  if (!f) {
    f.close();
    return false;
  }
  else {
    f.write((const uint8_t*)content, len);
    f.close();
    return true;
  }
}

String readData(const char* filename) {
  SPIFFS.begin();
  File f = SPIFFS.open(String("/") + filename, "r");
  String ret = f.readString();
  f.close();
  return ret;
}
void setupServer() {
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/index", [] {
    server.send(200, "text/html", readData("index.html"));
  });

  server.on("/UPDATE_THROTTLE", [] {
    String t_state = server.arg("VALUE");
    server.send(200, "text/html", t_state);
    input_throttle = t_state.toInt();
  });
  server.on("/LOAD_DATA", [] {
    server.send(200, "text/html",String(ESCout_1)+" "+String(ESCout_2)+" "+String(ESCout_3)+" "+String(ESCout_4));
  });
  server.on("/UPDATE_ROLL", [] {
    String t_state = server.arg("VALUE");
    server.send(200, "text/html", t_state);
    input_roll = t_state.toInt();
  });
  server.on("/UPDATE_PITCH", [] {
    String t_state = server.arg("VALUE");
    server.send(200, "text/html", t_state);
    input_pitch = t_state.toInt();
  });
  server.on("/UPDATE_YAW", [] {
    String t_state = server.arg("VALUE");
    server.send(200, "text/html", t_state);
    input_yaw = t_state.toInt();
  });
  server.on("/transfile", HTTP_ANY, [] {
    server.send(200, "text/html",
    "<meta charset = 'utf-8'>"
    "<html>"
    "<title>"
    "Transmit File"
    "</title>"
    "<form method='POST' action = '/transfile' enctype='multipart/form-data'>"
    "<input type = 'file' name= 'Chon File'>"
    "<input type = 'submit' name= 'Gui File'>"
    "</form>"
    "</html>"
               );
  }, [] {
    HTTPUpload& file = server.upload();

    if (file.status == UPLOAD_FILE_START)
    {
      Serial.println("Bat dau gui " + file.filename);
      clearData("index.html");
    }
    else if (file.status == UPLOAD_FILE_WRITE) {
      //      Serial.println("Dang gui");
      //      String tmp = String ((char*) file.buf);
      //      Serial.println("Noi dung file:");
      //      Serial.print(tmp);
      saveData("index.html", (const char*)file.buf, file.currentSize);
    }
  });
  server.begin();
  Serial.println("HTTP server started");
}
