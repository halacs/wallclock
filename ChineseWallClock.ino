#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "myTime.h"
#include "myDisplay.h"
#include <LittleFS.h>
#include <ArduinoJson.h>  // https://arduinojson.org/v6/example/config/
#include "myConfig.h"
#include <ArduinoOTA.h>
#include <string.h>
#include <stdlib.h>

WiFiManager wm;
WiFiManagerParameter device_name;
WiFiManagerParameter ntp_server;
WiFiManagerParameter time_zone;
WiFiManagerParameter second_blinking;
WiFiManagerParameter clever;
WiFiManagerParameter leading_zeros;
WiFiManagerParameter brightness;
WiFiManagerParameter auto_brightness;

const char *configFilename = "/halclock.conf";   // <- SD library uses 8.3 filenames
Config config;                                   // <- global configuration object

/*
 Loads the configuration from a file
*/
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  if (LittleFS.begin()) {
    Serial.println("mounted file system");
    if (LittleFS.exists(filename)) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File file = LittleFS.open(filename, "r");
      if (file) {
        Serial.println("opened config file");

        // because of debug purposes send full content of config file to serial
        /*
        File file2 = LittleFS.open(filename, "r");
        String data = file2.readString();
        Serial.println(data);
        file2.close();
        */

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        StaticJsonDocument<512> doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);
        if (!error) {
          Serial.println("Deserialization OK");
          
          strlcpy(config.device_name,                         // <- destination
                  doc["device_name"] | device_name_default,            // <- source
                  sizeof(config.device_name));                // <- destination's capacity

          strlcpy(config.ntp_server,                         // <- destination
                  doc["ntp_server"] | ntp_server_default,     // <- source
                  sizeof(config.ntp_server));                // <- destination's capacity
          
          strlcpy(config.timezone,                           // <- destination
                  doc["timezone"] | timezone_default,       // <- source
                  sizeof(config.ntp_server));                // <- destination's capacity

          config.second_blinking = strcmp(doc["second_blinking"] | second_blinking_default_char, "true") == 0;
          config.clever = strcmp(doc["clever"] | clever_numbering_default_char, "true") == 0;
          config.leading_zeros = strcmp(doc["leading_zeros"] | leading_zeros_default_char, "true") == 0;
          config.auto_brightness = strcmp(doc["auto_brightness"] | auto_brightness_default_char, "true") == 0;          
          config.brightness = atoi(doc["brightness"] | brightness_default_char);
        } else {
          Serial.println(F("Failed to read file, using default configuration"));
        }
        // Close the file (Curiously, File's destructor doesn't close the file)
        file.close();
       } else {
      Serial.println("cannot open file");
    }
    } else {
      Serial.println("file not found");

      // Load defaults  
      strlcpy(config.device_name,                          // <- destination
                    device_name_default,                            // <- source
                    sizeof(config.device_name));                // <- destination's capacity
      
      strlcpy(config.ntp_server,                          // <- destination
                    ntp_server_default,                    // <- source
                    sizeof(config.ntp_server));                // <- destination's capacity
      strlcpy(config.timezone,                           // <- destination
                    timezone_default,                   // <- source
                    sizeof(config.timezone));                 // <- destination's capacity
        
      config.brightness = brightness_default;
      config.leading_zeros = leading_zeros_default;
      config.clever = clever_numbering_default;
      config.second_blinking = second_blinking_default;
      config.auto_brightness = auto_brightness_default;

      Serial.println("Defaults loaded");
    }
  } else {
    Serial.println("failed to mount FS");
  }

}

