/*
 * Evolutronix-robotics
 * 
 *  
 * (c) Copyright 2025 - Cyrill Breukelaar
 * This code  is copyrighted. It is not allowed to use these  commerically
 * without explicit written approval
 * 
 *PCA9685 Servo-controller with GUI 
 *StandAlone with serial commands
 *<SEQ_SAVE, sequence1,180,5,>
 * PIN CONNECTIONS
 * 
 * SDA is the serial data
 * SCL is the serial clock
 *
 * GND --> GND white
 * VCC --> 5V  grey
 * SDA --> A4  red
 * SCL --> A5  blue
 * 
 * Hardware Serial1
 * Rx-- > 0 Tx
 * Tx -- > 1 Rx
 * GND -- > GND
 * 3,3V --> 3,3V
 *
 * Uno, Ethernet    A4 (SDA), A5 (SCL)
 * Mega2560         20 (SDA), 21 (SCL)
 * Leonardo          2 (SDA),  3 (SCL)
 * Due              20 (SDA), 21 (SCL) or SDA1, SCL1
 *
 */






#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>

#define SEQUENCE_SIZE 22 
const uint8_t walkSequences[][SEQUENCE_SIZE] PROGMEM = {
    {120, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},


    // Add more sequences as needed

    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},
    {45, 5, 110, 6, 20, 6, 45, 10, 160, 10, 90, 5, 180, 6, 120, 6, 45, 10, 160, 10, 120, 20},

};


//SoftwareSerial Bluetooth(3, 2); // HC-05: TX op pin 3, RX op pin 2

// Calculate the number of walk sequences
const int NUM_WALK_SEQUENCES = sizeof(walkSequences) / sizeof(walkSequences[0]);



// Maak een object voor de PWMServoDriver (PCA9685)
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Servo min en max PWM waarden (aanpassen indien nodig)
#define SERVO_MIN  100  // Min PWM waarde voor de servo (0 graden)   120
#define SERVO_MAX  510  // Max PWM waarde voor de servo (180 graden) 600

// Functie om een hoek om te zetten naar een PWM waarde
int angleToPWM(int angle) {
  return map(angle, 0, 170, SERVO_MIN, SERVO_MAX);
}



// StateMachine-klasse voor servo-aansturing via PCA9685

//=====================================================================================

class StateMachine {
  uint8_t servoChannel;
  uint8_t currentPosition;
  uint8_t targetPosition;
  uint8_t increment;
  uint8_t updateInterval;
  unsigned long lastUpdate;

public:
  StateMachine(int channel, int interval) {
    servoChannel = channel;
    updateInterval = interval;
    increment = 1;
    currentPosition = 90;  // Startpositie
    targetPosition = 90;   // Startpositie
    lastUpdate = 0;
  }

  void Update() {
    if ((millis() - lastUpdate) > updateInterval) {
      lastUpdate = millis();

      if (currentPosition < targetPosition) {
        currentPosition += increment;
        if (currentPosition > targetPosition) currentPosition = targetPosition;
      } else if (currentPosition > targetPosition) {
        currentPosition -= increment;
        if (currentPosition < targetPosition) currentPosition = targetPosition;
      }

      pwm.setPWM(servoChannel, 0, angleToPWM(currentPosition));
    }
  }

  void setTargetPosition(int position, int speed) {
    targetPosition = position;
    increment = speed > 0 ? speed : 1;  // Zorg dat de snelheid positief is
  }

  bool isAtTarget() {
    return currentPosition == targetPosition;
  }
uint8_t getCurrentPosition() const {
    return currentPosition;
}


};

// Aantal servo's en hun instellingen
#define NUM_SERVOS 11
StateMachine servos[NUM_SERVOS] = {
  StateMachine(0, 20),
  StateMachine(1, 20),
  StateMachine(2, 20),
  StateMachine(3, 20),
  StateMachine(4, 20),
  StateMachine(5, 20),
  StateMachine(6, 20),
  StateMachine(7, 20),
  StateMachine(8, 20),
  StateMachine(9, 20),
  StateMachine(10, 20)
  
};

// Structuur voor een enkele Servo-actie
struct ServoAction {
  uint8_t servoId;
  uint8_t position;
  uint8_t speed;
};

