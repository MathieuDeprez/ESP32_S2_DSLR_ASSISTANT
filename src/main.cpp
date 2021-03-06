#include "main.h"
#include "main_variable.h"

#include "wifiCredentials.h"

bool liveViewStatus = false;
/*
#include <hidboot.h>
#include <usbhub.h>

class KbdRptParser : public KeyboardReportParser
{
  void PrintKey(uint8_t mod, uint8_t key);

protected:
  void OnControlKeysChanged(uint8_t before, uint8_t after);

  void OnKeyDown(uint8_t mod, uint8_t key);
  void OnKeyUp(uint8_t mod, uint8_t key);
  void OnKeyPressed(uint8_t key);
};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
  MODIFIERKEYS mod;
  *((uint8_t *)&mod) = m;
  Serial.print((mod.bmLeftCtrl == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI == 1) ? "G" : " ");

  Serial.print(" >");
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI == 1) ? "G" : " ");
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  Serial.print("DN ");
  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);

  if (c)
    OnKeyPressed(c);
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after)
{

  MODIFIERKEYS beforeMod;
  *((uint8_t *)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t *)&afterMod) = after;

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl)
  {
    Serial.println("LeftCtrl changed");
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift)
  {
    Serial.println("LeftShift changed");
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt)
  {
    Serial.println("LeftAlt changed");
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI)
  {
    Serial.println("LeftGUI changed");
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl)
  {
    Serial.println("RightCtrl changed");
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift)
  {
    Serial.println("RightShift changed");
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt)
  {
    Serial.println("RightAlt changed");
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI)
  {
    Serial.println("RightGUI changed");
  }
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  Serial.print("UP ");
  PrintKey(mod, key);
}

void KbdRptParser::OnKeyPressed(uint8_t key)
{
  Serial.print("ASCII: ");
  Serial.println((char)key);
};

USB Usb;
// USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb);

KbdRptParser Prs;*/

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
int toggleLiveViewRP = -1;

void notifyClients()
{
  ws.textAll("ABC");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "toggleLiveView") == 0)
    {
      Serial.println("toggleLiveView");
      toggleLiveViewRP = !liveViewStatus;
      // notifyClients();
    }
    else if (strcmp((char *)data, "getPreview") == 0)
    {
      Serial.println("getPreview");
      myCamera.getPreview();
      //myCamera.GetObjectHandles();
    }
  }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String wsCallback(const String &var)
{
  Serial.println(var);
  if (var == "LIVE_VIEW_STATE")
  {
    if (liveViewStatus)
    {
      return "ON";
    }
    else
    {
      return "OFF";
    }
  }
  return String();
}

void webSocketInit()
{
  WiFi.begin(mySsid, myPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, wsCallback); });

  // Start server
  server.begin();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  /*if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay(200);

  HidKeyboard.SetReportParser(0, &Prs);*/

  webSocketInit();

  myCamera.init();
}

void loop()
{
  myCamera.loopUsb();

  // delay(100);

  static unsigned long beginEvent = millis();
  if (millis() - beginEvent > 2000 && myCamera.isCameraReady)
  {
    beginEvent = millis();
    // Serial.println("checkEvent");

    static bool openSession = false;
    static bool getDevProp = false;
    if (!openSession)
    {
      openSession = true;
      myCamera.openSession();
    }
    /*else if (!getDevProp)
    {
      getDevProp = true;
      myCamera.getDevProps();
    }*/
    else
    {
      // myCamera.getAperture();
      // myCamera.checkEvent();
      if (toggleLiveViewRP != -1)
      {
        myCamera.setLiveViewStatus(toggleLiveViewRP);
        toggleLiveViewRP = -1;
      }
      else if (liveViewStatus)
      {
        myCamera.getImage();
        beginEvent = millis() - 3000;
      }
    }
  }
  // Usb.Task();
}
