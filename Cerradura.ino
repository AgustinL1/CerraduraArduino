#include <Keypad.h>
#include <LiquidCrystal.h>

// Inicialización del LCD
LiquidCrystal lcd(10, 11, 12, 13, A0, A1);

// Pines para los LEDs
const int redLed = A2;
const int greenLed = A3;

// Variables para manejar las contraseñas
String password = "123456";
String inputPassword = "";
String newPassword = "";

// Estado de la puerta y de la contraseña
bool isDoorOpen = false;
bool correctPassword = false;
bool changePasswordMode = false;

// Temporizador para el tiempo de apertura
unsigned long unlockTime = 0;
const unsigned long unlockDuration = 30000; // Duración para mantener la puerta abierta
bool timerEvent=false;

// Contador para el botón A
uint8_t count = 0;

// Configuración del teclado
const byte ROW_NUM = 4;
const byte COL_NUM = 4;
char keys[COL_NUM][ROW_NUM] = {
  { '1', '4', '7', '*' },
  { '2', '5', '8', '0' },
  { '3', '6', '9', '#' },
  { 'A', 'B', 'C', 'D' }
};
byte pin_rows[ROW_NUM] = { 2, 3, 4, 5 };
byte pin_column[COL_NUM] = { 6, 7, 8, 9 };
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COL_NUM);

void setup() {
  // Configuraciones iniciales
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  lcd.print("Ingrese clave:");
}

void loop() {
  // Escucha tecla presionada en el teclado
  char key = keypad.getKey();
  if (key) {
    handleKeyPress(key);
  }
  
  // Si está en el evento de temporizador
  if (timerEvent) {
    lcd.clear();
    lcd.print("Tiempo: ");
    lcd.setCursor(0, 1);
    const int seconds = millis() - unlockTime;
    String timeString = String((unlockDuration - seconds)/1000) + " s";
    lcd.print(timeString);
    delay(1000);
  }

  // Verifica si hay datos en el puerto serial
  if (Serial.available() > 0) {
    String serialInput = Serial.readString();
    if (serialInput.length() == 6) {
      password = serialInput;
      lcd.clear();
      lcd.print("Clave cambiada");
    }
  }

  // Verifica si debe cerrar la puerta
  if (isDoorOpen && !correctPassword) {
    isDoorOpen = false;
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  }

  // Verifica si el tiempo de apertura ha expirado
  if (isDoorOpen && (millis() - unlockTime > unlockDuration) && timerEvent) {
    isDoorOpen = false;
    correctPassword = false;
    timerEvent=false;
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
    lcd.clear();
    lcd.print("Puerta cerrada");
  }
}

// Función para manejar tecla presionada
void handleKeyPress(char key) {
  lcd.clear();
  if (changePasswordMode) {
    handlePasswordChange(key);
    return;
  }
  
  // Restablece contador si una tecla diferente a 'A' es presionada
  if (key != 'A') {
    count = 0;
  }
  
  // Otras funciones para manejar teclas
  if (key == '*') { clearPassword(); return; }
  if (key == '#') { checkPassword(); return; }
  if (key == 'A') { togglePasswordChangeMode(); return; }
  if (key == 'B') { handleBButton(); return; }
  if (key == 'C') { handleCButton(); return; }
  if (key == 'D') { handleDButton(); return; }
  if (isDigit(key)) { accumulatePassword(key); return; }
}

// Función para manejar el cambio de contraseña
void handlePasswordChange(char key) {
  // Si se presiona '*', se borra la nueva contraseña
  if (key == '*') {
    newPassword = "";
    lcd.print("Borrando...");
    return;
  }
  // Si se presiona '#', se confirma la nueva contraseña
  if (key == '#') {
    // Se verifica si la nueva contraseña tiene 6 caracteres
    if (newPassword.length() == 6) {
      password = newPassword; // Se cambia la contraseña
      newPassword = "";
      changePasswordMode = false; // Se sale del modo cambio de contraseña
      correctPassword = false; // Se reinicia la validación de contraseña
      lcd.print("Clave cambiada");
    } else {
      lcd.print("Clave incompleta");
    }
    return;
  }
  // Si se presiona 'D', se cancela el cambio de contraseña
  if (key == 'D') {
    changePasswordMode = false;
    newPassword = "";
    lcd.print("Salida sin cambios");
    return;
  }

  // Si no se presionan los botones anteriores, se acumula el caracter para la nueva contraseña
  newPassword += key;
  lcd.print("Nueva clave:");
  lcd.setCursor(0, 1);
  lcd.print(newPassword);
}

// Función para borrar la contraseña ingresada
void clearPassword() {
  inputPassword = "";
  lcd.print("Borrando...");
}

// Función para verificar si la contraseña ingresada es correcta
void checkPassword() {
  if (inputPassword == password) {
    correctPassword = true;
    lcd.clear();
    lcd.print("Acceso concedido");
  } else {
    correctPassword = false;
    lcd.clear();
    lcd.print("Acceso denegado");
  }
  inputPassword = "";
}

// Función para alternar entre el modo normal y el modo de cambio de contraseña
void togglePasswordChangeMode() {
  count++;
  if (correctPassword || count == 3) {
    changePasswordMode = true;
    newPassword = "";
    lcd.print("Modo cambio");
    lcd.setCursor(0, 1);
    lcd.print("clave");
  }
}

// Función para manejar el botón B
void handleBButton() {
  if (correctPassword) {
    lcd.print("Acceso OK");
    lcd.setCursor(0, 1);
    lcd.print("FIJO");
    isDoorOpen = true;
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
  } else {
    lcd.print("Y la clave?");
  }
}

// Función para manejar el botón C
void handleCButton() {
  if (correctPassword) {
    timerEvent = true;
    isDoorOpen = true;
    unlockTime = millis();
    lcd.print("Acceso OK ");
    lcd.setCursor(0, 1);
    lcd.print("30 SEGUNDOS");
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
  } else {
    lcd.print("Y la clave?");
  }
}

// Función para manejar el botón D
void handleDButton() {
  if (isDoorOpen) {
    lcd.print("Cerrando la puerta.");
    isDoorOpen = false;
    correctPassword = false;
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  } else {
    lcd.print("Puerta cerrada");
  }
}

// Función para verificar si un carácter es un dígito
bool isDigit(char key) {
  return key >= '0' && key <= '9';
}

// Función para acumular los caracteres de la contraseña ingresada
void accumulatePassword(char key) {
  inputPassword += key;
  lcd.print("Ingrese clave:");
  lcd.setCursor(0, 1);
  lcd.print(inputPassword);
}