// Structuur voor een Sequence
struct Sequence {
  String name;
  ServoAction servoActions[11]; // Maximaal 11 servos per sequence
  uint8_t actionCount;             // Aantal acties in deze sequence
};

// Maximaal aantal sequences
#define MAX_SEQUENCES 16
Sequence sequences[MAX_SEQUENCES];
uint8_t sequenceCount = 0; // Huidig aantal opgeslagen sequences

// Variabelen voor de state machine
enum State { IDLE, PLAY_SEQUENCE };
State currentState = IDLE;
unsigned long stateChangeTime = 0;
uint8_t currentSequence = 0;


//======================================================================================

// CommandHandler Class
class CommandHandler {
public:



  void processCommand(String command) {
    if (command.length() < 3) return;
    command = command.substring(1, command.length() - 1);

    int firstComma = command.indexOf(',');
    String mainCommand = (firstComma != -1) ? command.substring(0, firstComma) : command;
    String parameters = (firstComma != -1) ? command.substring(firstComma + 1) : "";

    if (mainCommand == "SEQ_SAVE") {
      Serial.println(F("seq ontvangen"));
      processSeqSave(parameters);
       } else if (mainCommand == "SEQ_PROG") {
        uint8_t sequencesToLoad = parameters.toInt();
        if (sequencesToLoad <= 0 || sequencesToLoad > NUM_WALK_SEQUENCES) {
            Serial.println(F("Ongeldig aantal sequences!"));
        } else {
            processSeqProg(sequencesToLoad); // Roep processSeqProg aan
        }
    } else if (mainCommand == "PLAY") {
        currentState = PLAY_SEQUENCE;
        stateChangeTime = millis(); // Reset de timer
    } else if (mainCommand == "SLIDE") {
        slider(parameters); // Nieuwe functie om te stoppen      
    } else if (mainCommand == "STOP") {
        stopSequence(); // Nieuwe functie om te stoppen  
    } else {
      Serial.println("Onbekend commando!");
    }
  }

private:

  void processSeqSave(String parameters) {
    if (sequenceCount >= MAX_SEQUENCES) return;
        Serial.println(F("in save"));
    int firstComma = parameters.indexOf(',');
    if (firstComma == -1) return;

    String sequenceName = parameters.substring(0, firstComma);
    parameters = parameters.substring(firstComma + 1);

    Sequence newSequence;
    newSequence.name = sequenceName;
    newSequence.actionCount = 0;

    while (parameters.length() > 0 && newSequence.actionCount < 12) {
      uint8_t commaIndex1 = parameters.indexOf(',');
      if (commaIndex1 == -1) break;
      uint8_t position = parameters.substring(0, commaIndex1).toInt();
      parameters = parameters.substring(commaIndex1 + 1);

      uint8_t commaIndex2 = parameters.indexOf(',');
      uint8_t speed;
      if (commaIndex2 != -1) {
        speed = parameters.substring(0, commaIndex2).toInt();
        parameters = parameters.substring(commaIndex2 + 1);
      } else {
        speed = parameters.toInt();
        parameters = "";
      }

      ServoAction action = {newSequence.actionCount, position, speed};
      newSequence.servoActions[newSequence.actionCount] = action;
      newSequence.actionCount++;
    }

    sequences[sequenceCount] = newSequence;
    sequenceCount++;
  }

  void startPlayback() {
    if (sequenceCount == 0) return;

    Serial.println(F("Start sequence playback..."));
    currentSequence = 0;
    playSequence(currentSequence);
    currentState = PLAY_SEQUENCE;
  }



void loadSequencesFromProgmem(uint8_t sequencesToLoad) {
    sequenceCount = 0; // Reset huidige sequences
    for (uint8_t i = 0; i < sequencesToLoad && i < NUM_WALK_SEQUENCES; i++) {
        Sequence newSequence;
        newSequence.name = "PROGMEM_" + String(i); // Geef een naam gebaseerd op het indexnummer
        newSequence.actionCount = SEQUENCE_SIZE / 2; // Elke sequence bevat positie/snelheid-paren

        for (int j = 0; j < SEQUENCE_SIZE; j += 2) {
            uint8_t position = pgm_read_byte(&walkSequences[i][j]);
            uint8_t speed = pgm_read_byte(&walkSequences[i][j + 1]);
            ServoAction action = {j / 2, position, speed};
            newSequence.servoActions[j / 2] = action;
        }

        sequences[sequenceCount++] = newSequence; // Voeg toe aan sequence-array
        if (sequenceCount >= MAX_SEQUENCES) break; // Zorg dat we niet buiten de array gaan
    }

    Serial.print(F("Succesvol "));
    Serial.print(sequencesToLoad);
    Serial.println(F(" sequences geladen uit PROGMEM!"));
}

void processSeqProg(uint8_t sequencesToLoad) {
  loadSequencesFromProgmem(sequencesToLoad);
  currentState = PLAY_SEQUENCE; // Zet de status naar PLAY_SEQUENCE om de sequences af te spelen
  stateChangeTime = millis();   // Reset de timer
  Serial.println(F("Start sequence playback na PROGMEM load"));
}

