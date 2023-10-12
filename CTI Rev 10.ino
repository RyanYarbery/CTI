//
//
//          CCCCCCCCCCCCC        TTTTTTTTTTTTTTTTTTTTTTT        IIIIIIIIII
//       CCC::::::::::::C        T:::::::::::::::::::::T        I::::::::I
//     CC:::::::::::::::C        T:::::::::::::::::::::T        I::::::::I
//    C:::::CCCCCCCC::::C        T:::::TT:::::::TT:::::T        II::::::II
//   C:::::C       CCCCCC        TTTTTT  T:::::T  TTTTTT          I::::I
//  C:::::C                              T:::::T                  I::::I
//  C:::::C                              T:::::T                  I::::I
//  C:::::C                              T:::::T                  I::::I
//  C:::::C                              T:::::T                  I::::I
//  C:::::C                              T:::::T                  I::::I
//  C:::::C                              T:::::T                  I::::I
//   C:::::C       CCCCCC                T:::::T                  I::::I
//    C:::::CCCCCCCC::::C              TT:::::::TT              II::::::II
//     CC:::::::::::::::C ......       T:::::::::T       ...... I::::::::I
//       CCC::::::::::::C .::::.       T:::::::::T       .::::. I::::::::I
//          CCCCCCCCCCCCC ......       TTTTTTTTTTT       ...... IIIIIIIIII
//
//
//                              (Childhood Trauma Inducer)
//
//
//
//

//| Created On: 2/6/23
//| By: Ryan Yarbery
//|
//| Description:
//| This program is needed for functionality of the C.T.I. and is a mutation of the V6 program created
//| by Tungsten EXE.
//|
//| First fully functioning Rev. Just need voltage issue to be fixed.

//_______________________________________________________________________________________________________
// BEGINNING OF PROGRAM           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
#define ENCODER_DO_NOT_USE_INTERRUPTS                             // Self Explanatory

#include <Bounce2.h>                                              // https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>			                                    // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>   		                              // https://github.com/adafruit/Adafruit_SSD1306
#include <Encoder.h>


// Pin Assignments
//_______________________________________________________________________________________________________
constexpr uint8_t PIN_TRIG_REV               = 2;                     // PIN for button to Rev
constexpr uint8_t PIN_SOLENOID_MOSFET        = 3;    		              // PIN Signal Mosfet Gate to Solenoid
constexpr uint8_t PIN_ENC_SW                 = 4;                     // PIN Signal Rotary Encoder pin 1
constexpr uint8_t PIN_ESC_TOP                = 5;                     // PIN to control ESC, normally the white wire from ESC
constexpr uint8_t PIN_ESC_BOT                = 6;                     // PIN to control ESC, normally the white wire from ESC
constexpr uint8_t PIN_ENC_CLK                = 7;     	              // PIN Signal Rotary Encoder pin 2
constexpr uint8_t PIN_ENC_DT                 = 8;   	              	// PIN Signal PWM Rotary Encoder pin 2
constexpr uint8_t PIN_SWITCH_BURST           = 9;    		              // PIN receiving signal from Position I on 3 position switch. Burst Fire
constexpr uint8_t PIN_SWITCH_AUTO            = 10;   		              // PIN receiving signal from Position II on 3 position switch. Automatic Fire
constexpr uint8_t PIN_TRIG_FIRE	             = 11;  		              // PIN receiving signal from trigger
constexpr uint8_t PIN_MAG_RESET              = 12; 	  	              // PIN receiving signal from Mag reset switch

constexpr uint8_t PIN_OLED_RESET             = 13;                    // OLED Reset
constexpr uint8_t PIN_SAFETY                 = 14;		                // PIN receiving signal from safety switch
constexpr uint8_t PIN_FLYWHEEL_MOSFET        = 15;                    // PIN Signal Mosfet Gate to ESC X 2
#define           PIN_SENSOR_VOLT            = A6	   	              // PIN receiving signal from voltage sensor

// End of Pin Assignments TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

const int SCREEN_WIDTH                   = 128;                   // OLED display width, in pixels
const int SCREEN_HEIGHT                  = 64;                    // OLED display height, in pixels

