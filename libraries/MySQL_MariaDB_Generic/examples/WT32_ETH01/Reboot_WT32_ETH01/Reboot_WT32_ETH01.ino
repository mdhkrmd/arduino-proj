/*********************************************************************************************************************************
  Reboot_WiFi.ino

  Library for communicating with a MySQL or MariaDB Server

  Based on and modified from Dr. Charles A. Bell's MySQL_Connector_Arduino Library https://github.com/ChuckBell/MySQL_Connector_Arduino
  to support nRF52, SAMD21/SAMD51, SAM DUE, STM32F/L/H/G/WB/MP1, ESP8266, ESP32, etc. boards using W5x00, ENC28J60, LAM8742A Ethernet,
  WiFiNINA, ESP-AT, built-in ESP8266/ESP32 WiFi.

  The library provides simple and easy Client interface to MySQL or MariaDB Server.

  Built by Khoi Hoang https://github.com/khoih-prog/MySQL_MariaDB_Generic
  Licensed under MIT license
 **********************************************************************************************************************************/
/*
  MySQL Connector/Arduino Example : reboot if connection lost

  This example demonstrates how to reboot an Arduino if connection to the
  server is lost for a period of time.
  
  For more information and documentation, visit the wiki:
  https://github.com/ChuckBell/MySQL_Connector_Arduino/wiki.

  INSTRUCTIONS FOR USE

  1) Change the address of the server to the IP address of the MySQL server
  2) Change the user and password to a valid MySQL user and password
  3) Connect a USB cable to your Arduino
  4) Select the correct board and port
  5) Compile and upload the sketch to your Arduino
  6) Once uploaded, open Serial Monitor (use 115200 speed) and observe

  To test the reboot, unplug your Ethernet cable once you see "disconnected"
  then wait for the timeout. Once the Arduino reboots, plug the cable in again
  and you should see the query processing resume.

  Created by: Dr. Charles A. Bell
*/

#if !(defined(ESP32))
  #error This code is intended to run on the WT32 boards and ESP32 platform ! Please check your Tools->Board setting.
#endif

#define MYSQL_DEBUG_PORT      Serial

// Debug Level from 0 to 4
#define _MYSQL_LOGLEVEL_      1

#include <WebServer_WT32_ETH01.h>

#include <MySQL_Generic.h>

// Select the IP address according to your local network
IPAddress myIP(192, 168, 2, 232);
IPAddress myGW(192, 168, 2, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

#define USING_HOST_NAME     true

#if USING_HOST_NAME
  // Optional using hostname, and Ethernet built-in DNS lookup
  char server[] = "your_account.ddns.net"; // change to your server's hostname/URL
#else
  IPAddress server(192, 168, 2, 112);
#endif

uint16_t server_port = 5698;    //3306;

char user[]             = "invited-guest";      // MySQL user login username
char password[]         = "the-invited-guest";  // MySQL user login password

MySQL_Connection conn((Client *)&client);
MySQL_Query query = MySQL_Query(&conn);

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  MYSQL_DISPLAY1("\nStarting Reboot_WT32_ETH01 on", BOARD_NAME);
  MYSQL_DISPLAY(WEBSERVER_WT32_ETH01_VERSION);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO, 
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_onEvent();

  WT32_ETH01_waitForConnect();

  // print out info about the connection:
  MYSQL_DISPLAY1("Connected to network. My IP address is:", ETH.localIP());
}

// Begin reboot code
int num_fails;                  // variable for number of failure attempts
#define MAX_FAILED_CONNECTS 5   // maximum number of failed connects to MySQL

void soft_reset()
{
#if WIFI_USE_SAMD
  #if ( defined(__SAMD51__) || defined(__SAMD51J20A__) || defined(__SAMD51J19A__) || defined(__SAMD51G19A__)  )
    // For SAMD51
    // see Table 17-5 Timeout Period (valid values 0-11)
    WDT->CONFIG.reg = 5;
    WDT->CTRLA.reg = WDT_CTRLA_ENABLE;
    // To check if OK or bit.ENABLE/CLEAR
    while (WDT->SYNCBUSY.bit.WEN == 1);
  
    // use the WDT watchdog timer to force a system reset.
    WDT->CLEAR.reg = 0x00;
    // To check if OK or bit.ENABLE/CLEAR
    while (WDT->SYNCBUSY.bit.WEN == 1);
  #else
    // For SAMD21, etc
    // see Table 17-5 Timeout Period (valid values 0-11)
    WDT->CONFIG.reg = 5;
    WDT->CTRL.reg = WDT_CTRL_ENABLE;
    while (WDT->STATUS.bit.SYNCBUSY == 1);
  
    // use the WDT watchdog timer to force a system reset.
    WDT->CLEAR.reg = 0x00;
    while (WDT->STATUS.bit.SYNCBUSY == 1);
  #endif
#elif WIFI_USE_NRF528XX
  //delay(1000);
  // Restart for nRF52
  NVIC_SystemReset();
#elif WIFI_USE_SAMDUE
  void(*resetFunc)() = 0;
  resetFunc();
#elif WIFI_USE_STM32
  void(*resetFunc)() = 0;
  resetFunc();

#elif WIFI_USE_TEENSY
  #if defined(__IMXRT1062__)
    // Teensy 4.1/4.0
    SCB_AIRCR = 0x05FA0004; //write value for restart for Teensy
  #else
    void(*resetFunc)() = 0;
    resetFunc();
  #endif
  
#elif ( defined(WIFI_USE_RP2040) && defined(ARDUINO_ARCH_MBED) )

  NVIC_SystemReset();
  
#endif
}

// End reboot code

void loop()
{
  if (conn.connected())
  {
    MYSQL_DISPLAY("Running a query: SHOW DATABASES");
    
    // Execute the query

    // KH, check if valid before fetching
    if ( !query.execute("SHOW DATABASES") )
    {
      MYSQL_DISPLAY("Querying error");
      return;
    }
    //////
  
    query.show_results();             // show the results
    query.close();                    // close the cursor
    conn.close();                     // close the connection
    num_fails = 0;                    // reset failures
    
    delay(10000);
  }
  else
  {
    MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
    MYSQL_DISPLAY3("User =", user, ", PW =", password);

    //if (conn.connect(server, server_port, user, password))
    if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
    {
      delay(500);
    }
    else
    {
      num_fails++;
      MYSQL_DISPLAY("Connect failed!");

      if (num_fails == MAX_FAILED_CONNECTS)
      {
        MYSQL_DISPLAY("Ok, that's it. I'm outta here. Rebooting...");
        delay(2000);
        // Here we tell the Arduino to reboot by redirecting the instruction
        // pointer to the "top" or position 0. This is a soft reset and may
        // not solve all hardware-related lockups.
        soft_reset();
      }
    }
  }

  MYSQL_DISPLAY("\nSleeping...");
  MYSQL_DISPLAY("==========================================");
}
