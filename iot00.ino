/*
Internet Of Thing 001 by Dan Ray

usage: http://iot001/?p=50,50

Board:
 * Arduino Uno

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * servo attached to pins 8,9

 modified Web Server example:
 Web Server created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h> 

byte mac[] = { 0xDA, 0xAD, 0xED, 0x00, 0x00, 0x01 };
const int RCOUNT = 2;
String Refererers[RCOUNT];
int RefererersCounterer = 0;
boolean echo = false;//true=echoes the request headers;false=shows the last n=RCOUNT requests
boolean newmessage = false;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xED, 0xED, 0xED };
  //hostname: http://wiznetededed/
  // a request would look like:
  //http://wiznetededed/?p=50

/////////////////////////////////////////////////////////////////////
///////////// IPAddress /////////////////////////////////////////////
//IPAddress ip(192,168,0,3);  /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int milliseconds_of_delay = 10;
// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

Servo horz_servo;  // create servo object to control a servo 
Servo vert_servo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
int horz_servo_pin = 9;
int vert_servo_pin = 8;
String pos = "0";    // variable to store the current position 
String los = "0";
int loop_count = 0;
long refresh_counter = 0;

void setup() {
  set_up_serial(false);//set to "true" for debugging
  horz_servo.attach(horz_servo_pin);  // attaches the servo on pin 9 to the servo object
  vert_servo.attach(vert_servo_pin);
  set_horz_servo("20");//indicate that it is starting
  delay(1000);//wait for the servo to finish moving
  // start the Ethernet connection and the server:
  get_ip(mac);
  //Ethernet.begin(mac, ip);
  set_horz_servo("60");//indicate that it is progressing
  delay(1000);//wait for the servo to finish moving
  
  server.begin();
  serial_print("server is at ");
  serial_println(String(Ethernet.localIP()));
  set_horz_servo("100");//indicate that it is working
  set_vert_servo("50");
  clearArray();
}

void loop() {
  refresh_counter++;
  if(refresh_counter > 60000)//60000 = 1/10 min
  {
    //set_servo("50");
    //delay(2000);
    refresh_counter = 0;
    get_ip(mac);
    //set_servo(pos);
  }
  // listen for incoming clients
  EthernetClient client = server.available();
  String theCurrentLine;
  if (client) 
  {
    loop_count = 0;
    if(echo)
    {
      clearArray();
      newmessage = true;
    }
    serial_println(">>>>>>>>>>");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String theCurrentLine = String("");
    String p = "0";
    while (client.connected()) {
      if (client.available())//===============================================
      {
          char c = client.read();         
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) 
          {
             theCurrentLine = String("");
             parse_theCurrentLine("end:" + theCurrentLine);
             write_textpage(client, pos);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
             break;
          }

          if (c == '\n') 
          {
            // you're starting a new line
            currentLineIsBlank = true;
            //serial_println(theCurrentLine);
            p = parse_theCurrentLine(theCurrentLine);
            
            if(p != "0")
            {
              write_webpage(client, p);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!             
              set_horz_servo(Horz_from_p(p));
              set_vert_servo(Vert_from_p(p));
              pos = p;
              delay(milliseconds_of_delay);
              break;              
            }
           // if(newmessage)
           // {
           //   send_standard_header(client);
           // }
            client.println(theCurrentLine);
            theCurrentLine = String("");
          } 
          else if (c != '\r') 
          {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
            theCurrentLine =  String(theCurrentLine + c);
          }
          
      }//=====================================================================
      //client.stop();
    }
    // close the connection:
    client.stop();
    client.flush();
    serial_println("<<<<<<<<<<");
    serial_println(" ");
    //delay(milliseconds_of_delay);
  }else
  {
     //no client there
     delay(milliseconds_of_delay);
     loop_count++;
     if(loop_count > 100)
     {
       loop_count = 0;
       if(vert_servo.attached())
       {
         vert_servo.detach();
       }
       if(vert_servo.attached())
       {
          vert_servo.detach(); 
       }
     }
  }
}

void clearArray()
{
  for (int i = 0; i < RCOUNT; i++)
  {
    Refererers[i] = ".";
  }  
  RefererersCounterer = 0;
}

void get_ip(byte mac[])
{
  //theCurrentLine =  String(theCurrentLine + c);
  String the_dhcp_ip;
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) 
  {
    serial_println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    set_horz_servo("1");
    delay(5000);
    //need to return or something
  }
  else
  {
    // print your local IP address:
    serial_print("My IP address: ");
    for (byte thisByte = 0; thisByte < 4; thisByte++) 
    {
      // print the value of each byte of the IP address:
      the_dhcp_ip = String(the_dhcp_ip + String(thisByte, DEC));
      //serial_print(Ethernet.localIP()[thisByte], DEC);
      //serial_print("."); 
      the_dhcp_ip = String(the_dhcp_ip + ".");
    }
    serial_println(the_dhcp_ip);    
  }
}

void set_horz_servo(String one_to_100)
{
  int the_length = one_to_100.length() + 1;//+1 includes the null terminating character
  char Str1to100[the_length];
  one_to_100.toCharArray(Str1to100, the_length);
  int the_int = atoi(Str1to100);
  the_int = constrain(the_int, 1, 100);
  int reverse_mapped = map(the_int, 1,100,170,1);
  if(horz_servo.attached() == false)
  {
    horz_servo.attach(horz_servo_pin);
  }
  horz_servo.write(reverse_mapped);
}

void set_vert_servo(String one_to_100)
{
  int the_length = one_to_100.length() + 1;//+1 includes the null terminating character
  char Str1to100[the_length];
  one_to_100.toCharArray(Str1to100, the_length);
  int the_int = atoi(Str1to100);
  the_int = constrain(the_int, 1, 100);
  int reverse_mapped = map(the_int, 1,100,170,1);
  if(vert_servo.attached() == false)
  {
    vert_servo.attach(vert_servo_pin);
  }
  vert_servo.write(reverse_mapped);
}

String Vert_from_p(String theP)
{
  int thesep = theP.indexOf(',');
  String v = "0";
  if(thesep != -1)
  {
    v = theP.substring(thesep + 1, theP.length() - 1);
  }
  return v;
}

String Horz_from_p(String theP)
{
  int thesep = theP.indexOf(',');
  String h = "0";
  if(thesep != -1)
  {
    h = theP.substring(0, thesep - 1);
    
  } 
  return h;
}

String parse_theCurrentLine(String theCurrentLine)
{
  /*
    GET /?p=10 HTTP/1.1
    Host: 192.168.0.3
    Connection: keep-alive
    Cache-Control: max-age=0
    Accept: text/html,application/xhtml+xml,application/xml;q=0.9,**;q=0.8
    User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.95 Safari/537.36
    Referer: http://192.168.0.3/?p=10
    Accept-Encoding: gzip,deflate,sdch
    Accept-Language: en-US,en;q=0.8
  */ 
  
  //find GET / ?p=...[space]
  serial_println(theCurrentLine);
  String p = "0";
  String my_message = " ";
  //int secondClosingBracket = stringOne.indexOf('>', firstClosingBracket + 1 );
  //if (stringOne.substring(14,18) == "text") {
  //string.substring(from, to)
  
  // "?... "
  int start_of_p = theCurrentLine.indexOf('?');
  if(start_of_p != -1)
  {
    int end_of_p = theCurrentLine.indexOf('&', start_of_p);
    if(end_of_p == -1)
    {
      end_of_p = theCurrentLine.indexOf(' ', start_of_p);
    }
    if (theCurrentLine.substring(start_of_p,start_of_p + 3) == "?p=")
    {
      p = theCurrentLine.substring(start_of_p + 3, end_of_p);
      RefererersCounterer++;
      if(RefererersCounterer >= RCOUNT)
      {
          RefererersCounterer = 0;
      }
      Refererers[RefererersCounterer] = theCurrentLine; 
    }
  }
  if(echo)
  {
      if(RefererersCounterer >= RCOUNT)
      {
          RefererersCounterer = 0;
      }
      Refererers[RefererersCounterer] = theCurrentLine;  
      RefererersCounterer++;  
  }
  serial_println("read:" + theCurrentLine);
  serial_println("p=" + p);
  //find the referer line
  return p;
}

void print_refererers(EthernetClient client)
{
  client.println(Refererers[RefererersCounterer]);
  for (int i = RefererersCounterer-1; i > 0; i--)
  {
    //serial_println(Refererers[i]);
    client.println(Refererers[i]);
  }
  //client.println(" <br />");
  for (int i = RCOUNT-1; i > RefererersCounterer ; i--)
  {
    //serial_println(Refererers[i]);
    client.println(Refererers[i] + "  ");
  }
}

void write_textpage(EthernetClient client, String p)
{
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  print_refererers(client);
  client.println("");  
  client.println("");
}

void send_standard_header(EthernetClient client)
{
  newmessage = false;
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();  
}
void write_webpage(EthernetClient client, String p)
{
  send_standard_header(client);
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<br />");
  client.println("p=" + p + "<br />");
  print_refererers(client);
  client.println("<br />");
  /*
  // output the value of each analog input pin
  for (int analogChannel = 0; analogChannel < 3; analogChannel++) {
    int sensorReading = analogRead(analogChannel);
    client.print("analog input ");
    client.print(analogChannel);
    client.print(" is ");
    client.print(sensorReading);
    client.println("<br />");       
  }
  */
  client.println("</html>");  
}

