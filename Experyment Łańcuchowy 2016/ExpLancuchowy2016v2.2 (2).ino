  /* Eksperyment Lancuchowy 2016 - sterowanie za pomoca Arduino MEGA 2560
  copyrajt Szymon Wojcik (C)
  */
  // --- DEKLARACJE FIZYCZNYCH WYPROWADZEN - patrz schemat plytki --- //
  int LCD_ad0 = 25; // najmlodszy bit adresu wyswietlaczy LCD
  int LCD_ad1 = 26; // drugi bit
  int LCD_ad2 = 27; // trzeci najstarszy bit - nieuzywany w sumie bo tylko cztery wyswietlacze dzialaja :(
  int LCD_stb = 28; // pin blokujący transmisję, LOW = blokada, HIGH = przesylanie  - nazywany też STROB
  int czujnik1 = 29; // wykrycie kulki wpadającej do machiny LOW = wykryto
  int czujnik2 = 30; // wykrycie kulki w wyrzutni poziomej LOW = wykryto
  int czujnik3 = 31; // do ustalenia, kulka w lewitronie alboco;
  int czujnik4 = 32; // do ustalenia, kulka w rurce wyjsciowej - pozegnanie;
  int PkWyrzutnia = 34; // sterowanie przekaznikiem wyrzutni poziomej HIGH = wyrzuć, LOW = przekaźnik niezasilony (stan normalny)
  int PkWyjscie = 33; // sterowanie blokady kulki wyjsciowej, HIGH = wypuszcza kulke do rurki wyjsciowej
  int SilnikLewitronu1 = 12;
  int SilnikLewitronu2 = 11; // sterowanie winda z lewitronem jak oba HIGH = stoi, jedno LOW - kreci w dol albo w gore - SPRAWDZIC!
  int KrancowkaLewitronuGora = 37; // wykrycie pozycji gornej lewitronu - sygnal do zastopowania silnika, moze LOW = wykryto?
  int KrancowkaLewitronuDol = 38; // wykrycie pozycji dolnej lewitronu - stop silnika, moze LOW = wykryto?
  const int WL = 8; //wlacz laser w lewitronie, LOW = pracuje, HIGH = czuwanie, UWAGA, ZRE PRAD JAK CHOLERA, WLACZAC TYLKO NA CHWILE!
  // --- Zmienne pomocnicze --- //
  boolean Wyrzucona = LOW; // flaga zadzialania wyrzutni poziomej
  boolean Wykryta = LOW;
  int val1 = 0; // zmienna trzymajaca sygnal z czujnika 1
  int val2 = 0; // zmienna trzymajaca sygnal z czujnika 2
  int val3 = 0; // zmienna trzymajaca sygnal z czujnika 3
  int val4 = 0; // zmienna trzymajaca sygnal z czujnika 4
  int valKG = 0; //zmienna krancowki gornej, 0 = odczytujemy zadzialanie, 1 = krancowka zwolniona
  int valKD = 0; //zmienna krancowki dolnej, 0 = odczytujemy zadzialanie, 1 = krancowka zwolniona
