/* Moorhuhn-Lasertag V0.3, 14.06.2017
   von Sebastian Voss
*/

#include "Vector2.h"

// Bibliothek für die Moorhühner
#include "Moorhuhn.h"
// Bibliotheken für das LCD-Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // LCD mit SDA an A4, SCL an A5; 20 Spalten, 4 Zeilen

#define ClearScreen()   lcd.clear(); \
  lcd.setCursor(0,0); \

  // Definition der LED-Kette
#define LED_KETTE 0

  // Definition der Pins für die LEDs, die das Ziel angeben
#define LED_OFFSET 1

  // Definition des Piezo-Summers für das akustische Feedback
#define BUZZER_PIN 7

  // Definition der Servos für die Bewegung der Huehner
#define SERVO_OFFSET 8

  // Referenzwerte für die LDR
  // wenn das Spiel gestartet wird, wird ein Countdown heruntergezählt und am Ende des Countdowns, also direkt vor dem Spiel werden die Referenzwerte gemessen
  // damit die aktuelle Umgebungshelligkeit als Referenz dient und damit niemand mehr direkt vor dem Spiel steht und den Referenzwert durch seinen Schatten
  // verändert
  int ldr_ref;

  // struct für die Punkte der Spieler (maximal 2)

  typedef struct {
    int a;
    int b;
    int activePlayers;
  } Players;

  namespace global {
  Vector<Moorhuhn> huehner;
  Players players;
  float reactTime = 4.0; // Reaktionszeit zum Abschießen
  int hbew = 5; // Häufigkeit der Bewegungen
  }

  int newRandom(int oldRandom) {
    int z;
    do {
      z = random(6);
    } while (oldRandom == z);
    return z;
  }

  // ********************************************************** Spiele *********************************************************************************************
  namespace games {
  /* Am Anfang werden alle Moorhühner auf lebend gestellt und dann müssen diese möglichst schnell abgeschossen werden. Zurückgegeben wird die benötigte Zeit in Sekunden, gerundet
     auf zwei Nachkommastellen.
     Funktionsweise:
  */
  // TODO eventull optimieren.. ist so unschön
  // TODO zum laufen kriegen
  void killemall () {
    // bisherige Überlegungen/Skizzen
    // Hühner zum Leben erwecken: alive auf true und LED an
    global::huehner[0].live(true);
    global::huehner[1].live(true);

    int timer = 0;
    while (!global::huehner[0].shot(ldr_ref) && !global::huehner[1].shot(ldr_ref)) {
      if (timer % 2 == 0 && global::huehner[0].isAlive()) {
        global::huehner[0].fly();
      }
      if (timer % 2 == 1 && global::huehner[1].isAlive()) {
        global::huehner[1].fly();
      }
      timer++;
      delay(100);
    }

    for (int i = 0; i <= 1; i++) {
      if (global::huehner[i].shot(ldr_ref)) {
        global::huehner[i].live(false);
        global::huehner[i].die();
      }
    }
    // Danach wieder zurück zu Schleife mit den übrig gebliebenen Hühnern...

    do {} while (1); // infinity .. huuuuuiiii
  }

  /* Klassisches Moorhuhn: Es taucht ein Moorhuhn an zufälliger Stelle auf und muss möglichst schnell abgeschossen werden.
      Die Zeit vom Auftauchen bis zum Abschuss wird gezählt. Je nach Schnelligkeit gibt es 50, 30 oder 10 Punkte oder die Runde ist beendet.
      Die Grenzen für die Punkte werden je nach eingestellter Reaktionszeit gesetzt (Schwierigkeit 1). Wenn die Runde nicht beendet, sondern Punkte vergeben wurden,
      geht es weiter mit dem nächsten Moorhuhn. Zurückgegeben wird die Anzahl der Punkte. Außerdem kann man das Spiel zu zweit (abwechselnd) spielen.
      Die Bewegungshäufigkeit hbew gibt an, wie oft sich das Huhn bewegen soll (Schwierigkeit 2). Wenn hbew=1 ist, dann bewegt sich das Huhn alle 0.1 Sekunden,
      bei hbew=2 alle 0.2 Sekunden usw. Empfehlenswert ist also hbew von 10 bis 20.
  */

  void classic (int sp, float react_time, int hbew) {
    ClearScreen();

    lcd.print("Spielstart in ..."); // Countdown
    for (int i = 3; i > 0; i--) {
      lcd.setCursor(9, 1);
      lcd.print(i);
      delay(1000);
    }

    ClearScreen();
    lcd.print("Moorhuhn klassisch"); // Überschrift

    // setup
    global::players.a = 0;
    global::players.b = 0;
    int znr = 10; // Zufällige Nummer eines Huhns - Start mit 10, damit der Zufallsgenerator unten diese Zahl auf jeden Fall überschreibt
    float zeit; // gemessene Reaktionszeit
    int schleife = 1; // Schleifenzähler für abwechselndes Spielen
    do {
      int dran = schleife % sp; // ermittle, welcher Spieler dran ist
      ClearScreen();
      lcd.print("Spieler " + String(dran + 1) + " ist dran.");

      znr = newRandom(znr);

      ldr_ref = analogRead(A0); // Referenzwert für die Umgebungshelligkeit einlesen
      global::huehner[znr].live(true); // Huhn zum Leben erwecken: LED an

      // Solange das Huhn nicht abgeschossen wurde, soll es sich immer wieder bewegen
      // hbew = 1 -> Huhn bewegt sich jede zweite Runde, also alle 100 Millisekunden
      // hbew = 2 -> Huhn bewegt sich jede vierte Runde, also alle 200 Millisekunden
      // hbew = n -> Huhn bewegt sich jede 2n-te Runde, also alle n*100 Millisekunden
      int timer = 0;
      unsigned long start = millis(); // Start der Zeitmessung
      while (!global::huehner[znr].shot(ldr_ref)) {
        if (timer % (2 * hbew) == 0) {
          global::huehner[znr].fly();
        }
        delay(50);
        timer++;
      }
      unsigned long ende = millis(); // Ende der Zeitmessung

      // TODO ein Befehl
      global::huehner[znr].live(false); // LED aus
      global::huehner[znr].die(); // Sterbeton

      zeit = (ende - start) / 1000.; // Reaktionszeit berechnen

      // Punkte ermitteln
      int points = 0;
      if (zeit <= 0.33 * global::reactTime) {
        points = 50;
        lcd.setCursor(8, 2);
        lcd.print("+50!");
      } else if (0.33 * global::reactTime < zeit && zeit <= 0.66 * global::reactTime) {
        points = 30;
        lcd.setCursor(8, 2);
        lcd.print("+30!");
      } else if (0.66 * react_time < zeit && zeit <= react_time) {
        points = 10;
        lcd.setCursor(8, 2);
        lcd.print("+10!");
      } else if (react_time < zeit) {
        lcd.setCursor(5, 2);
        lcd.print("Zu langsam!");
      }
      if (dran == 0) {
        global::players.a += points;
      } else {
        global::players.b += points;
      }
      // Punkte ausgeben
      lcd.setCursor(0, 3);
      lcd.print("Punkte v. Sp." + String(dran + 1) + ":");
      if (dran == 0) {
        lcd.print(global::players.a);
      } else {
        lcd.print(global::players.b);
      }
      delay(2000);
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      schleife++;
    } while (zeit <= global::reactTime);
  }
  }

  // **************************************************************** Menüführung *******************************************************************************
  int lcd_key     = 0;
