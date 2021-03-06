/*
 * Niebo wersja 2.0
 * Autorstwa Szymona Wójcika,
 */


#include <IRremote.h>
#include <EEPROM.h>
#include <leOS2.h>

leOS2 threads;
//PINY (wyprowadzenia procesora):
const int termistor = A0;// termistor mierzacy temperature zasilacza
const int fan = 5; // wiatrak chlodzacy zasilacz 
const int LED_zasil= 2; //Wylaczanie zasialacza led-ów
const int RECV_PIN = 12; //wejscie odbiornika podczerwieni
const int ZASIL = 11; //wykrycie braku zasilania
const int DIODA = 13; // dioda sygnalizująca odbiór sygnału lub stan czuwania
const int SUFIT1 = 9; //sekcja led-ów 1
const int SUFIT2 = 10; //sekcja led-ów 2
// ZMIENNE:
int liczba; // zmienna pomocnicza użyta w procedurze płynnego rozjaśniania/ściemniania
float powlight; // przechowuje wartośc do przypisania jasności LED-ów
int codeType = -1; // Typ kodu (pilot)
unsigned long codeValue; // przechowuje wartość rozkazu przypisaną do danego klawisza pilota
unsigned int rawCodes[RAWBUF]; // Przechowywanie kodów nierozpoznanych - nieużywane w tym programie (element biblioteki)
int codeLen; // Nieużywane w programie - wymagania biblioteki
byte light = 0; // zmienna przechowująca wartość odpowiedzialną za jasność (0 - nie świeci 100 - MAX
int wiatrak =0; // zmienna określająca tryb pracy wiatraka (0 - nie kręci sie 1 - kręci się 2 - kręci sie z makstmalną prędkością)
int sensorValue; // wartość odczytana z termistora (wejście analogowe z przetwornika), np: 400 - 70° C   700 - 40° C
int outputValue; // wartość przypisana do wiatraka, np 60 - minimalne obroty, 150 - średnie obroty, 255 maksymalne obroty (powinno być 1023 - błąd do poprawienia w kolejnej wersji)
boolean stanZasil =0; // zmienna przechowująca katualny stan zasilania LED-ów
boolean flaga =0; // flaga/zmienna potrzebna do zapisywania danych do EEPROM-u jednorazowo po wykryciu braku zasilania
boolean flagzas =1; // flaga/zmienna zabezpieczająca przed ciągłym wyłączaniem i włączaniem zasilania LED-ów
//DEFINICJA WARTOŚCI LICZBOWYCH:
#define potega 1.5 // potęga potrzebna przy wyliczaniu wartości do rpzypisania do LED-ów
// Inicjowanie biblioteki do odbioru pilota podczerwieni:
IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

void setup()
{
  Serial.begin(9600); // standardowe ustawienie portu szeregowego
  irrecv.enableIRIn(); // Uruchomienie czujnika podczerwieni
  // WARTOŚCI POCZĄTKOWE I TRYBY PRACY PINÓW PROCESORA:
  pinMode(LED_zasil, OUTPUT); // zasilacz
  //pinMode(3, OUTPUT); // wiatrak
  pinMode(ZASIL, INPUT);  //Wykrycie braku zasilania
  pinMode(RECV_PIN, INPUT); // Podczerwień
  pinMode(DIODA, OUTPUT); // LED sygnalizacyjny
  digitalWrite(LED_zasil, LOW); // włączenie zasilacza głównego LED- ów

  //Ustawienie 10-bitowego trybu pracy pinów sterujących PWM
  //**********************************************************************************
  // Set pwm clock divider
  //**********************************************************************************
  TCCR1B &= ~(1 << CS12);
  TCCR1B  |=   (1 << CS11);
  TCCR1B &= ~(1 << CS10);  

  //**********************************************************************************
  // Set pwm resolution  to mode 7 (10 bit)
  //**********************************************************************************

  TCCR1B &= ~(1 << WGM13);    // Timer B clear bit 4
  TCCR1B |=  (1 << WGM12);    // set bit 3

  TCCR1A |= (1 << WGM11);    //  Timer A set bit 1
  TCCR1A |= (1 << WGM10);    //  set bit 0 

  //Odczyt z EEPROM-u po właczeniu zasilania
  light = EEPROM.read(0);
  Serial.println("Reading INT from EEPROM: ");
  Serial.println(light);
  Serial.println();
  powlight = pow(light,potega);
  analogWrite(SUFIT1,powlight);
  analogWrite(SUFIT2,powlight);

  //Uruchomienie wątków - biblioteka leOS2
  threads.begin();
    threads.addTask(gora, threads.convertMs(50));
    threads.addTask(dol, threads.convertMs(50));
    threads.addTask(wyl, threads.convertMs(2000));
    threads.pauseTask(gora);
    threads.pauseTask(dol);
    threads.pauseTask(wyl);
}