void setup() {
// ustawiamy wszystkie piny odpowiednio do ich ról:
// --- PINY WEJSCIOWE -- //
  pinMode(czujnik1, INPUT);
  pinMode(czujnik2, INPUT);
  pinMode(czujnik3, INPUT);
  pinMode(czujnik4, INPUT);
  pinMode(KrancowkaLewitronuDol, INPUT);
  pinMode(KrancowkaLewitronuGora, INPUT);
 // --- PINY WYJSCIOWE -- // 
  pinMode(LCD_ad0, OUTPUT);
  pinMode(LCD_ad1, OUTPUT);
  pinMode(LCD_ad2, OUTPUT);
  pinMode(LCD_stb, OUTPUT);
  pinMode(PkWyrzutnia, OUTPUT);
  pinMode(PkWyjscie, OUTPUT); 
  pinMode(SilnikLewitronu1, OUTPUT);
  pinMode(SilnikLewitronu2, OUTPUT);
 // --- USTAWIAMY STAN POCZATKOWY PO RESECIE --- //
  digitalWrite(LCD_stb, LOW);	// blokada wysylania danych do LCD
  digitalWrite(LCD_ad2, LOW);   // adres 2 --> 0
  digitalWrite(LCD_ad1, LOW);   // adres 1 --> 0
  digitalWrite(LCD_ad0, LOW);	// adres 0 --> 0
  digitalWrite(SilnikLewitronu1, HIGH); // sprawdzic to jeszcze, czy lepiej dwa LOWy dac?
  digitalWrite(SilnikLewitronu2, HIGH);
  digitalWrite(PkWyrzutnia, LOW); 	// przekaznik wylaczony
  digitalWrite(PkWyjscie, LOW);		// przekaznik wylaczony
  digitalWrite(WL, LOW);// lewitron na nie czuwanu, laser nie swieci - WAZNE! ZRE PRAD JAK CHOLERA.
 // Ustawiamy port szeregowy - monitorowanie i debug:
  Serial.begin(9600);
 // Ustawiamy port szeregowy 1 - wyswietlanie napisow na LCD:
  Serial1.begin(19200, SERIAL_8O1);
 // START PROGRAMU - inicjalizacja i przywitanie // 
 czyscLCD(0);
 czyscLCD(1);
 czyscLCD(2);
 czyscLCD(3);
  digitalWrite(LCD_stb, LOW);
  digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
  digitalWrite(LCD_ad1, LOW);    // adres 1 --> 0
  digitalWrite(LCD_ad0, LOW);   // adres 0 --> 0
  digitalWrite(LCD_stb, HIGH);
  delay(100);
  Serial1.println("Omega 2 wita");
  Serial1.println("             ");
  Serial1.println("                    ");
  delay(100); 
 // zmiana adresu
/*  digitalWrite(LCD_stb, LOW);
  digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
  digitalWrite(LCD_ad1, LOW);    // adres 1 --> 0
  digitalWrite(LCD_ad0, HIGH);   // adres 0 --> 1
  digitalWrite(LCD_stb, HIGH);
  delay(100);
  Serial1.println("Opiekun:");
  Serial1.println("Marcin Wojcik");
  delay(100);*/
  digitalWrite(LCD_stb, LOW);
  digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
  digitalWrite(LCD_ad1, HIGH);   // adres 1 --> 1
  digitalWrite(LCD_ad0, LOW);    // adres 0 --> 0
  digitalWrite(LCD_stb, HIGH);
  delay(100);
  Serial1.println("Opiekun:");
  Serial1.println("Marcin Wojcik");
  Serial1.println("                    ");
  delay(100);
  digitalWrite(LCD_stb, LOW);
  digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
  digitalWrite(LCD_ad1, HIGH);   // adres 1 --> 1
  digitalWrite(LCD_ad0, HIGH);   // adres 0 --> 1
  digitalWrite(LCD_stb, HIGH);
  delay(100);
  Serial1.println("Program:");
  Serial1.println("Szymon Wojcik");
  Serial1.println("Budowa:Kacper Wojcik");
  delay(100);
}

