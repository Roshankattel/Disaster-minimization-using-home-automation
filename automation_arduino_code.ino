#include <SoftwareSerial.h>
SoftwareSerial GPRS(8,9); // RX, TX
#include <Keypad.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
const byte ROWS = 3; //four rows
const byte COLS = 3; //four columns
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};
byte rowPins[ROWS] = {A5, A4, A3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A2, A1, A0 }; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

char pass[4]={'8','8','4','8'};
char temp[5];
int i=0;
char data = "0";   

enum parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_READ_CMTI_STORAGE_TYPE,
  PS_READ_CMTI_ID,

  PS_READ_CMGR_STATUS,
  PS_READ_CMGR_NUMBER,
  PS_READ_CMGR_SOMETHING,
  PS_READ_CMGR_DATE,
  PS_READ_CMGR_CONTENT
};

byte state = PS_DETECT_MSG_TYPE;

char buffer[80];
byte pos = 0;

int lastReceivedSMSId = 0;
boolean validSender = false;

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void setup()
{
  GPRS.begin(9600);
  Serial.begin(9600);

  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);

lcd.begin(16,2);
 lab:
 lcd.print("Enter code");
 i=0;
 while(i!= 4) //taking 4 digit password
 {
   char customKey = customKeypad.getKey();
  if(customKey){
    temp[i]=customKey;
    lcd.setCursor(i,1);
    lcd.print(temp[i]);
     i++;
  } 
  
 }  
  delay(300);
  for( int j=0;j<4;j++) //checking with the preinstalled password
{
 if(temp[j]!=pass[j])
{
 lcd.clear();
 lcd.home();
 lcd.print("Invalid code"); 
 delay(1000);
 lcd.clear();
goto lab;                   //if invalid goto top   
}
}
lcd.clear();
lcd.print("Valid code");   //if valid continue
lcd.clear();
 lcd.print("*** Welcome ***");

  for (int i = 1; i <= 20; i++) 
  {
    GPRS.print("AT+CMGD=");
    GPRS.println(i);
    delay(200);
    
    while(GPRS.available()) 
      Serial.write(GPRS.read());
  }
lcd.clear();
lcd.print("waiting signal");
 
}


void loop()
{
  while(GPRS.available()) 
  {
    parseATText(GPRS.read());
  }
lcd.home();
  if(Serial.available() > 0) 
  {
    data = Serial.read();      
    
    if(data == 'A')            
      {
      digitalWrite(13, HIGH);  
      lcd.clear();
      lcd.print("Bulb 1 ON-BT");
      delay(1000);
      }
    else if(data == 'a')
    {     
      digitalWrite(13, LOW);
      lcd.clear();
      lcd.print("Bulb 1 OFF-BT");
      delay(1000);
    } 
      
      else if(data == 'B')            
      {
      digitalWrite(12, HIGH);  
      lcd.clear();
      lcd.print("Bulb 2 ON-BT");
      delay(1000);
      }
    else if(data == 'b')       
      {
      digitalWrite(12, LOW); 
      lcd.clear();
      lcd.print("Bulb 2 OFF-BT");
      delay(1000);  
      
      }
      else if(data == 'C')             
      {
      digitalWrite(11, HIGH); 
      lcd.clear();
      lcd.print("Bulb 3 ON-BT");
      delay(1000);
      }
    else if(data == 'c')       
     {
      digitalWrite(11, LOW);   
        lcd.clear();
        lcd.print("Bulb 3 OFF-BT");
        delay(1000);
     }
    else if(data == 'D')             
      {
        digitalWrite(10, HIGH);  
          lcd.clear();
          lcd.print("Bulb 4 ON-BT");
          delay(1000);
      }
    else if(data == 'd')       
      {
        digitalWrite(10, LOW);   
        lcd.clear();
         lcd.print("Bulb 4 OFF-BT");
         delay(1000);
        }
    else
    {
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      digitalWrite(12, LOW);
      digitalWrite(13, LOW);
      lcd.clear();
      lcd.print("All Bulbs OFF-BT");
      delay(1000);
    }                           
 }
}



