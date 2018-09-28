#include "ac.h"

IRSenderBitBang irSender(4);

HeatpumpIR *heatpumpIR[] = {new FujitsuHeatpumpIR(), NULL};


void ac_on() {
  int i = 0;
  const char* buf;
    log("AC ON");
    log("Sending IR to ");

    buf = heatpumpIR[i]->model();
    while (char modelChar = pgm_read_byte(buf++))
    {
      log(modelChar);
    }
    log(", info: ");

    buf = heatpumpIR[i]->info();
    while (char infoChar = pgm_read_byte(buf++))
    {
      log(infoChar);
    }
    log("");

    heatpumpIR[i]->send(irSender, POWER_ON, MODE_COOL, FAN_2, 25, VDIR_UP, HDIR_AUTO);
}

void ac_off() {
  int i = 0;
  const char* buf;
    log("AC OFF");
    log("Sending IR to ");

    buf = heatpumpIR[i]->model();
    while (char modelChar = pgm_read_byte(buf++))
    {
      log(modelChar);
    }
    log(", info: ");

    buf = heatpumpIR[i]->info();
    while (char infoChar = pgm_read_byte(buf++))
    {
      log(infoChar);
    }
    log("");

    heatpumpIR[i]->send(irSender, POWER_OFF, MODE_COOL, FAN_2, 25, VDIR_UP, HDIR_AUTO);
        
}