const int AMMO_MAX                       = 99 ;  		              // Maxmimum ammo
const uint8_t AMMO_MIN                   = 1;    		              // Minimum  ammo

const int BURST_MAX          	           = 99;   		              // Maxmimum rounds per burst
const uint8_t BURST_MIN                  = 2;    		              // Minimum rounds per burst

const uint8_t MODE_SINGLE                = 0;                     // Integer constant to indicate firing single shot
const uint8_t MODE_BURST                 = 1;                     // Integer constant to indicate firing burst
const uint8_t MODE_AUTO                  = 2;                     // Integer constant to indicate firing full auto
const uint8_t NUM_OF_MODE                = 3;                     // Number of mode available

const uint8_t DEFAULT_BURST_NUM          = 3;                     // Default number of burst fire darts

const uint8_t MODE_ROF_LOW               = 0;                     // Integer constant to indicate low rate of fire 
const uint8_t MODE_ROF_STANDARD          = 1;                     // Integer constant to indicate standard rate of fire 
const uint8_t MODE_ROF_HIGH              = 2;                     // Integer constant to indicate highest rate of fire 
const uint8_t NUM_OF_MODE_ROF            = 3;                     // Number of ROF available

const int REV_UP_DELAY                   = 100;                   // Increase/decrease this to control the flywheel rev-up time (in milliseconds)

const int BATTERY_MIN                    = 9.8;  		              // Min voltage of battery
const int BATTERY_MIN_3DIGIT             = 98;   		              // Min voltage of battery
const int BATTERY_MAX_3DIGIT             = 123;  	              	// Max voltage

//  ESC Spec
//_______________________________________________________________________________________________________
const int THROTTLE_DEFAULT               = 1300;
const int THROTTLE_MIN                   = 1000;
const int THROTTLE_MAX                   = 2000;

// End of ESC setting TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

int throttle                             = THROTTLE_DEFAULT;

String  rofLimitStrArr []                = {"        <<<>>>", "     <<<<<<>>>>>>", "<<<<<<<<<>>>>>>>>>"}; 
int     modeROFSelected                  = MODE_ROF_HIGH;         // Track the ROF selected, set default to High

int     delaySolenoidExtended  []        = {80, 75, 60};
int     delaySolenoidRetracted []        = {80, 55, 45};
  
int     ammoLimit                        = 18;                    // Default set as 18 dart mag
int     burstLimit;                                               // Darts per burst

uint8_t     modeFire                     = MODE_SINGLE;           // Track the mode of fire, Single, Burst or Auto, Single by default
uint8_t     dartChambered                = 0;                     // Track amount of dart(s) to fire when trigger pulled, 0 by default
int     dartQty                          = ammoLimit;             // Track amount of dart in mag, same default value as of ammoLimit
float   currentVoltage                   = 99.0;
int offset                               = 20;

long position                            = THROTTLE_DEFAULT;

boolean batteryLow                       = false;                 // Track battery status
boolean isRevving                        = false;                 // Track if blaster firing         
boolean isFiring                         = false;                 // Track if blaster firing         
boolean magOut                           = false;                 // Track is there a mag 
boolean safetyOn                         = false;                 // Track is safetyOn 

// C.T.R.- Childhood Trauma Reduced (Low FPS Mode)
boolean isCTRMode                         = false;                 // True when CTR mode is selected
boolean isCTRModeFullAuto                 = false;                 // True when CTR mode is on and is on full auto firing

unsigned long timerSolenoidDetect        = 0;
boolean       isSolenoidExtended         = false;

// Create Servo objects for motor control
Servo motorTop;                                                                    
Servo motorBot;                                                                    

// Create Encoder object and assign pins
Encoder knob(4,7);                                                

// Create Display object and assign pins and resolution.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, PIN_OLED_RESET);  

// Declare and Instantiate Bounce objects
Bounce btnRev                           = Bounce();
Bounce btnTrigger                       = Bounce();
Bounce switchSelectorOne                = Bounce();
Bounce switchSelectorTwo                = Bounce();
Bounce btnMagReset                      = Bounce();
Bounce btnSafety                        = Bounce();
Bounce rotEncSW                         = Bounce();

