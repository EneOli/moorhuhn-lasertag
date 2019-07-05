/*
   Moorhuhn.h - Bibliothek zur Festlegung von Moorhühnern
   Fähigkeiten: Leben durch LED anzeigen, Leben/Tod abfragen (LDR), Fliegen (Bewegung) und Sterben (Ton)
   V0.2, 12.04.2017
*/

#include "Moorhuhn.h"

Moorhuhn::Moorhuhn(int nr, int Led, int servoNr, int buzzer, boolean alive): nr(nr), led(led), servoNr(servoNr), buzzer(buzzer), alive(alive) {

  pinMode(this->led, OUTPUT);
  pinMode(this->servoNr, OUTPUT);
  pinMode(this->buzzer, OUTPUT);

  randomSeed(millis()); // Zufall einfügen - eigtl kein richtiger Zufall, aber ich habe keinen analogen Eingang frei
}

// Wenn das Moorhuhn am Leben ist, soll die LED leuchten, sonst nicht.
// Ob es am Leben ist oder nicht, wird durch die Spielart festgelegt und daher von außen durch ein Argument übergeben
void Moorhuhn::live (boolean alive) {
  this->alive = alive;
  if (this->alive) {
    digitalWrite(this->led, HIGH);
  } else {
    digitalWrite(this->led, LOW);
  }
}

boolean Moorhuhn::isAlive() {
  return this->alive;
}

// Diese Funktion fragt ab, ob das Moorhuhn getroffen wurde. Dazu vergleicht es den Wert des LDR in A0 mit dem Referenzwert.
// Der Referenzwert muss übergeben werden - Beim Aufruf des Moorhuhns muss der richtige Referenzwert mitgegeben werden.
boolean Moorhuhn::shot(int ldr_ref) {
  // Aktuellen Wert zu den LDRs einlesen und Vergleich (Differenz) zu Referenzwert bilden
  // Vergleich mit dem Referenzwert: Sinken heißt der LDR mit der kleineren Nr. ist getroffen,
  // Steigen heißt der LDR mit der größeren Nr. ist getroffen,
  // kein relevanter Unterschied bedeutet, dass keiner getroffen wurde
  int l = analogRead(map(this->nr, 0, 5, A0, A2));

  // Wenn der LDR zu Huhn0 getroffen wird, dann steigt die Spannung am LDR1 und damit der Wert in A0. Das heißt, die Differenz l-ldr_ref ist positiv.
  // Wenn der LDR zu Huhn1 getroffen wird, dann sinkt die Spannung am LDR1 und damit der Wert in A0. Das heißt, die Differenz l-ldr_ref ist negativ.
  // Da immer die ungeraden Nummern zwischen A0 und GND sitzen, gilt für Huhn3 und 5 dasselbe wie für Huhn1 und analog für die geraden Zahlen.

  if ((l - ldr_ref > 200 && this->nr % 2 == 0) || (l - ldr_ref < -200 && this->nr % 2 == 1)) {
    return true;
  } else {
    return false;
  }
}

// Das Moorhuhn kann fliegen und dabei sechs verschiedene Positionen annehmen, die zufällig gewählt werden. Vorher wird der zugehörige Servo ausgewählt.
void Moorhuhn::fly () {
  short r = random(5); // Zufallszahl von 0 bis 4 für fünf verschiedene Positionen
  int pos = r * 45; // Winkel

  /* Ein Winkel von 0 Grad entspricht einer Pulsbreite von 1000 Mikrosekunden.
     Ein Winkel von 180 Grad entspricht einer Pulsbreite von 2000 Mikrosekunden.
     In der Zeit scheint der Motor aber nicht den gewünschten Winkel zu erreichen.
     Daher werden die Grenzen etwas vergrößert und der Vorgang dreimal wiederholt.
     Die Wartezeit von 20ms entspricht den 50Hz, die der Motor braucht. (?!)
  */
  for (int i = 0; i < 3; i++) {
    digitalWrite(this->servoNr, HIGH); // Servo Pin auf HIGH zum aktivieren des Servos
    delayMicroseconds(map(pos, 0, 180, 900, 2100)); // Kurze Zeit warten
    digitalWrite(this->servoNr, LOW); // Servo Pin auf LOW zum deaktivieren des servos
    delay(20); // 20 ms warten
  }
}

// Wenn das Moorhuhn stirbt, kann der Abschusston als akustisches Feedback abgespielt werden
void Moorhuhn::die() {
  for (int i = 1; i <= 5; i++) {
    tone(this->buzzer, 1550 + i * 20);
    delay(20);
  }
  delay(20);
  for (int i = 1; i <= 8; i++) {
    tone(this->buzzer, 1650 - i * 20);
    delay(20);
  }
  for (int i = 1; i <= 10; i++) {
    tone(this->buzzer, 1650 - 8 * 20 - i * 30);
    delay(20);
  }

  delay(20);
  noTone(this->buzzer); // durch das Ausstellen des Tons auf diesem Pin kann auf einem anderen Pin ein Ton ausgegeben werden
}
