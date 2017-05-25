#define LO1 A2
#define LO2 A3
#define BEAT A0

class EKG_hw {
public:
  void begin() {
    pinMode(LO1, INPUT);
    pinMode(LO2, INPUT);
  }
  bool sample_good() { return !digitalRead(LO1) && !digitalRead(LO1); }
  int sample() { return analogRead(BEAT); }
};

EKG_hw ekghw;

void setup() {
  Serial.begin(115200);
  ekghw.begin();
}

void loop() {
  if (ekghw.sample_good())
    Serial.println(ekghw.sample());
  delay(5);
}