/*
 Saves the configuration to a file
*/
void saveConfiguration(const char *filename, const Config &config) {
  Serial.println("Saving configuration");

  if (LittleFS.begin()) {
    Serial.println("mounted file system");

    // Delete existing file, otherwise the configuration is appended to the file
    LittleFS.remove(filename);
  
    // Open file for writing
    File file = LittleFS.open(filename, "w");
    if (!file) {
      Serial.println(F("Failed to create file"));
      return;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    StaticJsonDocument<256> doc;

    // Set the values in the document
    doc["device_name"] = config.device_name;
    doc["ntp_server"] = config.ntp_server;
    doc["timezone"] = config.timezone;
    doc["second_blinking"] = config.second_blinking ? "true" : "false";
    doc["leading_zeros"] = config.leading_zeros ? "true" : "false";
    doc["auto_brightness"] = config.auto_brightness ? "true" : "false";

    char tmp_brightness[5];
    itoa(config.brightness, tmp_brightness, 10);
    doc["brightness"] = tmp_brightness;
    
    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }

    serializeJson(doc, Serial);
    Serial.println();

    // Close the file
    file.close();
  } else {
    Serial.println("failed to mount FS");
  }
}

/*
 Called when save button pressed on the web user interface (saving custom parameters)
*/
void saveParamsCallback () {
  strlcpy(config.device_name,                         // <- destination
                device_name.getValue(),              // <- source
                sizeof(config.device_name));          // <- destination's capacity

  strlcpy(config.ntp_server,                        // <- destination
                ntp_server.getValue(),             // <- source
                sizeof(config.ntp_server));         // <- destination's capacity

  strlcpy(config.timezone,                         // <- destination
                time_zone.getValue(),              // <- source
                sizeof(config.timezone));          // <- destination's capacity
  
  config.second_blinking = strcmp(second_blinking.getValue(), "1") == 0;
  config.clever = strcmp(clever.getValue(), "1") == 0;
  config.leading_zeros = strcmp(leading_zeros.getValue(), "1") == 0;
  config.auto_brightness = strcmp(auto_brightness.getValue(), "1") == 0;
  int brightness_tmp = atoi(brightness.getValue());
  if (brightness_tmp > 255) {
    Serial.printf("New brightness value too high: %d. Set to 255.", brightness_tmp);
    brightness_tmp = 255;
  }
  if (brightness_tmp < 1) {
    Serial.printf("New brightness value too low: %d. Set to 1.", brightness_tmp);
    brightness_tmp = 1;
  }
  config.brightness = (byte)brightness_tmp;
  
  saveConfiguration(configFilename, config);
  Serial.println("Trigger ESP restart");
  //ESP.restart();
  wm.reboot();
}

void menuSetup() {
  // custom menu via array or vector
  // 
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi","info","param","sep","restart"};
  wm.setMenu(menu);

  // set dark theme
  //wm.setClass("invert");

  int customFieldLength = 40;

  if (config.second_blinking) {
    new (&second_blinking) WiFiManagerParameter("second_blinking", "Second blinking", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\" checked", WFM_LABEL_AFTER);
  } else {
    new (&second_blinking) WiFiManagerParameter("second_blinking", "Second blinking", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\"", WFM_LABEL_AFTER);
  }
  
  if (config.clever) {
    new (&clever) WiFiManagerParameter("clever", "Clever numbering", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\" checked", WFM_LABEL_AFTER);
  } else {
    new (&clever) WiFiManagerParameter("clever", "Clever numbering", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\"", WFM_LABEL_AFTER);
  }

  if (config.leading_zeros) {
    new (&leading_zeros) WiFiManagerParameter("leading_zeros", "Leading zeros", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\" checked", WFM_LABEL_AFTER);
  } else {
    new (&leading_zeros) WiFiManagerParameter("leading_zeros", "Leading zeros", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\"", WFM_LABEL_AFTER);
  }

  if (config.auto_brightness) {
    new (&auto_brightness) WiFiManagerParameter("auto_brightness", "Auto brightness", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\" checked", WFM_LABEL_AFTER);
  } else {
    new (&auto_brightness) WiFiManagerParameter("auto_brightness", "Auto brightness", "1", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\"", WFM_LABEL_AFTER);
  }

  new (&device_name) WiFiManagerParameter("DeviceName", "Device name:", config.device_name, sizeof(config.device_name));
  new (&ntp_server) WiFiManagerParameter("NTPserver", "NTP server:", config.ntp_server, sizeof(config.ntp_server));
  new (&time_zone) WiFiManagerParameter("TimeZone", "Time zone:", config.timezone, sizeof(config.timezone));

  char tmp_brightness[5];
  itoa(config.brightness, tmp_brightness, 10);
  new (&brightness) WiFiManagerParameter("brightness", "Brightness:", tmp_brightness, sizeof(tmp_brightness), "placeholder=\"Custom Field Placeholder\" type=\"number\" min=\"0\" max=\"255\"");
  
  wm.addParameter(&device_name);
  wm.addParameter(&ntp_server);
  wm.addParameter(&time_zone);
  wm.addParameter(&brightness);
  wm.addParameter(&second_blinking);
  wm.addParameter(&clever);
  wm.addParameter(&leading_zeros);
  wm.addParameter(&auto_brightness);  
  
  //wm.setConfigPortalBlocking(false);
  wm.setSaveParamsCallback(saveParamsCallback);
}

