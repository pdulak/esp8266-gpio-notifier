#include "FS.h"
#include <ESP8266WiFi.h> 
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include "html_const.h"

WiFiManager wifiManager;

// Wiring of NodeMCU on ESP8266, inputs that are safe to use
#define D1 5    // GPIO5
#define D2 4    // GPIO4
#define D5 14   // GPIO14
#define D6 12   // GPIO12
#define D7 13   // GPIO13
#define PIN_COUNT 5 // how many pins in use

/*
 * PINS
 */
int pin_array[] = { D1, D2, D5, D6, D7 };               // pin reference
int pin_states[] = { 0, 0, 0, 0, 0 };                   // last state of the pin
int pin_states_change_unhandled[] = { 0, 0, 0, 0, 0 };  // state change is already handled or not?
unsigned long pin_last_change[] { 0, 0, 0, 0, 0 };      // time of last state change
unsigned long pin_handle_after[] { 0, 0, 0, 0, 0 };     // time after which to handle the pin
String pin_handle_urls[] = { "", "", "", "", "" };      // send notification to these urls
String pin_names[] = { "D1", "D2", "D5", "D6", "D7" };  // pin names on the NodeMCU board
/* 
 *  HTML Server
 */
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("start GPIO notifier");
  
  // start WiFi
  WiFi.hostname("ESP-gipo-notifier-" + WiFi.macAddress());
  wifiManager.autoConnect();

  // initialize digital pins, statuses, times and so on
  initialize_pins();

  // initialize web server
  initialize_www();

  // initialize handler
  initialize_handler();
}

void loop() {
  read_pin_statuses();
  handle_pin_changes();
  server.handleClient(); 
}

/*
 * 
 *  HTML SERVER
 * 
 */

void handle_root() {
  String h = HTML_header;
  String b = HTML_body;
  String f = HTML_footer;
  String fh = HTML_form_header;
  String ff = HTML_form_footer;

  String t = "<h5>Current CPU time: "; 
  t += TimeShowFormatted(millis());
  t += "</h5>";
  
  String s = pin_state_all();
  
  server.send(200, "text/html", h + b + s + t + fh + all_fieldsets() + ff + f);
}

void handle_saved() {
  String h = HTML_header;
  String f = HTML_footer;
  String b = HTML_saved_button;
  
  server.send(200, "text/html", h + b + f);
}

void handle_form() {
  for (int i = 0; i < PIN_COUNT; i++) {
    
    if ( server.hasArg( pin_names[i] ) && server.arg( pin_names[i] ) != NULL ) {
      pin_handle_after[i] = server.arg( pin_names[i] ).toInt();
      Serial.println("Field " + pin_names[i] + " filled with: " + pin_handle_after[i] );
    } else {
      pin_handle_after[i] = 0;
      Serial.println("Field " + pin_names[i] + " not found or null");
    }
    write_to_file(pin_names[i] + "_handler_time", server.arg( pin_names[i] ));

    if ( server.hasArg( pin_names[i] + "_u" ) && server.arg( pin_names[i] + "_u" ) != NULL ) {
      pin_handle_urls[i] = server.arg( pin_names[i] + "_u" );
      Serial.println("Field " + pin_names[i] + "_u filled with: " + pin_handle_urls[i] );
    } else {
      pin_handle_urls[i] = "";
      Serial.println("Field " + pin_names[i] + "_u not found or null");
    }
    write_to_file(pin_names[i] + "_handler_url", pin_handle_urls[i] );
    
  }
  
  server.sendHeader("Location","/saved"); 
  server.send(303);
}

void handle_not_found() {
  server.send(404, "text/plain", "Page not found");
}

void initialize_www() {
  server.on("/", handle_root);
  server.on("/save", HTTP_POST, handle_form);
  server.on("/saved", handle_saved);
  server.onNotFound(handle_not_found);
  server.begin(); 
}

String all_fieldsets() {
  String all_f;
  for (int i = 0; i < PIN_COUNT; i++) {
    all_f += pin_fieldset(i);
  }
  return all_f;
}

String pin_fieldset(int i) {
  String this_fieldset = "<fieldset><legend>" + pin_names[i] + "</legend>";
  this_fieldset += "<label for=\"" + pin_names[i] + "\">handle " + pin_names[i] + " after (milliseconds): </label>";
  this_fieldset += "<input type=\"number\" name=\"" + pin_names[i] + "\" value=\"" + pin_handle_after[i] + "\" min=\"0\" max=\"3600000\">";
  this_fieldset += "<label for=\"" + pin_names[i] + "_u\">Notification URL: </label>";
  this_fieldset += "<input type=\"text\" name=\"" + pin_names[i] + "_u\" value=\"" + pin_handle_urls[i] + "\" maxlength=\"500\" size=\"100\">";
  this_fieldset += "</fieldset>";
  
  return this_fieldset;
}

/*
 * 
 *  PINS toolbox
 * 
 */

void initialize_pins() {
  // set all pins in use to input type and set initial state
  for (int i = 0; i < PIN_COUNT; i++) {
    pinMode(pin_array[i], INPUT);
    pin_states[i] = digitalRead(pin_array[i]);
    pin_last_change[i] = millis();
  }
}

String pin_state_all() {
  String all_statuses;
  for (int i = 0; i < PIN_COUNT; i++) {
    all_statuses += pin_state(i);
  }
  return all_statuses;
}