//_______________________________________________________________________________________________________
// Function: setup
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void setup() { // Initialze  

  //_____________________________________________________________________________________________________
  // INPUT PINs setup
  // Note: Most input pins will be using internal pull-up resistor. A fall in signal indicate button pressed.
  //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  pinMode(PIN_TRIG_REV,INPUT_PULLUP);              // PULLUP
  btnRev.attach(PIN_TRIG_REV);
  btnRev.interval(5);
    
  pinMode(PIN_TRIG_FIRE,INPUT_PULLUP);      // PULLUP
  btnTrigger.attach(PIN_TRIG_FIRE);
  btnTrigger.interval(5);

  pinMode(PIN_SWITCH_BURST,INPUT_PULLUP);     // PULLUP
  switchSelectorOne.attach(PIN_SWITCH_BURST);
  switchSelectorOne.interval(5);

  pinMode(PIN_SWITCH_AUTO,INPUT_PULLUP);     // PULLUP
  switchSelectorTwo.attach(PIN_SWITCH_AUTO);
  switchSelectorTwo.interval(5);

  pinMode(PIN_MAG_RESET, INPUT_PULLUP);       // PULLUP
  btnMagReset.attach(PIN_MAG_RESET);
  btnMagReset.interval(5);

  pinMode(PIN_SAFETY, INPUT_PULLUP);          // PULLUP
  btnSafety.attach(PIN_SAFETY);  
  btnSafety.interval(5);
  
  pinMode(PIN_ENC_SW, INPUT_PULLUP);          // PULLUP
  rotEncSW.attach(PIN_ENC_SW);  
  rotEncSW.interval(5);

  pinMode(PIN_SENSOR_VOLT, INPUT);               // Not using PULLUP analog read 0 to 1023

  //_____________________________________________________________________________________________________
  // OUTPUT PINs setup
  //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT  
  pinMode(PIN_FLYWHEEL_MOSFET, OUTPUT);
  digitalWrite(PIN_FLYWHEEL_MOSFET, HIGH);  

  pinMode(PIN_SOLENOID_MOSFET, OUTPUT);
  digitalWrite(PIN_SOLENOID_MOSFET, LOW);
  
  //_____________________________________________________________________________________________________
  // ESC Arming Sequence
  // Note: //Arms the ESCs. Should hear a total of 5 beeps over a few seconds to indicate completion.
  //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  motorTop.attach(PIN_ESC_TOP, THROTTLE_MIN, THROTTLE_MAX);
  motorBot.attach(PIN_ESC_BOT, THROTTLE_MIN, THROTTLE_MAX);

  motorTop.writeMicroseconds(THROTTLE_MIN);
  motorBot.writeMicroseconds(THROTTLE_MIN);

  delay(4000);

  magOut   = (digitalRead(PIN_MAG_RESET) == HIGH);
  safetyOn = (digitalRead(PIN_SAFETY) == LOW);
  modeFire = 3 - ((digitalRead(PIN_SWITCH_AUTO) * 2) + (digitalRead(PIN_SWITCH_BURST) * 1)); // 0, 1 or 2        
  isCTRMode   = (digitalRead(PIN_TRIG_FIRE) == LOW && digitalRead(PIN_TRIG_REV) == LOW);
  burstLimit = (isCTRMode) ? modeFire + 2 : DEFAULT_BURST_NUM;
  position = throttle = ((digitalRead(PIN_TRIG_REV) == LOW && digitalRead(PIN_TRIG_FIRE) == HIGH) ? THROTTLE_MAX : THROTTLE_DEFAULT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  
  if (magOut) {
    dartQty     = 0;
    updateMagOutDisplay(); 
  } else if (safetyOn) { 
    updateSafetyDisplay();
  } else {
    dartQty = ammoLimit;
    updateDisplay();   
  }
}

//_______________________________________________________________________________________________________
// Function: loop
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void loop() { // Main Loop  
  if (!batteryLow) {  
    //_____________________________________________________________________________________________________
    // Update all buttons
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    updateAllBtns();
    
    //_____________________________________________________________________________________________________
    // Rotary Encoder Handle
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    long newPos = knob.read();

    if(newPos != position) {
      position = newPos;
      
      if (position >= 2000) {
      position = 2000;

      } else if(position <= 1100){
      position = 1100;
      
      }
      
      throttle = position;
      
    }
    
    //___________________________________________________________________________________________________
    // Listen to Mag Out
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    if (btnMagReset.fell()) { // Pressed, there is a Mag in the blaster
      dartQty = ammoLimit;
      magOut   = false;      
      
      if (safetyOn) {
        updateSafetyDisplay();
      } else {
        updateDisplay();
      }
    } else if (btnMagReset.rose()) { // No Mag in the blaster      
      shutdownSys();
      magOut = true;
      updateMagOutDisplay();
    }

    //___________________________________________________________________________________________________
    // Listen to Safety
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    if (btnSafety.fell()) {               // Safety on
      safetyOn = true;
      shutdownSys(); 
      
      if (!magOut) {
        updateSafetyDisplay();
      }
    } else if (btnSafety.rose()) {        // Safety off
      safetyOn = false;
      if (!magOut) {
        updateDisplay();
      }
    }

    //___________________________________________________________________________________________________
    // Listen to Rev Press/Release
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    if (btnRev.fell()) {                   // Rev pressed
      if (magOut) {
        if (ammoLimit > AMMO_MIN) {
          ammoLimit--;      
          updateMagOutDisplay();
        }
      } else if (safetyOn) {
          if (digitalRead(PIN_TRIG_FIRE) == LOW ) {
            modeROFSelected = ++modeROFSelected % NUM_OF_MODE_ROF;
            updateSafetyDisplay();
          }   
      } else if (isCTRMode) { 
          if (digitalRead(PIN_TRIG_FIRE) == HIGH) {
            triggerPressedHandle(MODE_BURST);
            isCTRModeFullAuto = false;
          } else {
            triggerPressedHandle(MODE_AUTO);
            isCTRModeFullAuto = true;
          }    
      } else {      
        motorTop.writeMicroseconds(throttle);  // Start flywheels
        motorBot.writeMicroseconds(throttle); 
        isRevving = true;
      }
    } else if (btnRev.rose() && !isCTRMode) {        // Rev released
      isRevving = false;
      if (!isFiring) {        
        motorTop.writeMicroseconds(THROTTLE_MIN);  // Start flywheels
        motorBot.writeMicroseconds(THROTTLE_MIN);  
      }
    }
  
    //___________________________________________________________________________________________________
    // Listen to Trigger Pull/Release
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    if (btnTrigger.fell()) {               // Trigger pull
      if (magOut) {
        if (ammoLimit < AMMO_MAX) {
          ammoLimit++;      
          updateMagOutDisplay();
        }
      } else if (safetyOn) {
        if (digitalRead(PIN_TRIG_REV) == LOW && !isCTRMode) {
          burstLimit = (burstLimit == BURST_MAX) ? BURST_MIN : burstLimit + 1;
          updateSafetyDisplay();
        }
      } else if (isCTRMode) {
        if (digitalRead(PIN_TRIG_REV) == HIGH) {
          triggerPressedHandle(MODE_SINGLE);
          isCTRModeFullAuto = false;
        } else {
          triggerPressedHandle(MODE_AUTO);
          isCTRModeFullAuto = true;
        }    
      } else {
        triggerPressedHandle(modeFire);
      }        
    } else if (btnTrigger.rose()) {        // Trigger released
      triggerReleasedHandle();
    }
  
    //___________________________________________________________________________________________________
    // Listen to Firing
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    shotFiringHandle();
  
    //___________________________________________________________________________________________________
    // Listen to Firing Mode change: Single Shot, Burst, Full Auto
    //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    if (switchSelectorOne.fell() || switchSelectorOne.rose() || switchSelectorTwo.fell() || switchSelectorTwo.rose() ) {  
      modeFire = 3 - ((digitalRead(PIN_SWITCH_AUTO) * 2) + (digitalRead(PIN_SWITCH_BURST) * 1)); // 0, 1 or 2    
      if (isCTRMode) {
        burstLimit = modeFire + 2;
      }
      if (!magOut && !safetyOn) {
        updateDisplay();
      }
    }    
  } else {
    // Battery is Low
    // Stop all Motors just in case.
    shutdownSys();    
    updateBatteryLowDisplay();
  }
}

