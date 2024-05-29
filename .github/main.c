#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int pinBouton = 10;
const int pinLED = 8;
const int pinServo = 11;
const int pinDHT = 9;

#define TYPE_DHT DHT11
DHT dht(pinDHT, TYPE_DHT);

Servo monServo;

const float seuilTemp = 25.0;

int etatBouton;
int dernierEtatBouton = LOW;
bool etatLED = false;
int positionServo = 0;
bool modeManuel = false;

unsigned long dernierTempsDebounce = 0;
unsigned long delaiDebounce = 50;

unsigned long dernierTempsLectureDHT = 0;
const long intervalleLectureDHT = 2000;

unsigned long dernierTempsMouvementServo = 0;
const long intervalleMouvementServo = 100;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  monServo.attach(pinServo);
  
  pinMode(pinLED, OUTPUT);
  pinMode(pinBouton, INPUT);
  
  dht.begin();
  
  Serial.begin(9600);
  Serial.println("Configuration terminée");
  
  lcd.init();
  lcd.backlight();
  lcd.print("Init complete");
}

void loop() {
  int lecture = digitalRead(pinBouton);

  if (lecture != dernierEtatBouton) {
    dernierTempsDebounce = millis();
  }

  if ((millis() - dernierTempsDebounce) > delaiDebounce) {
    if (lecture != etatBouton) {
      etatBouton = lecture;

      if (etatBouton == HIGH) {
        etatLED = !etatLED;
        digitalWrite(pinLED, etatLED);
        Serial.println(etatLED ? "LED allumée" : "LED éteinte");

        modeManuel = !modeManuel;
        Serial.println(modeManuel ? "Mode manuel activé" : "Mode automatique activé");

        if (modeManuel) {
          positionServo = 0;
          monServo.write(positionServo);
          lcd.setCursor(0, 1);
          lcd.print("Mode manuel ON ");
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Mode auto ON   ");
        }
      }
    }
  }

  if (!modeManuel && millis() - dernierTempsLectureDHT > intervalleLectureDHT) {
    dernierTempsLectureDHT = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.print("Tentative de lecture DHT...");
    if (isnan(h) || isnan(t)) {
      Serial.println(" Échec!");
      lcd.setCursor(0, 1);
      lcd.print("DHT Erreur     ");
    } else {
      Serial.print(" Succès - Humidité: ");
      Serial.print(h);
      Serial.print("%, Température: ");
      Serial.print(t);
      Serial.println(" °C");

      lcd.setCursor(0, 0);
      lcd.print("T: ");
      lcd.print(t);
      lcd.print("C H: ");
      lcd.print(h);
      lcd.print("%");

      if (t > seuilTemp && millis() - dernierTempsMouvementServo > intervalleMouvementServo) {
        dernierTempsMouvementServo = millis();
        positionServo = positionServo == 0 ? 180 : 0;
        monServo.write(positionServo);
        Serial.println("Servo déplacé toutes les 100 ms à cause de la température élevée.");
        lcd.setCursor(0, 1);
        lcd.print("Ventilateur ON  ");
      } else if (t <= seuilTemp) {
        positionServo = 0;
        monServo.write(positionServo);
        Serial.println("Servo arrêté à cause de la température normale.");
        lcd.setCursor(0, 1);
        lcd.print("Ventilateur OFF ");
      }
    }
  }

  dernierEtatBouton = lecture;
}