String pin_state(int i) {
  String this_pin_state = "<div class=\"pin-status\">State of " + pin_names[i] 
    + ": <span class=\'pin-state\'>" + pin_states[i] 
    + "</span>; last change on CPU time: " + TimeShowFormatted(pin_last_change[i]) 
    + " (" + TimeShowFormatted( millis() - pin_last_change[i] ) + " ago)";

  if ( pin_states_change_unhandled[i] == 1) {
    this_pin_state += " (not handled)";
  } else {
    this_pin_state += " (handled)";
  }

  if ( pin_handle_urls[i] == "" ) {
    this_pin_state += "<span class=\"comment\">Empty URL, will not handle</span>";
  } else {
    this_pin_state += "<span class=\"comment\">Handle change after: ";
    this_pin_state += pin_handle_after[i];
    this_pin_state += " milliseconds by sending GET to: " + pin_handle_urls[i] + "</span>";
  }
    
  this_pin_state += "</div>";
  return this_pin_state;
}

void read_pin_statuses() {
  byte this_pin_state;
  unsigned long current_millis = millis();
  unsigned long time_elapsed;
  
  // read all pins statuses
  for (int i = 0; i < PIN_COUNT; i++) {
    // read and display pin state
    this_pin_state = digitalRead(pin_array[i]);


    // check if state changed
    if ( this_pin_state != pin_states[i] ) {
      time_elapsed = current_millis - pin_last_change[i];
      Serial.print("State of " + pin_names[i] + ": ");
      Serial.print( this_pin_state );
      Serial.print(" <- state changed, was in previous state for: ");
      Serial.print( time_elapsed );
      Serial.print(" millis.");
      Serial.println();
      pin_states[i] = this_pin_state;       // save current state
      pin_last_change[i] = current_millis;  // save time of state change
      pin_states_change_unhandled[i] = 1;   // set change as unhandled
      
    } 
  }
}

void handle_pin_changes() {
  unsigned long current_millis = millis();
  
  for (int i = 0; i < PIN_COUNT; i++) {
    if (pin_states_change_unhandled[i] == 1) {
      
      // check if millis counter reached limit and returned to zero
      if (pin_last_change[i] > current_millis) {
        pin_last_change[i] = 0;
      }

      // check if should handle now
      if ( (current_millis - pin_last_change[i]) > pin_handle_after[i] ) {
        Serial.println("Time to handle for " + pin_names[i] + " reached, handling...");
        send_notification( pin_handle_urls[i] );
        pin_states_change_unhandled[i] = 0;
      }
      
    }
  }
}

void send_notification( String url ) {
  Serial.println("Sending notification to: " + url);
  if (url != "") {
    HTTPClient http;

    http.begin(url);
    int httpResponseCode = http.GET();
      
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  } else {
    Serial.println("Empty URL, skipping");
  }
}

void initialize_handler() {
  String buf;
  SPIFFS.begin();

  // read handler values for all pins
  Serial.println("Handler initialization");

  for (int i = 0; i < PIN_COUNT; i++) {
    Serial.println("Reading configuration for " + pin_names[i]);
    
    buf = load_from_file( pin_names[i] + "_handler_time" );
    if (buf == "") {
      pin_handle_after[i] = 0;
    } else {
      pin_handle_after[i] = buf.toInt();
    }

    Serial.print("Handle time set to: ");
    Serial.println( pin_handle_after[i] );

    pin_handle_urls[i] = load_from_file( pin_names[i] + "_handler_url" );
    
    Serial.println("Handle URL set to: " + pin_handle_urls[i]);
  }
  
}

/*
 * 
 *  Files toolbox
 * 
 */

String load_from_file(String file_name) {
  String result = "";
  
  File this_file = SPIFFS.open(file_name, "r");
  if (!this_file) {
    return result;
  }

  while (this_file.available()) {
      result += (char)this_file.read();
      // Serial.write(this_file.read());
  }
  
  this_file.close();

  return result;
}

bool write_to_file(String file_name, String contents) {
  
  File this_file = SPIFFS.open(file_name, "w");
  if (!this_file) {
    return false;
  }

  // int bytesWritten = this_file.print(contents);
  int bytesWritten = this_file.print(contents);
 
  if (bytesWritten == 0) {
      Serial.println("File " + file_name  + " write failed");
      return false;
  }
   
  this_file.close();

  return true;
}

String TimeShowFormatted(unsigned long ms) {
  long days = 0;
  long hours = 0;
  long mins = 0;
  long secs = 0;
  String secs_o = ":";
  String mins_o = ":";
  String hours_o = ":";
  secs = ms / 1000; // set the seconds remaining
  mins = secs / 60; //convert seconds to minutes
  hours = mins / 60; //convert minutes to hours
  days = hours / 24; //convert hours to days
  secs = secs - (mins * 60); //subtract the coverted seconds to minutes in order to display 59 secs max
  mins = mins - (hours * 60); //subtract the coverted minutes to hours in order to display 59 minutes max
  hours = hours - (days * 24); //subtract the coverted hours to days in order to display 23 hours max
  if (secs < 10) {
    secs_o = ":0";
  }
  if (mins < 10) {
    mins_o = ":0";
  }
  if (hours < 10) {
    hours_o = ":0";
  }
  return days + hours_o + hours + mins_o + mins + secs_o + secs;
}
