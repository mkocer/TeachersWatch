#include <pebble.h>
#include "battery.h"

static uint8_t current_battery = 100;
static bool current_bt = false;

static void handle_battery(BatteryChargeState charge_state) {

  if (charge_state.is_charging) {
      current_battery = 200;
  } else {
      current_battery = charge_state.charge_percent;
  }
}

static void handle_bluetooth(bool connected) {
    if (connected != current_bt) {
        vibes_short_pulse();
    }
    current_bt =  connected;
}

void init_BTbat_status(){
    handle_bluetooth(connection_service_peek_pebble_app_connection());
    battery_state_service_subscribe(handle_battery);
    connection_service_subscribe((ConnectionHandlers) {
       .pebble_app_connection_handler = handle_bluetooth
    });
    handle_battery(battery_state_service_peek());
}

uint8_t get_current_battery() {
    return current_battery;
}

bool get_current_bt() {
    return current_bt;
}