//_______________________________________________________________________________________________________
// Function: Update Bounce Objects
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateAllBtns() {

  btnRev.update();
  btnTrigger.update();
  switchSelectorOne.update();
  switchSelectorTwo.update();
  btnMagReset.update();
  btnSafety.update();
  rotEncSW.update();
  //Serial.println("Updated Buttons");
}


//_______________________________________________________________________________________________________
// Function: shotFiredHandle
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void shotFiringHandle() {
  if (isFiring) {
    if (isSolenoidExtended) {
      if ((millis() - timerSolenoidDetect) >= delaySolenoidExtended[modeROFSelected]) {
        digitalWrite(PIN_SOLENOID_MOSFET, LOW); // Retract Solenoid

        // Display refresh is too time consuming so im just changing text color 
        // to background color and then writing new text in contrasting color.
        display.setTextColor(BLACK);
        display.setCursor(90,18);
        display.print(dartQty);
  
        dartQty--;     // Decrease dart count
        dartChambered--;

        // Switch back to white text and write the new count
        display.setTextColor(WHITE);
        display.setCursor(90,18);
        display.print(dartQty);
        display.display();
        
        isSolenoidExtended = false;
        timerSolenoidDetect = millis();        
      }
    } else { // Solenoid had returned to rest position
      if (dartChambered == 0) {
        isFiring = false;
        if (!isRevving || isCTRMode) { // Rev button not pressed or in CTR mode
          isRevving = false;
          motorTop.writeMicroseconds(THROTTLE_MIN);
          motorBot.writeMicroseconds(THROTTLE_MIN);
        }
        updateDisplay();
      } else if ((millis() - timerSolenoidDetect) >= delaySolenoidRetracted[modeROFSelected]) {
        digitalWrite(PIN_SOLENOID_MOSFET, HIGH); // Extend Solenoid
        isSolenoidExtended = true;
        timerSolenoidDetect = millis();
      }      
    }
  }
}

