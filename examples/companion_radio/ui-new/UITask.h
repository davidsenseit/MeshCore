#pragma once

#include <MeshCore.h>
#include <helpers/ui/DisplayDriver.h>
#include <helpers/ui/UIScreen.h>
#include <helpers/SensorManager.h>
#include <helpers/BaseSerialInterface.h>
#include <Arduino.h>
#include <helpers/sensors/LPPDataHelpers.h>

#ifndef LED_STATE_ON
  #define LED_STATE_ON 1
#endif

#ifdef PIN_BUZZER
  #include <helpers/ui/buzzer.h>
#endif
#ifdef PIN_VIBRATION
  #include <helpers/ui/GenericVibration.h>
#endif

#include "../AbstractUITask.h"
#include "../NodePrefs.h"

class UITask : public AbstractUITask {
  DisplayDriver* _display;
  SensorManager* _sensors;
#ifdef PIN_BUZZER
  genericBuzzer buzzer;
#endif
#ifdef PIN_VIBRATION
  GenericVibration vibration;
#endif
  unsigned long _next_refresh, _auto_off;
  NodePrefs* _node_prefs;
  char _alert[80];
  unsigned long _alert_expiry;
  int _msgcount;
  unsigned long ui_started_at, next_batt_chck;
  int next_backlight_btn_check = 0;
#ifdef PIN_STATUS_LED
  int led_state = 0;
  int next_led_change = 0;
  int last_led_increment = 0;
#endif

#ifdef PIN_USER_BTN_ANA
  unsigned long _analogue_pin_read_millis = millis();
#endif

  // Emergency group message storage
  struct EmergencyMsg {
    char sender[32];
    char text[200];  // Increased to store longer messages
    uint32_t timestamp;
    bool timestamp_set = false;  // Flag to indicate if timestamp has been recorded
  } _latest_emergency_msg;

#ifdef PIN_BUZZER
  // Buzzer repeating pattern state
  bool _buzzer_alert_active = false;
  unsigned long _buzzer_next_event = 0;
  bool _buzzer_pause_phase = true;  // true = pause (1000ms), false = sound (500ms)
#endif

  UIScreen* splash;
  UIScreen* home;
  UIScreen* msg_preview;
  UIScreen* curr;

  void userLedHandler();

  // Button action handlers
  char checkDisplayOn(char c);
  char handleLongPress(char c);
  char handleDoubleClick(char c);
  char handleTripleClick(char c);

  void setCurrScreen(UIScreen* c);

public:

  UITask(mesh::MainBoard* board, BaseSerialInterface* serial) : AbstractUITask(board, serial), _display(NULL), _sensors(NULL) {
    next_batt_chck = _next_refresh = 0;
    ui_started_at = 0;
    curr = NULL;
  }
  void begin(DisplayDriver* display, SensorManager* sensors, NodePrefs* node_prefs);

  void gotoHomeScreen() { setCurrScreen(home); }
  void showAlert(const char* text, int duration_millis);
  int  getMsgCount() const { return _msgcount; }
  
  // Emergency message access
  const char* getLatestEmergencySender() const { return _latest_emergency_msg.sender; }
  const char* getLatestEmergencyText() const { return _latest_emergency_msg.text; }
  uint32_t getLatestEmergencyTimestamp() const { return _latest_emergency_msg.timestamp; }
  bool hasEmergencyMsg() const { return _latest_emergency_msg.sender[0] != 0; }
  bool isEmergencyTimestampSet() const { return _latest_emergency_msg.timestamp_set; }
  void setEmergencyTimestamp(uint32_t ts) { _latest_emergency_msg.timestamp = ts; _latest_emergency_msg.timestamp_set = true; }
  
  bool hasDisplay() const { return _display != NULL; }
  bool isButtonPressed() const;

  bool isBuzzerQuiet() { 
#ifdef PIN_BUZZER
    return buzzer.isQuiet();
#else
    return true;
#endif
  }

  void toggleBuzzer();
  bool getGPSState();
  void toggleGPS();

  // Emergency alert buzzer control
  void stopEmergencyBuzzer() {
#ifdef PIN_BUZZER
    _buzzer_alert_active = false;
    buzzer.stop();
#endif
  }

  // from AbstractUITask
  void msgRead(int msgcount) override;
  void newMsg(uint8_t path_len, const char* from_name, const char* text, int msgcount) override;
  void notify(UIEventType t = UIEventType::none) override;
  void loop() override;

  void shutdown(bool restart = false);
};
