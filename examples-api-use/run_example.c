#include <string>

using namespace std;

// Funktion zum Ausführen des gewählten Beispiels mit der Benutzereingabe als Parameter
void runExample(int userInput) {
    // Der Grundbefehl, der nicht verändert wird
    string baseCommand = "sudo ./demo --led-rows=64 --led-cols=64 --led-pixel-mapper \"U-mapper;Rotate:0\" --led-gpio-mapping=adafruit-hat --led-brightness=40 -D ";

    // Der gesamte Befehl wird durch Anhängen der Benutzereingabe gebildet
    string command = baseCommand + to_string(userInput);

    // Ausgabe des finalen Befehls
    cout << "Führe folgenden Befehl aus: " << command << endl;

    // Führe den Befehl aus
    int result = system(command.c_str());

    // Überprüfen, ob der Befehl erfolgreich war
    if (result != 0) {
        cout << "Fehler beim Ausführen des Befehls!" << endl;
    }
}

int main() {
    int userChoice;

    // Fordere den Benutzer zur Eingabe einer Zahl auf
    cout << "Bitte geben Sie eine Zahl ein (z. B. 10): ";
    cin >> userChoice;

    // Führe das Beispiel aus
    runExample(userChoice);

    return 0;
}