//_______________________________________________________________________________________________________
// Function: triggerPressedHandle
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void triggerPressedHandle(uint8_t caseModeFire) {  
  if (dartQty > 0) {
    if (isCTRMode && !isRevving) {
      // Start flywheels
      motorTop.writeMicroseconds(throttle);  
      motorBot.writeMicroseconds(throttle); 
      delay(REV_UP_DELAY);
      isRevving = true;
    }
    if (isRevving){
      switch(caseModeFire) {
        case MODE_SINGLE: dartChambered++; 
          break;
        case MODE_BURST : dartChambered += burstLimit; 
          if (dartChambered > dartQty) {
              dartChambered = dartQty;
          }
          break;
        case MODE_AUTO  : dartChambered = dartQty;
      }
      // Start Firing    
      if (!isFiring) {
        isFiring = true;
        display.setTextSize(3);
  
        digitalWrite(PIN_SOLENOID_MOSFET, HIGH); // Extend pusher
        timerSolenoidDetect = millis();
        isSolenoidExtended = true;      
      }    
    }
  }
}

//_______________________________________________________________________________________________________
// Function: triggerReleasedHandle
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void triggerReleasedHandle() {  
  if ((modeFire == MODE_AUTO || isCTRModeFullAuto) && isFiring) {
    isCTRModeFullAuto = false;
    if (dartChambered > 1) {      
      dartChambered = 1;    // Fire off last shot
    }
  }
}

//_______________________________________________________________________________________________________
// Function: readVoltage
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void readVoltage() {
  int voltagePinAnalogValue = analogRead(PIN_SENSOR_VOLT);
    
  // Might have to adjust the formula according to the voltage sensor you use
  int   voltagePinValue =  map(voltagePinAnalogValue,0,1023, 0, 2500) + offset;
  float newVoltage      = (voltagePinValue / 100.0);

  if (!batteryLow) {
    currentVoltage = (newVoltage < currentVoltage) ? newVoltage : currentVoltage;      
  } else {
    currentVoltage = (newVoltage > BATTERY_MIN) ? newVoltage : currentVoltage;  
  }
  batteryLow = (currentVoltage <= BATTERY_MIN);
}

