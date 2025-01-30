
this repository is for practicing.
to make a GUI in visual studio  C++/CLI to control servo's with arduino nano every and PCA9685 servo controller and HC-05 bluetooth.
Je ziet eerst de code waar mee begonnen is. Deze is makkelijker te begrijpen.
Daarna optimaliseren we de code en leggen uit waarom e.e.a. een verbetering is.

![Mijn afbeelding](evolutronix.png)


# Arduino Nano PCA9685_HC-05 Servo Controller

Dit programma is ontworpen voor gebruik met de **Evolutronics-robotics GUI servo-controller**. Het biedt een uitgebreide set functies om servo's aan te sturen, posities op te slaan als sequenties, en deze sequenties af te spelen. De sequenties kunnen ook in het PROGMEM van de Arduino worden geplaatst om standalone of via Bluetooth te worden afgespeeld. 

De volledige programmering is uitvoerig gedocumenteerd, waardoor het eenvoudig is om het programma verder te verbeteren of uit te breiden.

---

## Functionaliteiten

### Hoofdfuncties:
- **Servo-aansturing via GUI:** 
  Een serial command vanuit de GUI stuurt een specifieke servo naar de gewenste positie.
- **Sequentiebeheer:**
  - Posities kunnen worden opgeslagen als sequenties in een array.
  - Sequenties kunnen direct worden afgespeeld.
  - Stoppen van een sequentie-afspeelactie op elk moment.
- **Standalone sequentiebeheer via PROGMEM:**
  - Vanwege de beperkte geheugenruimte van de Arduino Nano worden sequenties opgeslagen in PROGMEM.
  - Deze sequenties kunnen standalone of via Bluetooth worden afgespeeld.
- **Bluetooth-functionaliteit:**
  - Met een HC-05 module kan de Arduino Nano ook zonder USB-aansluiting worden bediend.
  - Sequenties opgeslagen in PROGMEM kunnen via Bluetooth worden aangestuurd.

---

## Beschrijving van het proces

### Communicatie via de GUI:
1. De GUI stuurt een serial command naar de Arduino Nano.
2. De Arduino leest het command en voert de bijbehorende actie uit:
   - **SLIDE:** Een slider in de GUI stuurt de geselecteerde servo naar een specifieke positie via de PCA9685.
   - **SEQ_SAVE:** Sequenties van servo-posities worden opgeslagen in een array.
   - **PLAY:** De opgeslagen sequenties in de array worden afgespeeld.
   - **STOP:** Het afspelen van sequenties wordt direct gestopt.
   - **PROG:** Sequenties uit PROGMEM worden afgespeeld.

### Bluetooth-functionaliteit:
- De **HC-05 Bluetooth-module** maakt draadloze communicatie mogelijk.
- Sequenties opgeslagen in PROGMEM kunnen via Bluetooth worden afgespeeld, zodat de Arduino Nano ook zonder USB-verbinding kan worden gebruikt.

---

## Programma-architectuur

Het programma is opgesplitst in verschillende modules, elk verantwoordelijk voor een specifieke taak:

1. **Serial Module:**
   - Verantwoordelijk voor het ontvangen en verwerken van serial data.
   - Bepaalt welk command wordt uitgevoerd.
2. **Command Modules:**
   - **SEQ_SAVE:** Beheert het opslaan van sequenties.
   - **PLAY:** Zorgt voor het afspelen van sequenties uit een array.
   - **STOP:** Stopt onmiddellijk het afspelen van sequenties.
   - **SLIDE:** Aansturing van servo's via de GUI-slider.
   - **GROUP:** Verantwoordelijk voor het afspelen van PROGMEM-sequenties via serial of Bluetooth.

---

## Toekomstige uitbreidingen

Dit programma is ontwikkeld met het oog op flexibiliteit en uitbreidbaarheid. Nieuwe functies kunnen eenvoudig worden toegevoegd, zoals:
- Groepsbeheer voor sequenties.
- Geavanceerdere Bluetooth-commando's.
- Meer optimalisaties voor geheugengebruik.

---

Met dit programma biedt de **Arduino Nano PCA9685_HC-05 Servo Controller** een krachtige en veelzijdige oplossing voor servo- en robotica-projecten.

![Mijn afbeelding](flow_serial.png)

# C++ Example

```cpp
  while (Serial.available()) {
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

```



De inputBuffer sturen we vervolgens verder naar de commandHandler.procesCommand
Hier zal de String opgedeeld worden in het mainCommand en parameters.






![Mijn afbeelding](flow_command_a.png)

![Mijn afbeelding](flow_command_b.png)

# C++ Example

```cpp
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


```

### Command Processing: `SEQ_SAVE`

Daarna zal gekeken worden wat het `mainCommand` is en verder doorgestuurd worden naar de bijbehorende functie.

We kijken nu naar elk command, beginnend bij `SEQ_SAVE`.

#### Command: `SEQ_SAVE`

# C++ Example

```cpp
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

```





Het roept de functie `processSeqSave(String parameters)` aan. Deze functie werkt als volgt:

1. Controleert of het maximale aantal sequences is bereikt.  
   - Als dit het geval is, keert de functie terug.
2. Zo niet, dan zal de functie:
   - De string scheiden in de naam van de sequence en de rest van de string.
   - De `actionCount` resetten naar `0`.
3. Vervolgens controleert de functie:
   - Of `actionCount` kleiner is dan `12`.
   - En of de stringlengte groter is dan `0`.
4. Indien beide voorwaarden waar zijn:
   - Wordt de eerste positie en snelheid (`speed`) uit de string gehaald.
   - Deze waarden worden opgeslagen in `action[actionCount]` (positie en snelheid).
   - De `actionCount` wordt verhoogd.
   - De functie begint opnieuw, totdat de string leeg is **of** `actionCount > 12`.
5. Als de string leeg is of de limiet is bereikt:
   - Wordt de `sequenceCount` verhoogd.
   - En de functie begint met de volgende sequence.