#define T_OFFSET 100

  int read_buttons() {
    int key_in = analogRead(A3);  // Wert für die Taster einlesen
    if (key_in > 440) return T_OFFSET; // Wenn alle offen sind, ist der Widerstand hoch
    if (key_in > 410) return T_OFFSET + 1;  // t1 liegt bei 426
    if (key_in > 370) return T_OFFSET + 2; // t2 liegt bei 393
    if (key_in > 310) return T_OFFSET + 3;  // t3 liegt bei 342
    return T_OFFSET;  // wenn alle anderen nicht hinhauen, ist kein Taster gedrückt (eigentlich sind aber mehrere gedrückt)
  }

  /* Einstellung der Reaktionszeit, die man maximal hat, um das Moorhuhn abzuschießen.
     Wenn man diese einhält, bekommt man 10 Punkte. Wenn man es sogar in weniger als 2/3 dieser Zeit schafft, bekommt man 30 Punkte.
     Wenn man es sogar in weniger als 1/3 dieser Zeit schafft, bekommt man 50 Punkte.
     Die Variable wird global eingestellt, daher braucht die Funktion den Wert nicht zurückgeben.
  */
  void rt_settings() {
    ClearScreen();
    lcd.print("Max. Reaktionszeit");
    lcd.setCursor(0, 1);
    lcd.print("(fuer 10 Punkte)");
    lcd.setCursor(9, 2);
    lcd.print(global::reactTime, 1);
    lcd.print("s");
    lcd.setCursor(0, 3);
    lcd.print("--     Zurueck    ++");
    while (read_buttons() != T_OFFSET + 2) {
      while (read_buttons() == T_OFFSET) {}
      lcd_key = read_buttons();
      if (lcd_key == T_OFFSET + 1 && global::reactTime > 1.0) {
        global::reactTime--;
      } else if ( lcd_key == T_OFFSET + 3 && global::reactTime < 11.0) {
        global::reactTime++;
      }
      lcd.setCursor(9, 2);
      lcd.print("     ");
      lcd.setCursor(9, 2);
      lcd.print(global::reactTime, 1);
      lcd.print("s");
      delay(500);
    }
  }

  /* Die Bewegungshäufigkeit gibt an, wie oft sich das Huhn bewegt. Dabei bewegt es sich weniger oft, wenn die Bewegungshäufigkeit hbew größer wird.
      hbew gibt an, dass sich das Moorhuhn ein Mal alle hbew*100 Millisekunden bewegt. Wenn hbew = 10 ist, bewegt sich das Moorhuhn also einmal in 1s.
      Wohin sich das Moorhuhn bewegt, wird im Spiel zufällig ausgewählt.
  */
  void bh_settings() {
    int bh = global::hbew * 100;
    ClearScreen();
    lcd.print("Bewegungshaeufigkeit");
    lcd.setCursor(0, 1);
    lcd.print("Huhn bewegt sich");
    lcd.setCursor(0, 2);
    lcd.print("ein Mal in");
    lcd.setCursor(11, 2);
    lcd.print(bh);
    lcd.print("ms");
    lcd.setCursor(0, 3);
    lcd.print("--     Zurueck    ++");
    delay(500);
    while (read_buttons() != T_OFFSET + 2) {
      while (read_buttons() == T_OFFSET) {}
      lcd_key = read_buttons();
      if (lcd_key == T_OFFSET + 1 && global::hbew > 1) {
        global::hbew--;
      } else if ( lcd_key == T_OFFSET + 3 && global::hbew < 20) {
        global::hbew++;
      }
      bh = global::hbew * 100;
      lcd.setCursor(11, 2);
      lcd.print("      ");
      lcd.setCursor(11, 2);
      lcd.print(bh);
      lcd.print("ms");
      delay(500);
    }
  }

  void setup() {
    // LCD initialisieren
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);

    // Hintergrundbeleuchtung (LED-Kette)
    pinMode(LED_KETTE, OUTPUT);
    digitalWrite(LED_KETTE, HIGH);

    for (int i = 0; i < 6; i++) {
      global::huehner.push_back(Moorhuhn(i, LED_OFFSET + i, SERVO_OFFSET + i, BUZZER_PIN, false));
    }
    global::players.activePlayers = 0;
  }

  void loop() {
    // Startbildschirm aufbauen
    ClearScreen();
    lcd.print(" Moorhuhn-Lasertag! ");
    lcd.setCursor(0, 1);
    lcd.print("1: Moorhuhn Classic");
    lcd.setCursor(0, 2);
    lcd.print("2: Kill 'em all");
    lcd.setCursor(0, 3);
    lcd.print("3: Einstellungen");
    delay(500);
    while (read_buttons() == T_OFFSET) {}
    lcd_key = read_buttons();
    switch (lcd_key) {
      case T_OFFSET + 1:
        { // Es wurde das Spiel Classic gewählt. Bevor es losgeht, muss die Anzahl der Spieler festgelegt werden.
          ClearScreen();
          lcd.print("Klassisches Moorhuhn");
          lcd.setCursor(0, 1);
          lcd.print("   Spieleranzahl:   ");
          while (read_buttons() != T_OFFSET + 2) {
            lcd.setCursor(9, 2);
            lcd.print(global::players.activePlayers);
            if (global::players.activePlayers == 1) {
              lcd.setCursor(0, 3);
              lcd.print("                    ");
              lcd.setCursor(9, 3);
              lcd.print("Ok      ++");
            } else if (global::players.activePlayers == 2) {
              lcd.setCursor(0, 3);
              lcd.print("                    ");
              lcd.setCursor(0, 3);
              lcd.print("--       Ok");
            }
            while (read_buttons() == T_OFFSET) {}
            lcd_key = read_buttons();
            if (lcd_key == T_OFFSET + 1 && global::players.activePlayers > 1) {
              global::players.activePlayers--;
            } else if (lcd_key == T_OFFSET + 3 && global::players.activePlayers < 2) {
              global::players.activePlayers++;
            } else if (lcd_key == T_OFFSET + 2) {
              break;
            }
          }
          // Es wurde die Spieleranzahl festgelegt und es kann losgehen
          games::classic(global::players.activePlayers, global::reactTime, global::hbew); // eigentlich ist nicht nötig, die Variablen mitzugeben, weil sie global definiert sind?!
          // Aber es ist leichter verständlich, sie mitzugeben
          // Das Spiel ist zu Ende und es werden die Ergebnisse ausgegeben.
          ClearScreen();
          if (global::players.activePlayers == 1) {
            lcd.print("Ergebnis:");
            lcd.setCursor(0, 1);
            lcd.print("Du hast ");
            //lcd.print(counter[1]);
            lcd.print(global::players.a);
            lcd.print(" Punkte!");
            lcd.setCursor(0, 2);
            lcd.print("       Super!      ");
          } else {
            lcd.print("Ergebnisse:");
            lcd.setCursor(0, 1);
            lcd.print("Spieler1: ");
            //lcd.print(counter[1]);
            lcd.print(global::players.a);
            lcd.print("Pkt");
            lcd.setCursor(0, 2);
            lcd.print("Spieler2: ");
            //lcd.print(counter[2]);
            lcd.print(global::players.b);
            lcd.print("Pkt");
          }
          for (int i = 0; i < 3; i++) { // LED-Kette blinkt 3 Mal
            digitalWrite(LED_KETTE, LOW);
            delay(500);
            digitalWrite(LED_KETTE, HIGH);
            delay(500);
          }
          lcd.setCursor(0, 3);
          lcd.print("Weiter in...");
          for (int i = 5; i > 0; i--) {
            lcd.setCursor(13, 3);
            lcd.print(i);
            delay(1000);
          }
          break;
        }
      case T_OFFSET + 2:
        {
          // Es wurde das Spiel Kill 'em all ausgewählt
          ClearScreen();
          lcd.print("Error 404: Game not");
          lcd.setCursor(0, 1);
          lcd.print("found.");
          lcd.setCursor(0, 3);
          lcd.print("Weiter in...");
          for (int i = 5; i > 0; i--) {
            lcd.setCursor(13, 3);
            lcd.print(i);
            delay(1000);
          }
          break;
        }
      case T_OFFSET + 3:
        { // Es wurde der Einstellungsdialog aufgerufen
          ClearScreen();
          lcd.print("    Einstellungen   ");
          lcd.setCursor(0, 1);
          lcd.print("1: Reaktionszeit");
          lcd.setCursor(0, 2);
          lcd.print("2: Bewegung");
          lcd.setCursor(6, 3);
          lcd.print("Zurueck");
          delay(500);
          while (read_buttons() == T_OFFSET) {}
          lcd_key = read_buttons();
          if (lcd_key == T_OFFSET + 1) {
            rt_settings();
          } else if (lcd_key == T_OFFSET + 2) {
            bh_settings();
          } else if (lcd_key == T_OFFSET + 3) {
            break;
          }
        }
    }
  }