char* hostnameSingleton = 0;
char* getAPName(){
  if (hostnameSingleton == 0) {
    String hostString = String(WIFI_getChipId(),HEX);
    hostString.toUpperCase();
    String name = String("halclock") + hostString;

    hostnameSingleton = (char*)malloc(name.length() + 1);
    memcpy(hostnameSingleton, name.c_str(), name.length() + 1);
  }

  return hostnameSingleton;
}

char* getAPPassword() {
  return "password";
}

void wifiLoop() {
  wm.process();
}

void wifiSetup(char* web_title) {
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  Serial.printf("Host and AP name: %s\n", getAPName());

  wm.setTitle(web_title);
  wm.setHostname(getAPName());
  wm.setConnectRetries(5);
  
  wm.setConfigPortalBlocking(false);
  wm.setWiFiAutoReconnect(true);
  wm.setDisableConfigPortal(false);

  Serial.printf("AP name: %s\nAP password: %s\n", getAPName(), getAPPassword());
  bool res = wm.autoConnect(getAPName(), getAPPassword()); // password protected ap

  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } else {
      //if you get here you have connected to the WiFi    
      Serial.println("Connected to WiFi :)");
      wm.startWebPortal();
  }
}

void OTASetup() {
  Serial.println("Configuring OTA");

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");
  ArduinoOTA.setHostname(getAPName());

  // No authentication by default
  // ArduinoOTA.setPassword("admin");
  char* otaPassword = getAPName();
  ArduinoOTA.setPassword(otaPassword);
  Serial.printf("OTA password set to %s\n", otaPassword);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  
  ArduinoOTA.begin();

  Serial.println("OTA is ready");
}

/*
// int sprintf(char *str, const char *format, ...)
void logging(char* severity, char* message) {
  Serial.printf("[%s] %s\n", severity, message);
}

// int sprintf(char *str, const char *format, ...)
void info(char* message) {
  logging("INFO", message);
}

// int sprintf(char *str, const char *format, ...)
void warning(char* message) {
  logging("WARNING", message);
}

// int sprintf(char *str, const char *format, ...)
void error(char* message) {
  logging("ERROR", message);
  Serial.printf(const char *format, ...)
}
*/

void setup() {
  // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Booting");
 
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  //WiFiManager wm;

  loadConfiguration(configFilename, config);
  Serial.printf("--------------------\nTimeZone: %s\nNTP Server: %s\nDevice name: %s\nClever: %d\nSecond blinking: %d\nbrightness: %d\nLeading zeros: %d\nAuto brightness: %d\n--------------------\n" ,config.timezone, config.ntp_server, config.device_name, config.clever, config.second_blinking, config.brightness, config.leading_zeros, config.auto_brightness);

  displaySetup(); // initialize display first to be able to show messages there
  syncTimeSetup();
  menuSetup();
  wifiSetup(config.device_name);
  
  #ifdef ENABLE_OTA
    OTASetup();
  #endif

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  // wm.resetSettings();
}

void loop() {
  wifiLoop();
  syncTimeLoop();
  syncDisplayScheduler();
  #ifdef ENABLE_OTA
    ArduinoOTA.handle();
  #endif
}
