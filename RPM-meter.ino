#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

volatile unsigned long last_rise_right = 0;
volatile int ignore_next_rise_right = 0;

const char* SSID = "Vincent AP 2.0";
const char* PASS = "vincentkenbeek";

const int RIGHT_PIN = D4;
const float RADIUS_CM = 4;
const float RADIUS_M = RADIUS_CM * 0.01;

const char* EMPTY_LINE = "                "; // 16 empty chars

int lcd_setup_complete = 0;
int wifi_connected = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Wire.begin();
  
  cli(); // stop interrupts

  lcdSetup();
  wifiSetup();
  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  
  sei(); // allow interrupts

  attachInterrupt(digitalPinToInterrupt(RIGHT_PIN), right_wheel_rising, RISING);
  
  Serial.println("Setup complete");
}

void loop() {

}

void right_wheel_rising() {
  int timer = micros() - last_rise_right;
  int duration, rpm;
  float m_per_s, km_per_h, m_per_min;
  if (timer > 50 && ignore_next_rise_right == 0) {
    duration = (micros() - last_rise_right);
    last_rise_right = micros();
    rpm = (60000000 / duration);
    m_per_s = RADIUS_M * rpm * 0.10472;
    km_per_h = m_per_s * 3.6;
    m_per_min = m_per_s * 60;
    lcdClearLine(0);
    lcd.print("RPM = ");
    lcd.print(rpm);
    lcdClearLine(1);
    lcd.print("m/min = ");
    lcd.print(m_per_min);
    
    ignore_next_rise_right = 1;
//    digitalWrite(LED_BUILTIN, HIGH);
  } else if (timer > 50 && ignore_next_rise_right == 1) {
    ignore_next_rise_right = 0;
//    digitalWrite(LED_BUILTIN, LOW);
  }
}

void lcdClearLine(int line) {
  lcd.setCursor(0, line);
  lcd.print(EMPTY_LINE);
  lcd.setCursor(0, line);
}

void lcdSetup() {
  Serial.println("Setting up LCD");
  lcd.begin(16, 2);
  lcd.init();

  lcd.backlight();

  lcdClearLine(0);
  lcdClearLine(1);

  lcd_setup_complete = 1;
  Serial.println("LCD setup complete");
}

void wifiSetup() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  int attempts = 0, cur_pos = -1;
  lcd.setCursor(0, 0);
  while (WiFi.status() != WL_CONNECTED) {
    attempts++;

    if (attempts % 16 == 0) {
      cur_pos = 0;
      lcdClearLine(0);
    } else {
      cur_pos++;
    }
    
    delay(500);
    if (lcd_setup_complete == 1) {
      lcd.setCursor(cur_pos, 0);
      lcd.print(".");
    }
    Serial.print(".");
  }

  wifi_connected = 1;
  lcdClearLine(0);
  lcd.print("WiFi Connected");
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void showI2CAddresses() {
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}
