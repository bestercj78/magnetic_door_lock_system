/*
  Author: CJ Bester
  Date Completed: 2022/08/06

  This sketch was created so that I could add a magnetic door lock to my wood working shop.
  I found the original code written by Saddam Khan and then modified it to suite my needs.
  Original link: https://circuitdigest.com/microcontroller-projects/fingerprint-based-biometric-security-system-arduino-uno

  This skecth includes the following libraries:
  Adafruit_Fingerprint
  LiquidCrystal_I2C


  The LiquidCrystal_I2C library can be found here using the following
  link: https://github.com/bestercj78/LiquidCrystal_I2C.git
  
*/

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); //Serial Communications

// Define Variables
uint8_t id;
int openLight   = 4;
int errorLight  = 5;
int closeLight  = 6;
int relayPin    = 7;
int Enroll      = 8;
int del         = 9;
int up          = 10;
int down        = 11;
int rel         = 12;

//Created instances
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  delay(1000);
  pinMode(Enroll, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(del, INPUT_PULLUP);
  pinMode(rel, INPUT_PULLUP);
  pinMode(openLight, OUTPUT);
  pinMode(closeLight, OUTPUT);
  pinMode(errorLight, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  lcd.begin(16, 2);

  // very important to set the backlight, or esle you will not see anything on your LCD screen
  lcd.backlight();
  lcd.print("Finger Print");
  lcd.setCursor(0, 1);
  lcd.print("Security System");

  delay(2000);

  finger.begin(57600);
  Serial.begin(9600);
  lcd.clear();
  lcd.print("Finding Module");
  lcd.setCursor(0, 1);

  delay(1000);

  //Looking for the Finger print sensor
  if (finger.verifyPassword()) {

    Serial.println("Found fingerprint sensor!");
    lcd.clear();
    lcd.print("Found Module ");
    delay(1000);
  } else {

    Serial.println("Did not find fingerprint sensor :(");
    lcd.clear();
    lcd.print("module not Found");
    lcd.setCursor(0, 1);
    lcd.print("Check Connections");
    while (1);
  }
}

void loop() {

  //Start your system
  lcd.setCursor(0, 0);
  lcd.print("Press UP/Down ");
  lcd.setCursor(0, 1);
  lcd.print("to start System");
  digitalWrite(closeLight, HIGH);

  //Checking if the up or down buttons were pressed
  if (digitalRead(up) == HIGH || digitalRead(down) == HIGH) {

    //Requesting to scan your fingerprint.
    for (int i = 0; i < 5; i++) {
      lcd.clear();
      lcd.print("Place Finger");
      delay(2000);

      //Fecthing the images
      int result = getFingerprintIDez();

      //Checking if the result for the fingerprint collection was successful
      if (result >= 0) {
        //Difine input user
        int inputUser = 0;

        //Fetching the exact fingerprint and calling the open door function
        switch (finger.fingerID) {

          case 1:
            inputUser = 1;
            openDoor(inputUser);
            break;
        }
      }
    }
  } else if (digitalRead(rel) == LOW) {  //Press release button to open the door if you are inside the workshop
    digitalWrite(openLight, HIGH);
    digitalWrite(closeLight, LOW);
    lcd.clear();
    lcd.print("Door Open");
    digitalWrite(relayPin, HIGH);
    delay(10000);
    digitalWrite(relayPin, LOW);
    digitalWrite(closeLight, HIGH);
    digitalWrite(openLight, LOW);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Door Closed   ");
    delay(3000);
  }

  //Checking if the Enroll or Delete buttons were pushed.
  checkKeys();
  delay(1000);
}

//Opens the door if the user was successfully authenticated
int openDoor(int inputUser) {
  //Defining the user string
  String user = "";

  //Checking who the user is and assigning a username to the input variable
  switch (inputUser) {

    case 1:
      user = "CJ";
      break;
  }

  //Opening the door
  digitalWrite(openLight, HIGH);
  digitalWrite(closeLight, LOW);
  lcd.clear();
  lcd.print("Finger Found");
  lcd.setCursor(0, 1);
  lcd.print("Welcome " + user);
  digitalWrite(relayPin, HIGH);
  delay(10000);
  digitalWrite(relayPin, LOW);
  digitalWrite(closeLight, HIGH);
  digitalWrite(openLight, LOW);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Door Closed   ");
  delay(3000);
  return;
}

void checkKeys() {
  //Checking if the enroll or delete button was pressed
  if (digitalRead(Enroll) == HIGH) {

    //Security measure put in place so that only an authorized user can add a new fingerprint
    for (int i = 0; i < 5; i++) {
      lcd.clear();
      lcd.print("Place Finger");
      delay(2000);
      int result = getFingerprintIDez();

      if (result >= 0) {

        switch (finger.fingerID) {
          case 1:
            lcd.clear();
            lcd.print("Please Wait");
            delay(1000);
            while (digitalRead(Enroll) == HIGH);
            EnrollFingerprint();
            break;
          case 2:
            lcd.clear();
            lcd.print("Please Wait");
            delay(1000);
            while (digitalRead(Enroll) == HIGH);
            EnrollFingerprint();
            break;
          default:
            Serial.println("Action Not Allowed");
            lcd.clear();
            lcd.print("Unauthorised");
            digitalWrite(closeLight, LOW);
            digitalWrite(errorLight, HIGH);
            delay(5000);
            digitalWrite(errorLight, LOW);
            return;
            break;
        }
      }
    }
  } else if (digitalRead(del) == HIGH) {  //Checking if the delete button was pressed

    //Security measure put in place so that only an authorized user can delete a fingerprint
    for (int i = 0; i < 5; i++) {
      lcd.clear();
      lcd.print("Place Finger");
      delay(2000);
      int result = getFingerprintIDez();

      if (result >= 0) {

        switch (finger.fingerID) {
          case 1:
            lcd.clear();
            lcd.print("Please Wait");
            delay(1000);
            deleteFingerprint();
            break;
          case 2:
            lcd.clear();
            lcd.print("Please Wait");
            delay(1000);
            deleteFingerprint();
            break;
          default:
            Serial.println("Action Not Allowed");
            lcd.clear();
            lcd.print("Unauthorised");
            break;
        }
      }
    }
  }
}

//Fingerprint Enrollment
void EnrollFingerprint() {
  int count = 0;
  lcd.clear();
  lcd.print("Enroll Finger    ");
  lcd.setCursor(0, 1);
  lcd.print("Location:");

  while (1) {
    lcd.setCursor(9, 1);
    lcd.print(count);

    if (digitalRead(up) == HIGH) {
      count++;
      if (count > 25)
        count = 0;
      delay(500);
    } else if (digitalRead(down) == HIGH) {
      count--;
      if (count < 0)
        count = 25;
      delay(500);
    } else if (digitalRead(del) == HIGH) {
      id = count;
      getFingerprintEnroll();
      return;
    } else if (digitalRead(Enroll) == HIGH) {
      return;
    }
  }
}

//Fingerprint Delete
void deleteFingerprint() {
  int count = 0;
  lcd.clear();
  lcd.print("Delete Finger    ");
  lcd.setCursor(0, 1);
  lcd.print("Location:");

  while (1) {
    lcd.setCursor(9, 1);
    lcd.print(count);
    if (digitalRead(up) == HIGH) {
      count++;
      if (count > 25)
        count = 0;
      delay(500);
    } else if (digitalRead(down) == HIGH) {
      count--;
      if (count < 0)
        count = 25;
      delay(500);
    } else if (digitalRead(del) == HIGH) {
      id = count;
      deleteFingerprint(id);
      return;
    } else if (digitalRead(Enroll) == HIGH) {
      return;
    }
  }
}

//Enroll Fingerprint
uint8_t getFingerprintEnroll() {
  int p = -1;
  lcd.clear();
  lcd.print("finger ID:");
  lcd.print(id);
  lcd.setCursor(0, 1);
  lcd.print("Place Finger");
  delay(2000);

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    switch (p) {

      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.clear();
        lcd.print("Image taken");
        break;

      case FINGERPRINT_NOFINGER:
        Serial.println("No Finger");
        lcd.clear();
        lcd.print("No Finger");
        break;

      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        lcd.clear();
        lcd.print("Comm Error");
        break;

      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        lcd.clear();
        lcd.print("Imaging Error");
        break;

      default:
        Serial.println("Unknown error");
        lcd.clear();
        lcd.print("Unknown Error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      lcd.clear();
      lcd.print("Image converted");
      break;

    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      lcd.clear();
      lcd.print("Image too messy");
      return p;

    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd.clear();
      lcd.print("Comm Error");
      return p;

    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.print("Feature Not Found");
      return p;

    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.print("Feature Not Found");
      return p;

    default:
      Serial.println("Unknown error");
      lcd.clear();
      lcd.print("Unknown Error");
      return p;
  }

  Serial.println("Remove Finger");
  lcd.clear();
  lcd.print("Remove Finger");
  delay(2000);
  p = 0;

  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.print("Place Finger");
  lcd.setCursor(0, 1);
  lcd.print("   Again");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;

      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;

      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;

      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;

      default:
        Serial.println("Unknown error");
        return;
    }
  }

  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;

    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;

    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;

    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;

    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  p = finger.createModel();

  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    lcd.print("Stored!");
    delay(2000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK)
    return -1;
  p = finger.image2Tz();

  if (p != FINGERPRINT_OK)
    return -1;
  p = finger.fingerFastSearch();

  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Finger Not Found");
    lcd.setCursor(0, 1);
    lcd.print("Try Again Later");
    delay(2000);
    return -1;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  return finger.fingerID;
}

//Delete fingerprint
uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;
  lcd.clear();
  lcd.print("Please wait");
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    lcd.clear();
    lcd.print("Finger Deleted");
    lcd.setCursor(0, 1);
    lcd.print("Successfully");
    delay(1000);
  } else {
    Serial.print("Something Wrong");
    lcd.clear();
    lcd.print("Something Wrong");
    lcd.setCursor(0, 1);
    lcd.print("Try Again Later");
    delay(2000);
    return p;
  }
}
