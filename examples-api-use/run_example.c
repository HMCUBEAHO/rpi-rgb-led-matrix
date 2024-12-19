#include <stdio.h>
#include <stdlib.h>

void runExample(int userInput) {
    // Der Grundbefehl, der nicht verändert wird
    const char *baseCommand = "sudo ./demo --led-rows=64 --led-cols=64 --led-chain=6 --led-gpio-mapping=adafruit-hat --led-no-hardware-pulse --led-slowdown-gpio=4 --led-brightness=40 -D ";
    
    // Erstelle ein Puffer für den vollständigen Befehl
    char command[256];
    
    // Erstelle den vollständigen Befehl durch Anhängen der Benutzereingabe
    snprintf(command, sizeof(command), "%s%d", baseCommand, userInput);
    
    // Ausgabe des finalen Befehls
    printf("Führe folgenden Befehl aus: %s\n", command);

    // Führe den Befehl aus
    int result = system(command);

    // Überprüfen, ob der Befehl erfolgreich war
    if (result != 0) {
        printf("Fehler beim Ausführen des Befehls!\n");
    }
}

int main() {
    int userChoice;

    // Fordere den Benutzer zur Eingabe einer Zahl auf
    printf("Bitte geben Sie eine Zahl ein (z. B. 10): ");
    scanf("%d", &userChoice);

    // Führe das Beispiel aus
    runExample(userChoice);

    return 0;
}