    // Functie om een SLIDER-commando te verwerken
    void slider(String parameters) {
        // Splits de parameters op basis van komma's
        int commaIndex = parameters.indexOf(',');
        if (commaIndex == -1) {
            Serial.println("Fout: Onjuiste SLIDER parameters.");
            return;
        }

        // Haal de servo ID en de gewenste positie uit de parameters
        String servoIdStr = parameters.substring(0, commaIndex);
        String positionStr = parameters.substring(commaIndex + 1);

        // Zet de servo ID en positie om naar integers
        int servoId = servoIdStr.toInt();
        int position = positionStr.toInt();

        // Controleer of de servo ID geldig is
        if (servoId >= 0 && servoId < NUM_SERVOS) {
            // Converteer de positie naar ticks en stuur naar de PCA9685
            int ticks = angleToPWM(position);
            pwm.setPWM(servoId, 0, ticks);

            // Bevestiging naar de seriële monitor
            Serial.print("Servo ");
            Serial.print(servoId);
            Serial.print(" ingesteld op ");
            Serial.print(position);
            Serial.println(" graden.");
        } else {
            Serial.println("Fout: Ongeldig servo ID.");
        }
    }



void stopSequence() {
    Serial.println(F("Sequence playback gestopt. Terug naar IDLE."));
    currentState = IDLE;

    for (int i = 0; i < NUM_SERVOS; i++) {
        servos[i].setTargetPosition(servos[i].getCurrentPosition(), 0);
    }
}



};

//=========================================================================================

CommandHandler commandHandler;
String inputBuffer = "";




// Functie om een sequence te starten
void playSequence(uint8_t sequenceIndex) {
  Sequence sequence = sequences[sequenceIndex];
  for (uint8_t i = 0; i < sequence.actionCount; i++) {
    ServoAction action = sequence.servoActions[i];
    servos[action.servoId].setTargetPosition(action.position, action.speed);
  }
}

// Controleer of alle servo's op hun doel zijn
bool allServosAtTarget() {
  for (uint8_t i = 0; i < NUM_SERVOS; i++) {
    if (!servos[i].isAtTarget()) {
      return false;
    }
  }
  return true;
}
//==================================================================================

void display_freeram() {
  Serial.print(F("- SRAM left: "));
  Serial.println(freeRam());
}

int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  
    ? (int)&__heap_start : (int) __brkval);  
}

//=====================================================================================


void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(50);
  Serial1.begin(9600);

  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].setTargetPosition(90, 10);
    servos[i].Update();
  }




//display_freeram();

}

void loop() {
  
  
  if (Serial.available()) {
    char received = Serial.read();
    if (received == '>') {
      inputBuffer += received;
      commandHandler.processCommand(inputBuffer);
      inputBuffer = "";
    } else if (received == '<') {
      inputBuffer = "<";
    } else {
      inputBuffer += received;
      
    }
  }

      if (Serial1.available()) {
    char received = Serial1.read();
    if (received == '>') {
      inputBuffer += received;
      Serial.println(inputBuffer);
      commandHandler.processCommand(inputBuffer);
      inputBuffer = "";
    } else if (received == '<') {
      inputBuffer = "<";
    } else {
      inputBuffer += received;
      
    }
  }




  switch (currentState) {
    case PLAY_SEQUENCE:
      for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        servos[i].Update();
      }

      if (allServosAtTarget()) {
        currentSequence = (currentSequence + 1) % sequenceCount;
        playSequence(currentSequence);
      }
      break;

    default:
      break;
  }
}
