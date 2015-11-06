#include <SoftwareSerial.h>

int pulsesensor = A0;
int tempsensor = A1;

String apiKey = "T3PYFXJYC31JBYT1";
SoftwareSerial esp(10,11); // RX,TX

int i, val;
float temp;
char buf[16];
String cmd, getStr, strTemp, strBPM;

volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduino finds a beat.

void setup()
{
  interruptSetup();
  pinMode(tempsensor,INPUT);
  pinMode(pulsesensor,INPUT);
  Serial.begin(9600);
  esp.begin(9600);
  esp.println("AT+RST");
}

void loop()
{
  val=0;
  for(i=0; i<10; i++)
  {
    val += analogRead(tempsensor);
   // delay(500);
  }

  // convert to temp:
  // temp value is in 0-1023 range
  // LM35 outputs 10mV/degree C. ie, 1 Volt => 100 degrees C
  // So Temp = (avg_val/1023)*5 Volts * 100 degrees/Volt
  temp = val*50.0f/1023.0f; 
  
  // convert to string
  strTemp = dtostrf(temp, 4, 1, buf);
  Serial.print("Temp =");
  Serial.println(strTemp);
  
  strBPM = dtostrf(BPM, 4, 1, buf);
  Serial.print("BPM =");
  Serial.println(strBPM);
  Serial.print("BPM =");
  Serial.println(BPM);
  cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  esp.println(cmd);
  Serial.println(cmd);
  
  if(esp.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }

  // prepare GET string
  getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp);
  if(QS == true)
  {
    getStr +="&field2=";
    getStr += String(strBPM);
    QS = false;
  }
  getStr += "\r\n\r\n";
  
  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  esp.println(cmd);
  Serial.println(cmd);
  delay(200);
  if(esp.find(">"))
  {
    esp.print(getStr);
    Serial.print(getStr);
  }
  else
  {
    esp.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }

  delay(1600);
}