void loop() {
  // --- Deklaracje zmiennych lokalnych --- //

  // --- Czytamy wejscia i podejmujemy kroki --- //
  val1 = digitalRead(czujnik1);	//część odpowiedzialna za odczytanie czy kulka wpadła
  val2 = digitalRead(czujnik2);
  val3 = digitalRead(czujnik3);
  val4 = digitalRead(czujnik4);
  valKD = digitalRead(KrancowkaLewitronuDol);
  valKG = digitalRead(KrancowkaLewitronuGora);
  Serial.println("Obieg petli");
//  Serial.println(val1,val2,val3,val4);
  
  if (val1 == 1 && Wykryta == LOW) {
    // Serial.println("Czujnik1+");
    czyscLCD(3);
	digitalWrite(LCD_stb, LOW);
    digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
    digitalWrite(LCD_ad1, HIGH);    // adres 1 --> 1
    digitalWrite(LCD_ad0, HIGH);    // adres 0 --> 1
    digitalWrite(LCD_stb, HIGH);	//zezwolenie na transmisje
	delay(100);
    Serial1.println("Witamy kulke");
	Serial1.println("ktora wlasnie wpadla");
  Serial1.println("             ");
	delay(100);
    digitalWrite(LCD_stb, LOW);
   Wykryta = HIGH;   
  }
  if (val2 == 1 && Wyrzucona == LOW) {
    // Serial.println("Czujnik2+");
    // wykryto kulke w wyrzutni poziomej - wywietlamy stosowny napis i wyrzucamy kulke.
	// dobrze byloby tez wyczyscic napis na 4tym wyswietlaczu.
  czyscLCD(2);
	digitalWrite(LCD_ad2, LOW);    	// adres 2 --> 0
    digitalWrite(LCD_ad1, HIGH);   	// adres 1 --> 1
    digitalWrite(LCD_ad0, LOW);    	// adres 0 --> 0 przechodzimy do wyświetlacza 2
    digitalWrite(LCD_stb, HIGH);	//zezwolenie na transmisje
	delay(100);
    Serial1.println("Teraz wystrzelimy");
	  Serial1.println("kulke wyrzutnia");
   Serial1.println("             ");
	delay(100);	
    digitalWrite(LCD_stb, LOW);
// -- Tutaj mozna dodac kod na odliczanie 5.. 4... 3.. 2... 1... :)
		digitalWrite(PkWyrzutnia, HIGH);
		delay(200);	
		digitalWrite(PkWyrzutnia, LOW);
		Wyrzucona = HIGH; // flagujemy wyrzucenie kuleczki - zerowanie po resecie calosci
		
  }
  if (val3 == HIGH) {
    czyscLCD(1);
    digitalWrite(LCD_stb, LOW);
    digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
    digitalWrite(LCD_ad1, LOW);    // adres 1 --> 0
    digitalWrite(LCD_ad0, HIGH);   // adres 0 --> 1 przechodzimy do wyświetlacza 1
    digitalWrite(LCD_stb, HIGH);
	delay(100);
    Serial1.println("Widzieliscie kiedys");
	Serial1.println("    taka lewitacje?");
	delay(100);	
	digitalWrite(LCD_stb, LOW);
 // --- tutaj kod sterujacy winda z lewitronem -> wlaczamy silnik do momentu kiedy nie osiagnie gornej pozycji, patrz KrancowkaGora = LOW
 // --- zakladamy ze winda z lewitronem jest na jakiejs dobrej pozycji, sprawdzamy tez stan czujnikow:
	if (valKG == 0 && valKD == 0) 
				{
		// mamy blad czujnikow - stop windy //
		digitalWrite(LCD_ad2, LOW);   	// adres 2 --> 0
		digitalWrite(LCD_ad1, LOW);   	// adres 1 --> 0
		digitalWrite(LCD_ad0, LOW);		// adres 0 --> 0
		digitalWrite(LCD_stb, HIGH);
		delay(100);
		Serial1.println("BLAD!!!");
		Serial1.println("Awaria czujnikow");
		delay(100);	
		digitalWrite(LCD_stb, LOW);
		digitalWrite(SilnikLewitronu1, HIGH); 	// stopujemy bezwzglednie winde z lewitronem
		digitalWrite(SilnikLewitronu2, HIGH);
		//digitalWrite(WL, HIGH);		// lewitron na czuwanu, laser nie swieci - WAZNE! ZRE PRAD JAK CHOLERA.
				}
	while (valKG == 1) { // tutaj moze nie skonczc petli dopoki czujnik nie zadziala - > mozna dolozyc && milis() < jakis_czas
		valKG = digitalRead(KrancowkaLewitronuGora); //musimy czytac w petli stan krancowki
		//digitalWrite(WL, LOW); // uzywac rozsadnie, ZRE PRAD!
		digitalWrite(SilnikLewitronu1, LOW); //gora
		digitalWrite(SilnikLewitronu2, HIGH);
		}
		digitalWrite(SilnikLewitronu1, HIGH); // stopujemy silnik po wykryciu krancowki
		digitalWrite(SilnikLewitronu2, HIGH);
		// -- Tutaj mozna dodac kod na odliczanie 5.. 4... 3.. 2... 1... :)
		digitalWrite(PkWyjscie, HIGH);
		delay(1000);	 // musi byc czas na przepuszczenie kulki przez rampe wyjsciowa - DOBRAC.
		digitalWrite(PkWyjscie, LOW);
		delay(500); //czekamy az sobie kulka wyleci spokojnie - dobrac czas stosownie
		//digitalWrite(WL, 1); // wylaczamy lewitron!
		
		
  }

  if (val4 == 1) {
    czyscLCD(0);
    digitalWrite(LCD_ad2, LOW);    // adres 2 --> 0
    digitalWrite(LCD_ad1, LOW);    // adres 1 --> 0
    digitalWrite(LCD_ad0, LOW);    // adres 0 --> 0
	digitalWrite(LCD_stb, HIGH);
	delay(100);	
    Serial1.println("Kulce mowimy pa pa");//wykrywamy pod koniec i żegnamy kulkę
	delay(100);
    digitalWrite(LCD_stb, LOW);
        //digitalWrite(WL, HIGH); // wylaczamy lewitron!
	while (valKD == 1) { // tutaj moze nie skonczc petli dopoki czujnik nie zadziala.
		valKD = digitalRead(KrancowkaLewitronuDol);
		digitalWrite(SilnikLewitronu1, HIGH); // sprawdzic kierunek!!!
		digitalWrite(SilnikLewitronu2, LOW);
		}
		digitalWrite(SilnikLewitronu1, HIGH); // stopujemy silnik po wykryciu krancowki
		digitalWrite(SilnikLewitronu2, HIGH);
  }
}
void czyscLCD (int numerLCD) {
  switch (numerLCD) {
    case 0:    // your hand is on the sensor
       digitalWrite(LCD_ad2, LOW);  // adres 2 --> 0
       digitalWrite(LCD_ad1, LOW);    // adres 1 --> 0
       digitalWrite(LCD_ad0, LOW);    // adres 0 --> 0
       digitalWrite(LCD_stb, HIGH);
        Serial1.println("                    ");
        Serial1.println("                    ");
        Serial1.println("                    ");
        Serial1.println("                  ");
       delay(100);
       digitalWrite(LCD_stb, LOW);
      break;
    case 1:    // your hand is close to the sensor
      digitalWrite(LCD_ad2, LOW);  // adres 2 --> 0
      digitalWrite(LCD_ad1, LOW);    // adres 1 --> 0
      digitalWrite(LCD_ad0, HIGH);    // adres 0 --> 1
      digitalWrite(LCD_stb, HIGH);
       Serial1.println("                    ");
       Serial1.println("                    ");
       Serial1.println("                    ");
       Serial1.println("                  ");
      delay(100);
      digitalWrite(LCD_stb, LOW);
      break;
    case 2:    // your hand is a few inches from the sensor
      digitalWrite(LCD_ad2, LOW);  // adres 2 --> 0
      digitalWrite(LCD_ad1, HIGH);    // adres 1 --> 1
      digitalWrite(LCD_ad0, LOW);    // adres 0 --> 0
      digitalWrite(LCD_stb, HIGH);
       Serial1.println("                    ");
       Serial1.println("                    ");
       Serial1.println("                    ");
       Serial1.println("                  ");
      delay(100);
      digitalWrite(LCD_stb, LOW);
      break;
    case 3:    // your hand is nowhere near the sensor
      digitalWrite(LCD_ad2, LOW);  // adres 2 --> 0
      digitalWrite(LCD_ad1, HIGH);    // adres 1 --> 1
      digitalWrite(LCD_ad0, HIGH);    // adres 0 --> 1
      digitalWrite(LCD_stb, HIGH);
       Serial1.println("                    ");
       Serial1.println("                    ");
       Serial1.println("                    ");
       Serial1.println("                  ");
      delay(100);
      digitalWrite(LCD_stb, LOW);
      break;
  }
 }
 