void loop() {
  int currentRead = digitalRead(ZASIL); // czytanie pinu sprawdzającego stan zasilania

  if(currentRead==LOW && flaga==0) {
      // zapis do EEPROM-u
      EEPROM.write(0, light);
      Serial.print(EEPROM.read(0)); Serial.print(" ");
      flaga=1;
  }
  // odczytywanie oporu termistora i właczenie wiatraka w odpowieni tryb:
  sensorValue = analogRead(termistor); 
  wiatrakSter();
  analogWrite(fan, outputValue);

  //Odczyt i dekodowanie rozkazów z pilota:
  if (irrecv.decode(&results)) {
    digitalWrite(DIODA, HIGH);
    storeCode(&results);
    irrecv.resume();
    digitalWrite(DIODA, LOW);
    if (codeType == 1 && (codeValue == 268 || codeValue == 2316)&& flagzas==1) {
      threads.restartTask(wyl);
      flagzas=0;
      stanZasil = !stanZasil;
    }
    if (codeType == 1 && (codeValue == 16 || codeValue == 2064)) {
      pauza();
      if (light<101) {
        light++;
      }
    }
    if (codeType == 1 && (codeValue == 17 || codeValue == 2065)) {
      pauza();
      if (light>0) {
        light--;
      }
    }
    if (codeType == 1 && (codeValue == 256 || codeValue == 2304)) {
      pauza();
      liczba=0;
      if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 257 || codeValue == 2305)) {
      pauza();
      liczba=11;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 258 || codeValue == 2306)) {
      pauza();
      liczba=22;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 259 || codeValue == 2307)) {
      pauza();
      liczba=33;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 260 || codeValue == 2308)) {
      pauza();
      liczba=44;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 261 || codeValue == 2309)) {
      pauza();
      liczba=55;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 262 || codeValue == 2310)) {
      pauza();
      liczba=66;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 263 || codeValue == 2311)) {
      pauza();
      liczba=77;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 264 || codeValue == 2312)) {
      pauza();
      liczba=88;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    if (codeType == 1 && (codeValue == 265 || codeValue == 2313)) {
      pauza();
      liczba=99;
      if (light<liczba) {
        threads.restartTask(gora);
      }
      else if (light>liczba) {
        threads.restartTask(dol);
      }
    }
    // Przypisanie obliczonej wartości do LED-ów
    zapisz();
    digitalWrite(LED_zasil,stanZasil);
    digitalWrite(DIODA,stanZasil);
  }
  
}

//PROCEDURY:
void storeCode(decode_results *results) { //Odczytanie wartości z pilota za pomocą czujnika podczerwieni
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (codeType == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
      //Serial.println(light);
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, DEC);
    codeValue = results->value;
    codeLen = results->bits;
  }
}

void gora() { // płynne rozjaśnianie
  if (light<liczba) {
    light++;
    zapisz();
  }
  else {
    threads.pauseTask(gora);
  }
}
void dol() { // płynne ściemnianie
  if (light>liczba) {
    light--;
    zapisz();
  }
  else {
    threads.pauseTask(dol);
  }
}
void zapisz () { // zapis danych do LED-ów
  Serial.println(light);
  powlight = pow(light,potega);
  if (powlight > 1010) {
      powlight = 1023;
  }
  Serial.println(powlight);
  analogWrite(SUFIT1,powlight);
  analogWrite(SUFIT2,powlight);
}
void pauza() { // pauzowanie wątków rozjaśniania i ściemniania
  threads.pauseTask(gora);
  threads.pauseTask(dol);
}
void wyl() { // blokowanie wyłączania zasilania LED-ów na 2 sekundy (patrz SETUP)
  flagzas=1;
  threads.pauseTask(wyl);
}
void wiatrakSter() { // ustawianie chłodzenia wiatrakiem
    if (sensorValue >650 && wiatrak == 0) {
    outputValue = 0;
  }
  if (sensorValue <=650 && wiatrak == 0) {
    wiatrak =1;
  }
  if (sensorValue <=700 && sensorValue >=400 && wiatrak==1) {
    outputValue = map(sensorValue, 400, 700, 255, 60);
  }
  if (sensorValue <=400 && wiatrak == 1) {
    wiatrak = 2;
    outputValue=255;
  }
  if (sensorValue >= 400 && wiatrak ==2) {
    wiatrak =1;
  }
  if (sensorValue>=700 && wiatrak==1) {
    wiatrak=0;
    outputValue=0;
  }
  /* Przykładowy kod do kalibracji termistora
  Serial.print("sensor = ");
  Serial.print(sensorValue);
  Serial.print("\t output = ");
  Serial.print(outputValue);
  Serial.print("\t wiatrak = ");
  Serial.println(wiatrak);*/
}