//_______________________________________________________________________________________________________
// Function: updateNormalDisplay
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateNormalDisplay() {
  readVoltage();
  int numOfCircle = 1;
  int intCurrentVolt = (int) (currentVoltage * 10);

  if (intCurrentVolt < BATTERY_MIN_3DIGIT) {
    intCurrentVolt = BATTERY_MIN_3DIGIT;
  } else if (intCurrentVolt > BATTERY_MAX_3DIGIT) {
    intCurrentVolt = BATTERY_MAX_3DIGIT;
  }
  
  int batt = map(intCurrentVolt, BATTERY_MIN_3DIGIT, BATTERY_MAX_3DIGIT, 0, 16);
  int fwPwrPrcntge = map(throttle, THROTTLE_MIN, THROTTLE_MAX, 0, 100);  // Map throttle to Percentage

  display.clearDisplay();
  
  display.fillRect(0, 0, (8*batt), 4, WHITE);
  for (int i=1; i<=batt; i++) {
    display.drawLine(i*8, 0, i*8, 4, BLACK);
  }
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,8);
  display.print(">> ");
  display.print(currentVoltage);
  display.println(" volts");  
  
  display.setCursor(0,19);
  display.print(fwPwrPrcntge);
  display.print("%");
  display.println("-C.T.I-");  
  
  display.setCursor(0,32);
  
  switch(modeFire) {
      case MODE_SINGLE: 
          display.println("Single Shot");  
        break;
      case MODE_BURST : 
          numOfCircle = burstLimit;
          display.print(burstLimit);          
          display.println(" Rounds Burst");  
        break;
      case MODE_AUTO  : 
          numOfCircle = 10;
          display.println("Full Auto");  
        break;
    }
    
  display.setCursor(0,57);
  display.println(rofLimitStrArr[modeROFSelected]);  
  
  display.setCursor(90,18);
  display.setTextSize(3);
  display.println(dartQty);  

  for(int i=0; i<numOfCircle; i++) {
    display.fillCircle((i * 9) + 3, 48, 3, WHITE);
  }
  display.display();
}

//_______________________________________________________________________________________________________
// Function: updateCTRModeDisplay
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateCTRModeDisplay() {
  readVoltage();
  int numOfCircle = burstLimit;
  int intCurrentVolt = (int) (currentVoltage * 10);

  if (intCurrentVolt < BATTERY_MIN_3DIGIT) {
    intCurrentVolt = BATTERY_MIN_3DIGIT;
  } else if (intCurrentVolt > BATTERY_MAX_3DIGIT) {
    intCurrentVolt = BATTERY_MAX_3DIGIT;
  }
  
  int batt = map(intCurrentVolt, BATTERY_MIN_3DIGIT, BATTERY_MAX_3DIGIT, 0, 16);
  int fwPwrPrcntge = map(throttle, THROTTLE_MIN, THROTTLE_MAX, 0, 100);  // Map throttle to Percentage

  display.clearDisplay();
  
  display.fillRect(0, 0, (8*batt), 4, WHITE);
  for (int i=1; i<=batt; i++) {
    display.drawLine(i*8, 0, i*8, 4, BLACK);
  }
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,8);
  display.print(">> ");
  display.print(currentVoltage);
  display.println(" volts");  
      
  display.setCursor(0,57);
  display.println(rofLimitStrArr[modeROFSelected]);  

  display.setTextSize(2);
  display.setCursor(0,23);
  display.print(fwPwrPrcntge + "%");
  display.println("~C.T.I~");   

  display.setCursor(90,18);
  display.setTextSize(3);
  display.println(dartQty);  

  for(int i=0; i<numOfCircle; i++) {
    display.fillCircle((i * 9) + 3, 48, 3, WHITE);
  }
  display.display();
}

