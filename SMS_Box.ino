#include <TinyGPS++.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(5, 4); //RX,TX

int GPSBaud = 9600;
TinyGPSPlus gps;

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.println("System Started...");
  delay(5000);
  
  mySerial.print("AT+CMGF=1\r");  // set SMS mode to text

  delay(100);

  mySerial.print("AT+CNMI=2,2,0,0,0\r"); // AT Command to receive a live SMS
  mySerial.print("AT+CMGD=1,4\r");
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  delay(100);
  Serial.println("System Ready...");
  digitalWrite(11, HIGH);
}

char msg;
String message = "";
int count =0;

int trigged = 0;
int cmt_count = 0;
char cmt[5];
int cmt_trigged = 0;

String hp;
String content;

void loop() 
{
  if (mySerial.available()>0)
  {
    msg=mySerial.read();
    count = 0;

    /*====================Capture +CMT====================*/
    if(msg == '+')
    {
      trigged = 1;
    }

    if(trigged == 1)
    {
      cmt[cmt_count]=msg;
      cmt_count += 1;

      if(cmt_count == 4)
      {
        if(cmt[3] == 'T')
        {
          cmt_trigged = 1;
          Serial.print("Success Capture");
          Serial.print("\n------------\n");
          Serial.print(cmt);
          Serial.print("\n");
          Serial.print("------------\n");
        }
        else
        {
          cmt_count = 0;
          trigged = 0;
        }
      }
    }

    if(cmt_trigged == 1)
    {
       message = message+msg;
    }
    /*====================End of Capture====================*/
  }
  else
  {
    if(count < 10)
    {
      count += 1;
    }
    else if(count == 10)
    {
      if(cmt_trigged == 1)
      {
        hp = process_hp(message);
        content = process_content(message);
        Serial.print("Phone Number\t:");
        Serial.println(hp);
        Serial.print("Message\t:");
        Serial.println(content);

        reply(hp, content);

        message = "";
        cmt_trigged = 0;
        cmt[3] = 'a';
        cmt_count = 0;
        trigged = 0;
      }

      count += 1;
    }
  }
}



String process_hp(String input)
{
  int phoneNumberStart = input.indexOf("+"); // Find the start index of the phone number
  int phoneNumberEnd = input.indexOf(",", phoneNumberStart) - 1; // Find the end index of the phone number
  String phoneNumber = input.substring(phoneNumberStart, phoneNumberEnd); // Extract the phone number

  return phoneNumber;
}

String process_content(String input)
{
  int messageStart = input.indexOf("\n") + 1; // Find the start index of the message
  String message = input.substring(messageStart); // Extract the message
  message.trim();
  
  return message;
}

void reply(String phone, String sms)
{
  if(sms == "1")
  {
    mySerial.println("AT+CMGS=\"" + phone + "\"");
    String location = GPS();
    mySerial.print(location); //text content
    mySerial.write(26);
    delay(500);
    Serial.println("Success Reply");
  }
  else if(sms == "2")
  {
    digitalWrite(9,HIGH);
    delay(10000); //Unlock the door for 10 second
    digitalWrite(9,LOW);
    Serial.println("Success Unlock");
  }
  else if(sms == "0")
  {
    mySerial.println("AT+CMGS=\"" + phone + "\"");
    mySerial.print("Menu:\n1. Request for GPS\n2. Unlock the Box's lock\n3. Beep the box\n0. Request for Menu\n\nIf the system not responding, please send the command again."); //text content
    mySerial.write(26);
    delay(500);
    Serial.println("Menu sent.");
  }
  else if(sms == "3")
  {
    for(int i=0;i<=10;i++)
    {
      digitalWrite(10, HIGH);
      delay(250); 
      digitalWrite(10,LOW); 
      delay(1000); 
    }
  }
}

String GPS()
{
  int count = 0;
  String output = "";
  while(1)
  {
    if(Serial.available() > 0)
    {
      if (gps.encode(Serial.read()))
      {
        if (gps.location.isValid())
        {
          double latitude = gps.location.lat();          
          output += String(latitude,6);
          output += ",";
          double longtitude = gps.location.lng();
          output += String(longtitude,6);
          break;
        }
      }
      else
      {
        count += 1;
        if(count == 10000)
        {
          output += "GPS out of Signal.";
          break;
        }
      }
    }
    else if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      output += "GPS No Signal.";
      delay(1000);
      break;
    }
  }

  return output;
}