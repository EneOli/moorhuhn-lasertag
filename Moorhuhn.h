/*
 * Moorhuhn.h - Bibliothek zur Festlegung von Moorhühnern
 * Fähigkeiten: Leben durch LED anzeigen, Leben/Tod abfragen (LDR), Fliegen (Bewegung) und Sterben (Ton)
 * V0.2, 12.04.2017
 */

#ifndef Moorhuhn_h
#define Moorhuhn_h
#include "Arduino.h"

class Moorhuhn {
  public:
    Moorhuhn(int nr, int led, int servoNr, int buzzer, boolean alive);
    void live(boolean alive);
    boolean shot(int ldr_ref);
    boolean isAlive();
    void fly();
    void die();

  private:
    int led;
    int buzzer;
    int servoNr;
    int nr;
    boolean alive;
};

#endif