//_______________________________________________________________________________________________________
// Function: updateDisplay
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateDisplay() {
  if (isCTRMode) {
    updateCTRModeDisplay();
  } else {
    updateNormalDisplay();
  }  
}

//_______________________________________________________________________________________________________
// Function: updateMagOutDisplay
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateMagOutDisplay() {
  readVoltage();
  
  int intCurrentVolt = (int) (currentVoltage * 10);

  if (intCurrentVolt < BATTERY_MIN_3DIGIT) {
    intCurrentVolt = BATTERY_MIN_3DIGIT;
  } else if (intCurrentVolt > BATTERY_MAX_3DIGIT) {
    intCurrentVolt = BATTERY_MAX_3DIGIT;
  }
  
  int batt = map(intCurrentVolt, BATTERY_MIN_3DIGIT, BATTERY_MAX_3DIGIT, 0, 16);

  display.clearDisplay();
  
  display.fillRect(0, 0, (8*batt), 4, WHITE);
  for (int i=1; i<=batt; i++) {
    display.drawLine(i*8, 0, i*8, 4, BLACK);
  }
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,8);
  display.print(">> ");
  display.print(currentVoltage);
  display.println(" volts");  
  
  display.setCursor(0,19);
  display.println("MAG OUT - SET");  
      
  display.setCursor(0,57);
  display.println("<*-*-*-*-*^*-*-*-*-*>");  

  display.setTextSize(2);
  display.setCursor(0,32);
  display.println("-AMMO-");  
  
  display.setCursor(90,18);
  display.setTextSize(3);
  display.println(ammoLimit);  

  display.display();
}

//_______________________________________________________________________________________________________
// Function: updateSafetyDisplay
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateSafetyDisplay() {
  readVoltage();
  int numOfCircle = burstLimit;
  int intCurrentVolt = (int) (currentVoltage * 10);

  if (intCurrentVolt < BATTERY_MIN_3DIGIT) {
    intCurrentVolt = BATTERY_MIN_3DIGIT;
  } else if (intCurrentVolt > BATTERY_MAX_3DIGIT) {
    intCurrentVolt = BATTERY_MAX_3DIGIT;
  }
  
  int batt = map(intCurrentVolt, BATTERY_MIN_3DIGIT, BATTERY_MAX_3DIGIT, 0, 16);

  display.clearDisplay();
  
  display.fillRect(0, 0, (8*batt), 4, WHITE);
  for (int i=1; i<=batt; i++) {
    display.drawLine(i*8, 0, i*8, 4, BLACK);
  }
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,8);
  display.print(">> ");
  display.print(currentVoltage);
  display.println(" volts");  
      
  display.setCursor(0,57);
  display.println(rofLimitStrArr[modeROFSelected]);  

  display.setTextSize(2);
  display.setCursor(0,21);
  display.println("SAFETY");  

  display.setCursor(90,18);
  display.setTextSize(3);
  display.println(dartQty);  

  for(int i=0; i<numOfCircle; i++) {
    display.fillCircle((i * 9) + 3, 48, 3, WHITE);
  }
  display.display();
}


//_______________________________________________________________________________________________________
// Function: updateBatteryLowDisplay
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void updateBatteryLowDisplay() {
  readVoltage();
  if (batteryLow) {
    display.clearDisplay();
        
    display.setTextSize(2);
    display.setCursor(0,14);
    display.println("+-------+");  
    display.println("|BattLow|=");  
    display.println("+-------+");  
  
    display.display();
  } else {
    if (magOut) {
      updateMagOutDisplay();
    } else {
      updateDisplay();
    }
  }
}

//_______________________________________________________________________________________________________
// Function: shutdown
//           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
void shutdownSys() {
  dartChambered = 0;
  motorTop.writeMicroseconds(THROTTLE_MIN);
  motorBot.writeMicroseconds(THROTTLE_MIN);
  digitalWrite(PIN_SOLENOID_MOSFET, LOW);
  digitalWrite(PIN_FLYWHEEL_MOSFET, LOW);
  isFiring = false;
}



//_______________________________________________________________________________________________________
// END OF PROGRAM           
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
