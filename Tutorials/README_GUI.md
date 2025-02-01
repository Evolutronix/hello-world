


💾 Commando: PROGMEM, aantal_sequences
Met het PROGMEM-commando kunnen we bewegingssequenties uit het flash-geheugen laden. Dit bespaart RAM-geheugen en zorgt ervoor dat we meerdere vooraf gedefinieerde loops kunnen gebruiken zonder het programma te vertragen.

📌 Gebruik van het PROGMEM-commando
Kopiëren
Bewerken
PROGMEM, aantal_sequences
aantal_sequences → Het aantal sequences dat geladen moet worden vanuit PROGMEM.
🎯 Voorbeeld
Kopiëren
Bewerken
PROGMEM, 3
🚀 Dit laadt de eerste 3 sequences uit het flash-geheugen en maakt ze beschikbaar voor gebruik.


📂 Hoe werkt het?
De functie reset de huidige sequences.
Leest de sequenties uit PROGMEM (flash-geheugen) en zet deze in een lijst.
Elke sequence bevat paren van positie en snelheid.
Controleert of de array niet over de limiet gaat.
Geeft een bevestigingsbericht op de seriële monitor.
