#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "Adafruit_Thermal.h"

#define pinSDA 10
#define pinRST 5

#define RX_Pin 7
#define TX_Pin 8

#define Print_Button 2
#define Buzzer 9
#define Relay 3
#define ON LOW

MFRC522 RFID(pinSDA, pinRST);
LiquidCrystal_I2C lcd(0x27 , 20 , 4);
SoftwareSerial mySerial_Print(RX_Pin, TX_Pin);

Adafruit_Thermal printer(&mySerial_Print);

String ID_RFID[] = {"NULL", "83 94 3E 13", "A3 B5 1A 0C"}; //DATA KARTU RFID

int index;

int a = 1,
    b = 1,
    c = 1,
    cek = 1,
    Aksi = 0;

float Tagihan;

String content = "";
byte letter;

bool ACC_Kartu = false;
bool Timer_On = false;

unsigned long mulai, selesai, dataStopWatch;
int i = 0;
int fPaus = 0;
long lastButton = 0;
long delayAntiBouncing = 50;
long dataPaus = 0;

float jam, menit, detik, miliDetik;
unsigned long over;

void setup() {
  Serial.begin(9600);
  mySerial_Print.begin(9600);
  SPI.begin();
  lcd.init();
  lcd.backlight();
  RFID.PCD_Init();
  printer.begin(9600);

  pinMode(Relay, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Print_Button, INPUT_PULLUP);
  digitalWrite(Relay, !ON);
  digitalWrite(Buzzer, LOW);

  Serial.println("RFID READER");
  Serial.println("");
  Serial.println("Tap Kartu/Gantungan !");
  Serial.println();
}

void Tampilan_Awal()
{
  lcd.setCursor(0, 0);
  lcd.print("  Tap Kartu Anda!  ");
}

void beep(int t)
{
  for ( int u = 0; u <= t; u++)
  {
    digitalWrite(Buzzer, HIGH);
    delay(100);
    digitalWrite(Buzzer, LOW);
    delay(100);
  }
}

void Tampilan_Stopwatch()
{
  lcd.setCursor(0, 2);
  lcd.print(jam, 0);
  lcd.print(":");
  lcd.print(menit, 0);
  lcd.print(":");
  lcd.print(detik, 0);
  lcd.print(".");
  if (jam < 10) {
    lcd.print(miliDetik, 0);
    lcd.print("   ");
  }
}

void Stopwatch_Berjalan()
{
  if ((millis() - lastButton) > delayAntiBouncing) {
    if (i == 0) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Start Timer");
      mulai = millis();
      fPaus = 0;
    }
    else if (i == 1) {
      lcd.setCursor(0, 0);
      lcd.print("Stop Timer  ");
      dataPaus = dataStopWatch;
      fPaus = 1;
    }
    i = !i;
  }
  lastButton = millis();
}

void Stopwatch_Reset()
{
  dataStopWatch = 0;
  dataPaus = 0;
  lcd.clear();
  lcd.print("Reset Stopwatch");
  lcd.setCursor(0, 1);
  lcd.print("0:0:0.0");
  delay(2000);
  lcd.clear();
  lcd.print("  Tekan Tombol  ");
}

void Stopwatch_Berhenti()
{
  if (i == 1) {
    selesai = millis();
    float jam, menit, detik, miliDetik;
    unsigned long over;

    // MATH time!!!
    dataStopWatch = selesai - mulai;
    dataStopWatch = dataPaus + dataStopWatch;

    jam = int(dataStopWatch / 3600000);
    over = dataStopWatch % 3600000;
    menit = int(over / 60000);
    over = over % 60000;
    detik = int(over / 1000);
    miliDetik = over % 1000;

    Tampilan_Stopwatch();
  }
}

void Hitung()
{
  float perJam, perMenit, Total;
  perJam = jam * 60000;
  perMenit = menit * 1000;

  Total = perJam + perMenit;
  Tagihan = Total;
}

void baca_RFID() {
  if ( ! RFID.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! RFID.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("ID Tag :");
  content = "";

  for (byte i = 0; i < RFID.uid.size; i++) {
    Serial.print(RFID.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(RFID.uid.uidByte[i], HEX);
    content.concat(String(RFID.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(RFID.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  RFID.PICC_HaltA();
  RFID.PCD_StopCrypto1();

  for (cek ; cek <= 10; cek++)
  {
    if (content.substring(1) == ID_RFID[cek]) {
      ACC_Kartu = true;
      index =  cek;
    }
  }
}

void Kartu_Diterima ()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Kartu Diterima");

  Serial.println("\t Card Authorized Access");  //Apabila menggunakan RFID Tag yang benar
  Serial.println();
}

void Kartu_Ditolak()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Kartu Ditolak");
  Serial.println("\tCard Declined");
  delay(500);
  baca_RFID();
}

void Cetak_Struk()
{
  //Thermal Printer Mencetak Struk
  printer.setSize('M');
  printer.justify('C');
  printer.println(F("RIWAYAT TRANSAKSI\n"));

  printer.setSize('S');
  printer.justify('L');
  printer.println("Tagihan :" + String(Tagihan));

  printer.setSize('M');
  printer.justify('C');
  printer.println(F("Terima Kasih\n\n"));
  printer.feed(2);

  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault();
}

void loop() {
  Tampilan_Awal();
  baca_RFID();

  if (content.substring(1) == "83 94 3E 13" ) // Ganti UID E3 4D F8 11 dengan UID Kartu Anda
  {
    c = 1;
    while (c <= 1) {
      Aksi = Aksi + 1;
      Serial.print("\nDATA AKSI :");
      Serial.print(Aksi);
      c++;
      delay(100);
    }
  }


  if (Aksi == 1)
  {
    Serial.print("\nAKSI 1 \t");
    Kartu_Diterima();
    digitalWrite(Relay, ON);
    beep(1);
    delay(1000);
    Stopwatch_Berjalan();
    Aksi = 2;
    delay(500);
  }

  if (content.substring(1) == "A3 B5 1A 0C" )
  {
    while (b <= 1)
    {
      Serial.print("\nAKSI 2 \t");
      digitalWrite(Relay, !ON);
      lcd.setCursor(0, 1);
      lcd.print("Stop Timer  ");
      dataPaus = dataStopWatch;
      Hitung();
      fPaus = 1;
      Aksi = 0;

      if (digitalRead(Print_Button) == ON)
      {
        while (a <= 1) {
          beep(1);
          lcd.setCursor(0, 3);
          lcd.print("  Print Out  ");
          Cetak_Struk();
          Stopwatch_Reset();
          lcd.setCursor(0, 3);
          lcd.print("             ");
          lcd.clear();
          a++;
          b++;
        }
      }
    }
  }

  if (i == 1) {
    selesai = millis();

    // MATH time!!!
    dataStopWatch = selesai - mulai;
    dataStopWatch = dataPaus + dataStopWatch;

    jam = int(dataStopWatch / 3600000);
    over = dataStopWatch % 3600000;
    menit = int(over / 60000);
    over = over % 60000;
    detik = int(over / 1000);
    miliDetik = over % 1000;

    if ((menit == 40 || menit == 20) && (detik < 1))
    {
      beep(2);
    }

    lcd.setCursor(0, 2);
    lcd.print(jam, 0);
    lcd.print(":");
    lcd.print(menit, 0);
    lcd.print(":");
    lcd.print(detik, 0);
    lcd.print(".");
    if (jam < 10) {
      lcd.print(miliDetik, 0);
      lcd.print("   ");
    }

    Hitung();
    lcd.setCursor(0, 3);
    lcd.print("Rp.");
    lcd.print(Tagihan);
  }
}
