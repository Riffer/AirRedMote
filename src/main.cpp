#include <Arduino.h>
#include <OneButton.h>

/*
 * define pins (standard), tell frequency and adjust interrupt handling
 */
#include "PinDefinitionsAndMore.h"

#define IRSND_IR_FREQUENCY 38000

#define IRMP_USE_COMPLETE_CALLBACK 1     // Enable callback functionality
#define IRMP_ENABLE_PIN_CHANGE_INTERRUPT // Enable interrupt functionality
#define USE_ONE_TIMER_FOR_IRMP_AND_IRSND // compile error?
#define IRMP_PROTOCOL_NAMES 1            // Enable protocol number mapping to protocol strings - needs some FLASH. Must before #include <irmp*>
#define IRSND_PROTOCOL_NAMES 1           // Enable protocol number mapping to protocol strings - requires some FLASH.

/*
 * single protocol to save space
 */
#define IRSND_SUPPORT_NEC_PROTOCOL 1 // NEC + APPLE          >= 10000                 ~100 bytes
#define IRMP_SUPPORT_NEC_PROTOCOL 1 // NEC + APPLE + ONKYO  >= 10000                 ~300 bytes

/*
 * After setting the definitions we can include the code and compile it.
 */
#include <irmp.hpp>
#include <irsnd.hpp>

/***
 * send and receive objects
 ***/
IRMP_DATA irmp_data;
IRMP_DATA irsnd_data;

/*** 
 * to handle receive package in loop() and not in interrupt
 ***/
bool sJustReceived;

bool holdingA1 = false;
bool stop = false;

/***
 *  OneButton on pin A1 - connect ground to A1 via momentaneous switch
 ***/
OneButton buttonA1(A1, true);

/**
 * @brief interrupt function for infrared received data
 * 
 **/
void handleReceivedIRData()
{
  irmp_get_data(&irmp_data);
  if (!holdingA1)
    sJustReceived = true; // Signal new data for main loop, this is the recommended way for handling a callback :-)
  interrupts();         // enable interrupts
}

/**
 * @brief try to send signal asynchronusly by infrared and print if succeeded 
 * 
 * @return true if signal sent
 * @return false if no signal sent
 */
bool send_signal()
{
  irsnd_data.command++;
  if(irsnd_data.command>0xff) irsnd_data.command = 0x00;
  if(irsnd_send_data(&irsnd_data, false))
  {
    irsnd_data_print(&Serial, &irsnd_data);
    return true;
  }
  return false;
}

/**
 * @brief click event handler
 * 
 */
void buttonA1_click()
{
  if (send_signal())
  {
  digitalWrite(A0, LOW);
  delay(250);
  digitalWrite(A0, HIGH);
  }
}

void buttonA1_longPress()
{
  holdingA1 = true;
}

void buttonA1_doubleclick()
{
  stop = true;
  holdingA1 = false;
}

void buttonA1_hold_start()
{
  //holdingA1 = true;
}
void buttonA1_hold_stop()
{
  //holdingA1 = false;
}
    /**
     * @brief set button events and infrared including interrupts
     *
     */
    void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
 
  Serial.println(F("START " __FILE__ " from " __DATE__ " using IRMP library version " VERSION_IRMP));

  /***
   * attach same event handler to one and long click
   ***/
  buttonA1.attachClick(buttonA1_click);
  buttonA1.attachDuringLongPress(buttonA1_longPress);
  buttonA1.attachDoubleClick(buttonA1_doubleclick);
  buttonA1.attachLongPressStart(buttonA1_hold_start);
  buttonA1.attachLongPressStop(buttonA1_hold_stop);

  /***
   * setup ir sending
   ***/
  irsnd_init();
  irmp_irsnd_LEDFeedback(true); // Enable send signal feedback at LED_BUILTIN
  irsnd_data.protocol = IRMP_NEC_PROTOCOL;
  irsnd_data.address = 0x0707;
  irsnd_data.command = 0x01; // The required inverse of the 8 bit command is added by the send routine.
  irsnd_data.flags = 1;      // repeat frame 1 times

  /***
   * setup ir receiving
   ***/
  irmp_init();
  irmp_register_complete_callback_function(&handleReceivedIRData);

  /***
   * for external buzzer (Matek can be triggered by signal on ground)
   ***/
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH); // turn buzzer off
}

/**
 * @brief loop for button-event and received data if any
 * 
 */
void loop() 
{
  buttonA1.tick();

  if (stop)
  {
    cli();
    delay(1000);
    sJustReceived = false;
    stop = false;
    sei();
  }

  if(holdingA1)
  {
    send_signal();
  }

  if(sJustReceived)
  {
    digitalWrite(A0, LOW);
    irmp_result_print(&irmp_data);
    delay(250);
    digitalWrite(A0, HIGH);
    sJustReceived = false;
    if (irmp_data.command != irsnd_data.command ) send_signal();
  }
}

