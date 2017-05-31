#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>
#include "SoftwareSerial.h"

#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

#include "ekg.h"

char ssid[] = "Kursancew";            // your network SSID (name)
char pass[] = "bcdfgh3690";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

#define CS   3
#define DC   4
#define RESET  2

#define LO1 A2
#define LO2 A3
#define BEATP A0
#define BUTTON 7

//#define DBG(x) Serial.println(x)
//#define DBGN(m,v) do{Serial.print(m);Serial.print(' ');Serial.println(v);}while(0)

#ifndef DBG
#define DBG(x)
#define DBGN(m,v)
#endif

SoftwareSerial Serial1(8, 9); // RX, TX
TFT screen = TFT(CS, DC, RESET);

class EKG_hw {
public:
  void begin() {  pinMode(LO1, INPUT); pinMode(LO2, INPUT); }
  bool sample_good() { return !digitalRead(LO1) && !digitalRead(LO1); }
  int sample() { return analogRead(BEATP); }
};

class Display {
  int x;
  int prev_h;
  EKG *e;
  enum {
    S_DYN,
    S_CALIBRATING,
  };
  int last;
  int cmin, cmax;
public:
  Display(EKG* e_) {
    x = 0;
    e = e_;
  }
  void draw_calibration() {
    if (last == S_CALIBRATING) return;
    screen.background(0, 0, 0);
    screen.stroke(255, 255, 0);
    screen.text("auto-calibrate...", 0, 0);
    last = S_CALIBRATING;
    delay(500);
  }
  void update(int v, int status) {
    if (status == EKG::CALIBRATING) {
      draw_calibration();
      return;
    }
    if (last != S_DYN) {
      clear_screen();
    }
    last = S_DYN;
    int h = screen.height() - map(v, cmin, cmax, 0, screen.height()-55) - 45;
    screen.stroke(255, 0, 0);
    screen.line(x-1, prev_h, x, h);
    screen.stroke(0, 0, 255);
    if (status == EKG::BEAT) {
      screen.line(x, screen.height()-30, x, screen.height()-20);
    }
    ++x;
    if(x >= screen.width()) {
      DBG("clear");
      x = 1;
      clear_screen();
    }
    prev_h = h;
  }
  
  void clear_screen() {
    screen.background(0, 0, 0);
    screen.stroke(255, 255, 0);
    screen.line(0, screen.height() - 20, screen.width(), screen.height() - 20);
    cmin = e->get_cal().get_min()+10;
    cmax = e->get_cal().get_max()-10;
    String stat = String(e->bpm()) + "bpm ||" + cmin + " " + cmax + " " + e->get_cal().get_maxd() + " " + e->get_cal().get_avg();
    screen.text(stat.c_str(), 0, screen.height() - 18);
  }

  void begin() {
    screen.begin();
    screen.background(0, 0, 0);
    screen.stroke(255, 255, 255);
    screen.setTextSize(1);
    screen.text("starting...", 0, 0);
  }
};

EKG_hw ekghw;
EKG ekg = EKG();
Display disp = Display(&ekg);

void setup() {
  Serial.begin(115200);
  Serial1.begin(38400);
  disp.begin();
  pinMode(BUTTON, INPUT_PULLUP);
}

unsigned char loopcnt = 0;

void loop() {
  bool good = ekghw.sample_good();
  int s = ekghw.sample();
  //Serial.println(s);
  int stat = ekg.push(s, good);
  int v = ekg.v();
  

  if (loopcnt >= 4 || stat == EKG::BEAT) {
    disp.update(v, stat);
    loopcnt = 0;
  }
  /*if (ekghw.sample_good()) {
    w.update(ekghw.sample());
    if(loopcnt & 1) Serial.println(w.avg());
  }*/
  //if (ekghw.sample_good())
  //  Serial.println(ekghw.sample());
  delay(5);
  ++loopcnt;
}