void parseATText(byte b) {

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); 

  switch (state) {
  case PS_DETECT_MSG_TYPE: 
    {
      if ( b == '\n' )
        resetBuffer();
      else {        
        if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
          state = PS_IGNORING_COMMAND_ECHO;
        }
        else if ( pos == 6 ) {
     

          if ( strcmp(buffer, "+CMTI:") == 0 ) {
            Serial.println("Received CMTI");
            state = PS_READ_CMTI_STORAGE_TYPE;
          }
          else if ( strcmp(buffer, "+CMGR:") == 0 ) {
            Serial.println("Received CMGR");            
            state = PS_READ_CMGR_STATUS;
          }
          resetBuffer();
        }
      }
    }
    break;

  case PS_IGNORING_COMMAND_ECHO:
    {
      if ( b == '\n' ) {
       
        state = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMTI_STORAGE_TYPE:
    {
      if ( b == ',' ) {
        Serial.print("SMS storage is ");
        Serial.println(buffer);
        state = PS_READ_CMTI_ID;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMTI_ID:
    {
      if ( b == '\n' ) {
        lastReceivedSMSId = atoi(buffer);
        Serial.print("SMS id is ");
        Serial.println(lastReceivedSMSId);

        GPRS.print("AT+CMGR=");
        GPRS.println(lastReceivedSMSId);
      

        state = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMGR_STATUS:
    {
      if ( b == ',' ) {
        Serial.print("CMGR status: ");
        Serial.println(buffer);
        state = PS_READ_CMGR_NUMBER;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMGR_NUMBER:
    {
      if ( b == ',' ) {
        Serial.print("CMGR number: ");
        Serial.println(buffer);

        //  to check the sender's cell number
        //validSender = false;
        //if ( strcmp(buffer, "\"+0123456789\",") == 0 )
        validSender = true;

        state = PS_READ_CMGR_SOMETHING;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMGR_SOMETHING:
    {
      if ( b == ',' ) {
        Serial.print("CMGR something: ");
        Serial.println(buffer);
        state = PS_READ_CMGR_DATE;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMGR_DATE:
    {
      if ( b == '\n' ) {
        Serial.print("CMGR date: ");
        Serial.println(buffer);
        state = PS_READ_CMGR_CONTENT;
        resetBuffer();
      }
    }
    break;

  case PS_READ_CMGR_CONTENT:
    {
      if ( b == '\n' ) {
        Serial.print("CMGR content: ");
        Serial.print(buffer);

        parseSMSContent();

        GPRS.print("AT+CMGD=");
        GPRS.println(lastReceivedSMSId);
        

        state = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;
  }
}

void parseSMSContent() {

  char* ptr = buffer;

  while ( strlen(ptr) >= 2 ) {

    if ( ptr[0] == 'a' ) {
      if ( ptr[1] == '1' )
        {
          digitalWrite(13, HIGH);
        lcd.clear();
      lcd.print("Bulb 1 ON-GSM");
       delay(1000);
        }
      else if (ptr[1]=='0')
        {
         digitalWrite(13, LOW);
         lcd.clear();
         lcd.print("Bulb 1 OFF-GSM");
        delay(1000);
        }
    }

    if ( ptr[0] == 'b' ) {
      if ( ptr[1] == '1' )
        {
          digitalWrite(12, HIGH);
          lcd.clear();
         lcd.print("Bulb 2 ON-GSM");
        delay(1000);
        }
      else if (ptr[1]=='0')
        {
          digitalWrite(12, LOW);
        lcd.clear();
      lcd.print("Bulb 2 OFF-GSM");
        delay(1000);
        }
    }

    if ( ptr[0] == 'c' ) {
      if ( ptr[1] == '1' )
        {
          
          digitalWrite(11, HIGH);
        lcd.clear();
      lcd.print("Bulb 3 ON-GSM");
        delay(1000);
        }
      else if (ptr[1]=='0')
        {
          digitalWrite(11, LOW);
          lcd.clear();
         lcd.print("Bulb 3 OFF-GSM");
        delay(1000);
        }
    }

    if ( ptr[0] == 'd' ) {
      if ( ptr[1] == '1' )
       {
        digitalWrite(10, HIGH);
       lcd.clear();
         lcd.print("Bulb 4 ON-GSM");
        delay(1000);
       }
      else if (ptr[1]=='0')
        {
         digitalWrite(10, LOW);
        lcd.clear();
        lcd.print("Bulb 4 OFF-GSM");
        delay(1000);
        }
        
    }

    ptr += 2;
  }
}




