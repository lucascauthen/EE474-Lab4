
// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Elegoo_TFTLCD.h FOR SETUP.
//Technical support:goodtft@163.com

#define ARDUINO_ON 1
#define DEBUG 1

#if ARDUINO_ON

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#else

#include <time.h>
#include <math.h>
#include <stdio.h>

#endif

#include <limits.h> // Used for random number generation
#include <time.h>
#include <stdlib.h>

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define NONE   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFC00

#if ARDUINO_ON

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#endif
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Elegoo_TFTLCD tft;

#if ARDUINO_ON
typedef bool Bool;
#define TRUE true
#define FALSE false
#else
enum myBool {
    FALSE = 0, TRUE = 1
};
typedef enum myBool Bool;
#endif


const int PWM_OUTPUT_PIN = 23;

const char INC_CHAR = '1';
const char DEC_CHAR = '-';
const char NO_SOLAR_PANEL_INPUT = '0';
char LAST_SOLAR_PANEL_CONTROL_CHAR = NO_SOLAR_PANEL_INPUT;

const char FORWARD_CHAR = 'F';
const char BACK_CHAR = 'B';
const char LEFT_CHAR = 'L';
const char RIGHT_CHAR = 'R';
const char DRILL_START_CHAR = 'D';
const char DRILL_STOP_CHAR = 'H';
const char NO_VEHICLE_COMMAND = '/';
char LAST_VEHICLE_COMM = NO_VEHICLE_COMMAND;


unsigned long runDelay = 5000000; //5 Sec
#define DEPLOY_RETRACT_TIME SECOND*10
#define DEFAULT_DUTY_CYCLE 0.1f
#define PWM_PERIOD SECOND/2
#define PWM_COUNTER_PERIOD SECOND / 4
#define SECOND 1000000.0f

long randomGenerationSeed = 98976;
Bool shouldPrintTaskTiming = TRUE;


//Thrust Control
unsigned int ThrusterControl = 0;

//Power Management
unsigned int *BatteryLevelPtr;
unsigned short BatteryLevel = 0;
unsigned short BatteryTemp = 0;
unsigned short FuelLevel = 100;
unsigned short PowerConsumption = 0;
unsigned short PowerGeneration = 0;
#if ARDUINO_ON
unsigned short BatteryPin = A15; //Analog Pin A15 on ATMEGA 2560
unsigned short BatteryTempPin = A14;
#else
unsigned short BatteryPin = 82; //Analog Pin A15 on ATMEGA 2560
unsigned short BatteryTempPin = 83;
#endif
unsigned short BatteryLevelArray[16];
unsigned short BatteryLevelIndex = 0;

Bool BatteryRapidTemp = FALSE;
Bool BatteryOverTemp = FALSE;
Bool AcknowledgeOverTemp = FALSE;
unsigned short BatteryTempArray[16];
unsigned short BatteryTempIndex = 0;

unsigned short BatteryDelay = 600;
unsigned short BatteryTempDelay = 500;

const unsigned short BATTERY_95 = 34;
const unsigned short BATTERY_50 = 18;
const unsigned short BATTERY_10 = 3;

const unsigned short FUEL_95 = 95;
const unsigned short FUEL_50 = 50;
const unsigned short FUEL_10 = 10;

//Solar Panel Control
Bool SolarPanelState = FALSE;
Bool SolarPanelDeploy = FALSE;
Bool SolarPanelRetract = FALSE;
Bool DriveMotorSpeedInc = FALSE;
Bool DriveMotorSpeedDec = FALSE;
unsigned short DriveMotorSpeed = (unsigned short) (DEFAULT_DUTY_CYCLE * 10.0f);

//Status Management and Annunciation
//Same as Power Management

//Warning Alarm
Bool FuelLow = FALSE;
Bool BatteryLow = FALSE;

//ImageCapture Task
float ImageCaptureFrequency = 0.0f;

unsigned long LongTimeDelay = 2000000;
unsigned long ShortTimeDelay = 1000000;

//Mining vehicle communications
char Command = NULL;
char Response = NULL;

//struct TaskStruct
typedef struct TaskStruct TCB;

struct TaskStruct {
    void (*task)(void *);

    TCB *next;
    TCB *prev;
    void *taskDataPtr;
    int priority;
};

TCB *head = NULL;
TCB *tail = NULL;
TCB powerSubsystemTCB;
TCB thrusterSubsystemTCB;
TCB satelliteComsTCB;
TCB consoleDisplayTCB;
TCB warningAlarmTCB;
TCB solarPanelControlTCB;
TCB consoleKeypadTCB;
TCB vehicalComsTCB;

struct PowerSubsystemDataStruct {
    Bool *solarPanelState;
    Bool *solarPanelDeploy;
    Bool *solarPanelRetract;
    unsigned short *batteryLevel;
    unsigned short *batteryTemp;
    unsigned short *powerConsumption;
    unsigned short *powerGeneration;
    unsigned short *batteryLevelArrayIndex;
    unsigned short *batteryTempIndex;
    Bool *batteryRapidTemp;
    Bool *batteryOverTemp;
    Bool *acknowledgeOverTemp;
};
typedef struct PowerSubsystemDataStruct PowerSubsystemData;

struct SolarPanelControlDataStruct {
    Bool *solarPanelState;
    Bool *solarPanelDeploy;
    Bool *solarPanelRetract;
    Bool *driveMotorSpeedInc;
    Bool *driveMotorSpeedDec;
    unsigned short *driveMotorSpeed;
};
typedef struct SolarPanelControlDataStruct SolarPanelControlData;

struct ConsoleKeypadDataStruct {
    Bool *driveMotorSpeedInc;
    Bool *driveMotorSpeedDec;
};
typedef struct ConsoleKeypadDataStruct ConsoleKeypadData;

struct ThrusterSubsystemDataStruct {
    unsigned int *thrusterControl;
    unsigned short *fuelLevel;
};
typedef struct ThrusterSubsystemDataStruct ThrusterSubsystemData;

struct SatelliteComsDataStruct {
    Bool *fuelLow;
    Bool *batteryLow;
    Bool *solarPanelState;
    unsigned short *batteryLevel;
    unsigned short *fuelLevel;
    unsigned short *powerConsumption;
    unsigned short *powerGeneration;
    unsigned int *thrusterControl;
    unsigned short *batteryTemp;
};
typedef struct SatelliteComsDataStruct SatelliteComsData;

struct VehicleCommsDataStruct {
    char *command;
    char *responce;
};
typedef struct VehicleCommsDataStruct VehicleCommsData;

struct ConsoleDisplayDataStruct {
    Bool *fuelLow;
    Bool *batteryLow;
    Bool *solarPanelState;
    Bool *batteryRapidTemp;
    unsigned short *batteryLevel;
    unsigned short *fuelLevel;
    unsigned short *powerConsumption;
    unsigned short *powerGeneration;
    unsigned short *batteryTemp;
};
typedef struct ConsoleDisplayDataStruct ConsoleDisplayData;

struct WarningAlarmDataStruct {
    Bool *fuelLow;
    Bool *batteryLow;
    Bool *batteryRapidTemp;
    unsigned short *batteryLevel;
    unsigned short *fuelLevel;
    unsigned short *batteryTemp;
};
typedef struct WarningAlarmDataStruct WarningAlarmData;

struct ImageCaptureDataStruct {
    float *imageCaptureFrequency;
};
typedef struct ImageCaptureDataStruct ImageCaptureData;


//Controls the execution of the power subsystem
void powerSubsystemTask(void *powerSubsystemData);

void batteryRead(PowerSubsystemData *data);

void batteryTempRead(PowerSubsystemData *data);

//Controls the execution of the thruster subsystem
void thrusterSubsystemTask(void *thrusterSubsystemData);

//Controls the execution of the satellite coms subsystem
void satelliteComsTask(void *satelliteComsData);

//Controls the execution of the console display subsystem
void consoleDisplayTask(void *consoleDisplayData);

//Controls the execution of the warning alarm subsystem
void warningAlarmTask(void *warningAlarmData);

//Controls the exeuction of the solar panel constrol subsystem
void solarPanelControlTask(void *solarPanelControlData);

//Controls the execution of the Keypad task
void consoleKeypadTask(void *consoleKeypadData);

//Controls the execution of the VehicleComms task
void vehicleCommsTask(void *vehicleCommsData);

//Controls the execution of the ImageCapture task
void imageCaptureTask(void *imageCaptureData);

//Returns a random integer between low and high inclusively
int randomInteger(int low, int high);

//Runs the loop of all six tasks, does not run the task if the task pointer is null
void scheduleTask();

//Prints a string to the tft given text, the length of the text, a color, and a line number
void print(char str[], int length, int color, int line);

//Starts up the system by creating all the objects that are needed to run the system
void setupSystem();

//Process earth input
void processesEarthInput(char in);

//Prints timing information for a function based on its last runtime
void printTaskTiming(char taskName[], unsigned long lastRunTime);

//Returns the current system time in milliseconds
unsigned long systemTime();

unsigned short unsignedShortMax(long a, long b) {
#if !ARDUINO_ON
    return (unsigned short) fmax(a, b);
#else
    return max(a, b);
#endif
}

unsigned short unsignedShortMin(long a, long b) {
#if !ARDUINO_ON
    return (unsigned short) fmin(a, b);
#else
    return min(a, b);
#endif
}

#if ARDUINO_ON

//Arduino setup function
void setup(void) {
    Serial.begin(9600); //Sets baud rate to 9600
    Serial1.begin(9600);

    pinMode(PWM_OUTPUT_PIN, OUTPUT); //SETUP PWM OUTPUT SIGNAL

    Serial.println(F("TFT LCD test")); //Prints to serial monitor

    //determines if shield or board
#ifdef USE_Elegoo_SHIELD_PINOUT
    Serial.println(F("Using Elegoo 2.4\" TFT Arduino Shield Pinout"));
#else
    Serial.println(F("Using Elegoo 2.4\" TFT Breakout Board Pinout"));
#endif

    //prints out tft size
    Serial.print("TFT size is ");
    Serial.print(tft.width());
    Serial.print("x");
    Serial.println(tft.height());

    tft.reset();
    tft.setTextSize(2);
    //prints out the current LCD driver version
    uint16_t identifier = tft.readID();
    if (identifier == 0x9325) {
        Serial.println(F("Found ILI9325 LCD driver"));
    } else if (identifier == 0x9328) {
        Serial.println(F("Found ILI9328 LCD driver"));
    } else if (identifier == 0x4535) {
        Serial.println(F("Found LGDP4535 LCD driver"));
    } else if (identifier == 0x7575) {
        Serial.println(F("Found HX8347G LCD driver"));
    } else if (identifier == 0x9341) {
        Serial.println(F("Found ILI9341 LCD driver"));
    } else if (identifier == 0x8357) {
        Serial.println(F("Found HX8357D LCD driver"));
    } else if (identifier == 0x0101) {
        identifier = 0x9341;
        Serial.println(F("Found 0x9341 LCD driver"));
    } else if (identifier == 0x1111) {
        identifier = 0x9328;
        Serial.println(F("Found 0x9328 LCD driver"));
    } else { //prints to serial monitor if wiring is bad or unknown LCD driver
        Serial.print(F("Unknown LCD driver chip: "));
        Serial.println(identifier, HEX);
        Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
        Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
        Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
        Serial.println(F("If using the breakout board, it should NOT be #defined!"));
        Serial.println(F("Also if using the breakout, double-check that all wiring"));
        Serial.println(F("matches the tutorial."));
        identifier = 0x9328;

    }
    tft.begin(identifier);
    tft.fillScreen(NONE);

}

//Arduino loop
void loop(void) {
    setupSystem();
}
#else

int main() {
    setupSystem();
}

#endif

//inserts a node at the end of the list
//code taken from class website: https://class.ece.uw.edu/474/peckol/assignments/lab3/project3Aut18.pdf
void insertNode(TCB *node) {
    if (NULL == head) {
        head = node;
        tail = node;
        head->next = node;
        head->prev = node;
    } else {
        TCB *cur = head;
        Bool set = FALSE;
        int newPriority = node->priority;
        //Do specific check for head
        if (head->priority > newPriority) {
            node->next = cur;
            node->prev = cur->prev;
            node->prev->next = node;
            node->next->prev = node;
            head = node;
            set = TRUE;
        }
        cur = head->next;
        while (cur != head && !set) {
            if (cur->priority > newPriority) {
                //Found place to insert

                set = TRUE;
                node->next = cur;
                node->prev = cur->prev;
                node->prev->next = node;
                node->next->prev = node;
                if(cur == tail) {
                    tail = node;
                }
                break;
            } else {
                cur = cur->next;
            }
        }
    }
}

//removes a node from the list
void removeNode(TCB *node) {
    if (node == head && node == tail) {
        head = NULL;
        tail = NULL;
    } else if (node == head) {
        node->next->prev = tail;        //chop off head
        head = node->next;            //reassign head
    } else if (node == tail) {
        node->prev->next = head;        //chop off tail
        tail = node->prev;            //reassign tail
    } else {
        node->next->prev = node->prev;    //clip next
        node->prev->next = node->next;    //clip prev
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Starts up the system by creating all the objects that are needed to run the system
void setupSystem() {
    /*
    * Init the various tasks
    */

    SolarPanelControlData solarPanelControlData;

    solarPanelControlData.solarPanelState = &SolarPanelState;
    solarPanelControlData.solarPanelDeploy = &SolarPanelDeploy;
    solarPanelControlData.solarPanelRetract = &SolarPanelRetract;
    solarPanelControlData.driveMotorSpeedInc = &DriveMotorSpeedInc;
    solarPanelControlData.driveMotorSpeedDec = &DriveMotorSpeedDec;
    solarPanelControlData.driveMotorSpeed = &DriveMotorSpeed;

    solarPanelControlTCB.taskDataPtr = (void *) &solarPanelControlData;
    solarPanelControlTCB.task = &solarPanelControlTask;
    solarPanelControlTCB.priority = 5;


    //Power Subsystem
    PowerSubsystemData powerSubsystemData;
    powerSubsystemData.solarPanelState = &SolarPanelState;
    powerSubsystemData.batteryLevel = &BatteryLevel;
    powerSubsystemData.batteryTemp = &BatteryTemp;
    powerSubsystemData.powerConsumption = &PowerConsumption;
    powerSubsystemData.powerGeneration = &PowerGeneration;
    powerSubsystemData.solarPanelDeploy = &SolarPanelDeploy;
    powerSubsystemData.solarPanelRetract = &SolarPanelRetract;
    powerSubsystemData.batteryLevelArrayIndex = &BatteryLevelIndex;
    powerSubsystemData.batteryTempIndex = &BatteryTempIndex;
    powerSubsystemData.batteryRapidTemp = &BatteryRapidTemp;
    powerSubsystemData.batteryOverTemp = &BatteryOverTemp;
    powerSubsystemData.acknowledgeOverTemp = &AcknowledgeOverTemp;

    powerSubsystemTCB.taskDataPtr = (void *) &powerSubsystemData;
    powerSubsystemTCB.task = &powerSubsystemTask;
    powerSubsystemTCB.priority = 4;
    powerSubsystemTCB.next = NULL;
    powerSubsystemTCB.prev = NULL;

    insertNode(&powerSubsystemTCB);
    TCB *item = head;

    //Thruster Subsystem
    ThrusterSubsystemData thrusterSubsystemData;
    thrusterSubsystemData.fuelLevel = &FuelLevel;
    thrusterSubsystemData.thrusterControl = &ThrusterControl;

    thrusterSubsystemTCB.taskDataPtr = (void *) &thrusterSubsystemData;
    thrusterSubsystemTCB.task = &thrusterSubsystemTask;
    thrusterSubsystemTCB.next = NULL;
    thrusterSubsystemTCB.prev = NULL;
    thrusterSubsystemTCB.priority = 3;

    insertNode(&thrusterSubsystemTCB);
    item = head;

    //Satellite Comms
    SatelliteComsData satelliteComsData;
    satelliteComsData.thrusterControl = &ThrusterControl;
    satelliteComsData.fuelLevel = &FuelLevel;
    satelliteComsData.powerGeneration = &PowerGeneration;
    satelliteComsData.powerConsumption = &PowerConsumption;
    satelliteComsData.batteryLevel = &BatteryLevel;
    satelliteComsData.solarPanelState = &SolarPanelState;
    satelliteComsData.batteryLow = &BatteryLow;
    satelliteComsData.fuelLow = &FuelLow;

    satelliteComsTCB.taskDataPtr = (void *) &satelliteComsData;
    satelliteComsTCB.task = &satelliteComsTask;
    satelliteComsTCB.next = NULL;
    satelliteComsTCB.prev = NULL;
    satelliteComsTCB.priority = 2;

    insertNode(&satelliteComsTCB);
    item = head;

    //Console Display
    ConsoleDisplayData consoleDisplayData;
    consoleDisplayData.fuelLow = &FuelLow;
    consoleDisplayData.batteryLow = &BatteryLow;
    consoleDisplayData.solarPanelState = &SolarPanelState;
    consoleDisplayData.batteryLevel = &BatteryLevel;
    consoleDisplayData.fuelLevel = &FuelLevel;
    consoleDisplayData.powerConsumption = &PowerConsumption;
    consoleDisplayData.powerGeneration = &PowerGeneration;
    consoleDisplayData.batteryTemp = &BatteryTemp;
    consoleDisplayData.batteryRapidTemp = &BatteryRapidTemp;

    consoleDisplayTCB.taskDataPtr = (void *) &consoleDisplayData;
    consoleDisplayTCB.task = &consoleDisplayTask;
    consoleDisplayTCB.next = NULL;
    consoleDisplayTCB.prev = NULL;
    consoleDisplayTCB.priority = 1;

    insertNode(&consoleDisplayTCB);
    item = head;

    //Warning Alarm
    WarningAlarmData warningAlarmData;
    warningAlarmData.batteryLevel = &BatteryLevel;
    warningAlarmData.batteryLow = &BatteryLow;
    warningAlarmData.fuelLow = &FuelLow;
    warningAlarmData.fuelLevel = &FuelLevel;
    warningAlarmData.batteryTemp = &BatteryTemp;
    warningAlarmData.batteryRapidTemp = &BatteryRapidTemp;

    warningAlarmTCB.taskDataPtr = (void *) &warningAlarmData;
    warningAlarmTCB.task = &warningAlarmTask;
    warningAlarmTCB.next = NULL;
    warningAlarmTCB.prev = NULL;
    warningAlarmTCB.priority = 1;

    insertNode(&warningAlarmTCB);
    item = head;


    ConsoleKeypadData consoleKeypadData;
    consoleKeypadData.driveMotorSpeedDec = &DriveMotorSpeedDec;
    consoleKeypadData.driveMotorSpeedInc = &DriveMotorSpeedInc;

    consoleKeypadTCB.taskDataPtr = (void *) &consoleKeypadData;
    consoleKeypadTCB.task = &consoleKeypadTask;
    consoleKeypadTCB.next = NULL;
    consoleKeypadTCB.prev = NULL;
    consoleKeypadTCB.priority = 1;

    //Vehicle Taks
    VehicleCommsData vehicleCommsData;
    vehicleCommsData.command = &Command;
    vehicleCommsData.responce = &Response;
    vehicalComsTCB.taskDataPtr = (void *) &vehicleCommsData;
    vehicalComsTCB.task = &vehicleCommsTask;
    vehicalComsTCB.next = NULL;
    vehicalComsTCB.prev = NULL;
    vehicalComsTCB.priority = 1;

    insertNode(&vehicalComsTCB);
    item = head;


    //Starts the schedule looping
    scheduleTask();
}

//Runs the loop of all six tasks, does not run the task if the task pointer is null
void scheduleTask() {
    TCB *cur = head;
    while (1) { //Loop forever
        if (NULL != head) {
            cur->task(cur->taskDataPtr);
            TCB *next = cur->prev->next;
            if (next != cur) {
                cur = next; //This happens when a task removes itself from the list
            } else {
                cur = cur->next; //This happens with the natural progression of the queue
            }
#if ARDUINO_ON
            while (Serial.available() > 0) {
                char input = Serial.read();
                processesEarthInput(input);
            }
            bool didPrint = false;
            while (Serial1.available() > 0) {
                didPrint = true;
                Serial.print(Serial1.read());
            }
            if (didPrint) {
                Serial.println();
            }
#endif
        }
    }
}

void processesEarthInput(char in) {
    if (INC_CHAR == in) {
        LAST_SOLAR_PANEL_CONTROL_CHAR = in;
    } else if (DEC_CHAR == in) {
        LAST_SOLAR_PANEL_CONTROL_CHAR = in;
    } else if (NO_SOLAR_PANEL_INPUT == in) {
        LAST_SOLAR_PANEL_CONTROL_CHAR = in;
    } else if (FORWARD_CHAR == in) {
        LAST_VEHICLE_COMM = in;
    } else if (BACK_CHAR == in) {
        LAST_VEHICLE_COMM = in;
    } else if (LEFT_CHAR == in) {
        LAST_VEHICLE_COMM = in;
    } else if (RIGHT_CHAR == in) {
        LAST_VEHICLE_COMM = in;
    } else if (DRILL_START_CHAR == in) {
        LAST_VEHICLE_COMM = in;
    } else if (DRILL_STOP_CHAR == in) {
        LAST_VEHICLE_COMM = in;
    }
}

//Controls the execution of the power subsystem
void powerSubsystemTask(void *powerSubsystemData) {
    //Count of the number times this function is called.
    // It is okay if this number wraps to 0 because we just care about if the function call is odd or even
    PowerSubsystemData *data = (PowerSubsystemData *) powerSubsystemData;
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    static unsigned long readBatteryLevelExecutionTime = 0;
    static unsigned long readBatteryTempExecutionTime = 0;
    if (0 == nextExecutionTime) {
        readBatteryLevelExecutionTime = systemTime() + BatteryDelay;
        readBatteryTempExecutionTime = systemTime() + BatteryTempDelay;
    }
    static Bool batteryInitialRead = TRUE;
    static Bool batteryInitialTempRead = TRUE;
    unsigned long t = systemTime();
    if (0 == nextExecutionTime || t > nextExecutionTime) {
#if !ARDUINO_ON && DEBUG
        printf("powerSubsystemTask\n");
#endif
        printTaskTiming("powerSubsystemTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        static unsigned int executionCount = 0;
        //powerConsumption
        static Bool consumptionIncreasing = TRUE;
        if (consumptionIncreasing) {
            if (executionCount % 2 == 0) {
                *(data->powerConsumption) += 2;
            } else {
                *(data->powerConsumption) -= 1;
            }
            if (*(data->powerConsumption) > 10) {
                consumptionIncreasing = FALSE;
            }
        } else {
            if (executionCount % 2 == 0) {
                *(data->powerConsumption) -= 2;
            } else {
                *(data->powerConsumption) += 1;
            }
            if (*(data->powerConsumption) < 5) {
                consumptionIncreasing = TRUE;
            }
        }

        //powerGeneration
        if (*data->solarPanelState) {
            if (*data->batteryLevel > BATTERY_95) {
                if (*data->solarPanelRetract) {  //If we have not already signalled to retract
                    insertNode(&solarPanelControlTCB); //Start the task
                }
                *data->solarPanelDeploy = FALSE;
                *data->solarPanelRetract = TRUE;
                *data->powerGeneration = 0;
            } else if (*data->batteryLevel < BATTERY_50) {
                //Increment the variable by 2 every even numbered time
                if (executionCount % 2 == 0) {
                    (*data->powerGeneration) += 2;
                } else { //Increment the variable by 1 every odd numbered time
                    (*data->powerGeneration) += 1;
                }
            } else {
                //Increment the variable by 2 every even numbered time
                if (executionCount % 2 == 0) {
                    (*data->powerGeneration) += 2;
                }
            }
        } else {
            if (*data->batteryLevel <= BATTERY_10) {
                if (!*data->solarPanelDeploy) { //If we have not already signalled to deploy
                    insertNode(&solarPanelControlTCB); //Add the task
                }
                *data->solarPanelDeploy = TRUE;
                *data->solarPanelRetract = FALSE;
            }
        }

        //Waits 600us before updating new level to ensure a valid reading
        //Converts analogRead from 50 to 332 for a 1.5V Battery to represent 0 to 36V
        //50 is an offset of the values, and 0.1285714 is the conversion from 280 values to 36V
        //Can easily be changed to take measurements every 600 us by adding another boolean

        if (!batteryInitialRead) {
            batteryRead(data);
        }

        //Waits 500us before updating new level to ensure a valid reading
        if (systemTime() >= readBatteryTempExecutionTime) {
            batteryTempRead(data);
        }
        readBatteryTempExecutionTime = systemTime() + BatteryTempDelay;
        nextExecutionTime = systemTime() + runDelay;
        executionCount++;
        
    }
   // if (batteryInitialTempRead && systemTime() >= readBatteryTempExecutionTime) {
   //     batteryTempRead(data);
   //     batteryInitialRead = FALSE;
   // }
    if (batteryInitialRead && systemTime() >= readBatteryLevelExecutionTime) {
        batteryRead(data);
        batteryInitialRead = FALSE;
    }
}

void batteryTempRead(PowerSubsystemData *data) {
#if ARDUINO_ON
    unsigned short newVal = (((analogRead(BatteryTempPin) - 50)*0.1286) / 3.25); // (*(data->powerConsumption)) + (*(data->powerGeneration))
#else
    unsigned short newVal = 0;
#endif
    if (newVal < 0) {
        newVal = 0;
    }

    int celsius = newVal * 32 + 33;
    *data->batteryTemp = celsius; //Converts to celsius
    //Saves the last recordered temp, enables warning if previous recorded value has a 20% diff
                      //forced delay to visually see the changes in rapid temp warning
    BatteryTempArray[*data->batteryTempIndex] = celsius;
    if ((0 != (*data->batteryTempIndex)) && (BatteryTempArray[*data->batteryTempIndex-1] > 0) && ((celsius / BatteryTempArray[(*data->batteryTempIndex - 1) % 16] < .8) ||
                                           (celsius / BatteryTempArray[(*data->batteryTempIndex - 1) % 16] > 1.2))) {
        *data->batteryRapidTemp = TRUE;
    } else {
        *data->batteryRapidTemp = FALSE;
    }

    if ((BatteryTempArray[*data->batteryTempIndex] > 180) && (BatteryTempArray[*data->batteryTempIndex] != BatteryTempArray[*data->batteryTempIndex-1])) {
        *data->batteryOverTemp = TRUE;
        *data->acknowledgeOverTemp = FALSE;
        //*data->initialAlarm = TRUE;
    } else if (*data->acknowledgeOverTemp) {
        *data->batteryOverTemp = FALSE;
        *data->acknowledgeOverTemp = FALSE;
    }

#if ARDUINO_ON && DEBUG
    Serial.print("Battery Temp: ");
    Serial.print(celsius);
    Serial.println();
#elif DEBUG
    printf("Battery Temp: %d\n", celsius);
#endif
}

void batteryRead(PowerSubsystemData *data) {
#if ARDUINO_ON
    unsigned short newVal = ((analogRead(BatteryPin) - 50)*0.1286); //- (*(data->powerConsumption)) + (*(data->powerGeneration))
#else
    unsigned short newVal = 0;
#endif
    if (newVal < 0) {
        newVal = 0;
    } else if (newVal > 36) {
        newVal = 36;
    }
    //BatteryLevelArray[*data->batteryLevelArrayIndex] = newVal;
    //*data->batteryLevelArrayIndex = (unsigned short) ((*data->batteryLevelArrayIndex + 1) % 16);
    *data->batteryLevel = newVal;
#if ARDUINO_ON && DEBUG
    Serial.print("Battery Level: ");
    Serial.print(newVal);
    Serial.println();
#elif DEBUG
    printf("Battery Level: %d\n", newVal);
#endif
}

//Controls the execution of the thruster subsystem
void thrusterSubsystemTask(void *thrusterSubsystemData) {
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
#if !ARDUINO_ON && DEBUG
        printf("thrusterSubsystemTask\n");
#endif
        printTaskTiming("thrusterSubsystemTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        ThrusterSubsystemData *data = (ThrusterSubsystemData *) thrusterSubsystemData;
        unsigned short left = 0, right = 0, up = 0, down = 0;

        unsigned int signal = *(data->thrusterControl);

        unsigned int direction = signal & (0xF); // Get the last 4 bits
        unsigned int magnitude = (signal & (0xF0)) >> 4; // Get the 5-7th bit and shift if back down
        unsigned int duration = (signal & (0xFF00)) >> 8;

        //Debug print info
#if !ARDUINO_ON && DEBUG
        printf("\t\tDirection %d\n", direction);
        printf("\t\tMagnitude %d\n", magnitude);
        printf("\t\tDuration %d\n", duration);
        printf("thrusterSubsystemTask\n");
#endif


        //Adjust fuel level based on command
        if (*data->fuelLevel > 0 && (int) *data->fuelLevel >= ((int) *data->fuelLevel - 4 * duration / 100)) {
            *data->fuelLevel = unsignedShortMax(0, *data->fuelLevel -
                                                   4 * duration /
                                                   100); //magnitude at this point is full on and full off
        } else {
            *data->fuelLevel = 0;
        }


        nextExecutionTime = systemTime() + runDelay;
    }

}

//Generates a random signal for the thruster based on the assignment specs
//Choose a random direction, magnitude, and duration and shifts the bits to fit that information into 16 bits
unsigned int getRandomThrustSignal() {
    unsigned int signal = 1;
    unsigned short direction = (unsigned short) randomInteger(0, 4);
    if (direction == 4) //No thrust
        return 0;
    signal = signal << direction;
    unsigned int magnitude = randomInteger(0, 15);
    unsigned int duration = randomInteger(0, 255);

    signal = signal | (magnitude << 4);
    signal = signal | (duration << 8);
    return signal;
}

//Controls the execution of the satellite coms subsystem
void satelliteComsTask(void *satelliteComsData) {
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
#if !ARDUINO_ON && DEBUG
        printf("satelliteComsTask\n");
#endif
        printTaskTiming("satelliteComsTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        SatelliteComsData *data = (SatelliteComsData *) satelliteComsData;

        //TODO: In future labs, send the following data:
        /*
        * Fuel Low
        * Battery Low
        * Solar Panel State
        * Battery Level
        * Fuel Level
        * Power Consumption
        * Power Generation
        */

        *(data->thrusterControl) = getRandomThrustSignal();
        nextExecutionTime = systemTime() + runDelay;
    }
}

//Controls the execution of the console display subsystem
void consoleDisplayTask(void *consoleDisplayData) {
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
#if !ARDUINO_ON && DEBUG
        printf("consoleDisplayTask\n");
#endif
        printTaskTiming("consoleDisplayTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        ConsoleDisplayData *data = (ConsoleDisplayData *) consoleDisplayData;
        Bool inStatusMode = TRUE; //TODO get this from some external input
        //printf("consoleDisplayTask\n");
        if (inStatusMode) {
            //Print
            //Solar Panel State
            //Battery Level
            //Fuel Level
            //Power Consumption
#if ARDUINO_ON
            Serial.print("\tSolar Panel State: ");
            Serial.println((*data->solarPanelState ? " ON" : "OFF"));
            Serial.print("\tBattery Level: ");
            Serial.println(*data->batteryLevel);
            Serial.print("\tBattery Temp: ");
            Serial.println(*data->batteryTemp);
            Serial.print("\tBattery Rapid Temp: ");
            Serial.println(*data->batteryRapidTemp ? " TRUE" : "FALSE");
            Serial.print("\tFuel Level: ");
            Serial.println(*data->fuelLevel);
            Serial.print("\tPower Consumption: ");
            Serial.println(*data->powerConsumption);
            Serial.print("\tPower Generation: ");
            Serial.println(*data->powerGeneration);
#elif 0
            printf("\tSolar Panel State: ");
            printf((*data->solarPanelState ? " ON" : "OFF"));
            printf("\tBattery Level: ");
            printf("%d", *data->batteryLevel);
            printf("\tBattery Temp: ");
            printf("%d", *data->batteryTemp);
            printf("\tFuel Level: ");
            printf("%d", *data->fuelLevel);
            printf("\tPower Consumption: ");
            printf("%d", *data->powerConsumption);
            printf("\tPower Generation: ");
            printf("%d\n", *data->powerGeneration);
#endif

        } else {
#if ARDUINO_ON
            if (*data->fuelLow == TRUE) {
                Serial.println("Fuel Low!");
            }
            if (*data->batteryLow == TRUE) {
                Serial.println("Battery Low!");
            }
        }

        Serial.println();
#else
        }
#endif
        nextExecutionTime = systemTime() + runDelay;
    }
}

//Controls the execution of the warning alarm subsystem
void warningAlarmTask(void *warningAlarmData) {
    WarningAlarmData *data = (WarningAlarmData *) warningAlarmData;
    static int fuelStatus = NONE;
    static int batteryStatus = NONE;
    static unsigned long hideFuelTime = 0;
    static unsigned long showFuelTime = 0;
    static unsigned long hideBatteryTime = 0;
    static unsigned long showBatteryTime = 0;

    *data->fuelLow = *data->fuelLevel <= FUEL_10 ? TRUE : FALSE;
    *data->batteryLow = *data->batteryLow <= BATTERY_10 ? TRUE : FALSE;

    unsigned long fuelDelay = (*data->fuelLevel <= FUEL_10) ? LongTimeDelay : ShortTimeDelay;
    int fuelColor = (*data->fuelLevel <= FUEL_10) ? RED : ORANGE;
    char fuelLevelString[3];
    unsigned short fuelLevel = 0;
    fuelLevel = *data->fuelLevel;
    sprintf(fuelLevelString, "%d", fuelLevel);
    char printedFuel[9] = "FUEL :";
    strcat(printedFuel, fuelLevelString);

    if (*data->fuelLevel <= FUEL_50) {
        if (fuelStatus == fuelColor) {
            if (showFuelTime == 0) { //If showing fuel status
                if (hideFuelTime < systemTime()) {
                    showFuelTime = systemTime() + fuelDelay;
                    hideFuelTime = 0;
                    print(printedFuel, 9, NONE, 0);
                }
            } else { //If hiding fuel status
                if (showFuelTime < systemTime()) {
                    hideFuelTime = systemTime() + fuelDelay;
                    showFuelTime = 0;
                    print(printedFuel, 9, fuelColor, 0);
                }
            }
        } else {
            fuelStatus = fuelColor;
            print(printedFuel, 9, fuelColor, 0);
            hideFuelTime = systemTime() + fuelDelay;
        }
    } else if (fuelStatus != GREEN) {
        print(printedFuel, 9, GREEN, 0);
        fuelStatus = GREEN;
    }

    //DISPLAY BATTERY LEVEL 0-36V Changing Color depending on level
    unsigned long batteryDelay = (*data->batteryLevel <= BATTERY_10) ? ShortTimeDelay : LongTimeDelay;
    int batteryColor = (*data->batteryLevel <= BATTERY_10) ? RED : ORANGE;
    char batLevelString[3];
    unsigned short batLevel = *data->batteryLevel;
    sprintf(batLevelString, "%d", batLevel);
    char printedBat[12] = "BATTERY: ";
    strcat(printedBat, batLevelString);
    if (*data->batteryLevel <= BATTERY_50) {
        if (batteryStatus == batteryColor) {
            if (showBatteryTime == 0) { //If showing battery status
                if (hideBatteryTime < systemTime()) {
                    showBatteryTime = systemTime() + batteryDelay;
                    hideBatteryTime = 0;
                    print(printedBat, 12, NONE, 1);
                }
            } else { //If hiding battery status
                if (showBatteryTime < systemTime()) {
                    hideBatteryTime = systemTime() + batteryDelay;
                    showBatteryTime = 0;
                    print(printedBat, 12, batteryColor, 1);
                }
            }
        } else {
            batteryStatus = batteryColor;
            print(printedBat, 12, batteryColor, 1);
            hideBatteryTime = systemTime() + batteryDelay;
        }
    } else if (batteryStatus != GREEN) {
        print(printedBat, 12, GREEN, 1);
        batteryStatus = GREEN;
    }

    //DISPLAYS BATTERY TEMP
    char batTempString[3];
    unsigned short batTemp = *data->batteryTemp;
    sprintf(batTempString, "%d", batTemp);
    char printedBatTemp[18] = "BATTERY TEMP: ";
    strcat(printedBatTemp, batTempString);
    print(printedBatTemp, 18, GREEN, 2);

    if (*data->batteryRapidTemp) {
        print("Battery Rapid Temp", 18, RED, 3);
    } else {
        print("Battery Rapid Temp", 18, NONE, 3);
    }

    /* UNFINISHED WARNING ALARM FOR OVERTEMP

    if (*data->batteryOverTemp && !acknowledgeTemp) {
        if (initialAlarm) {
            
        } else { //Cycle Pattern for Past 15 seconds

        } 
    } else {
        print("TEMPERATURE", 11, NONE, 4);
    }

    */
}

//Controls the execution of the solar panel control subsystem
void solarPanelControlTask(void *solarPanelControlData) {
    SolarPanelControlData *data = (SolarPanelControlData *) solarPanelControlData;

    static Bool launchedKeypadTask = FALSE;
    static Bool isPWMOn = FALSE;
    static long nextPWMRunTime = 0;
    static long nextCounterRunTime = 0;
    static float dutyCycle = DEFAULT_DUTY_CYCLE;
    static float elapsedTime = 0;
    Bool outputPWM = FALSE;

    if(*data->driveMotorSpeedInc) {
#if ARDUINO_ON
        dutyCycle = min(0.9f, dutyCycle + 0.05f);
#endif
        *data->driveMotorSpeedInc = FALSE;
    }
    if(*data->driveMotorSpeedDec) {
#if ARDUINO_ON
        dutyCycle = max(0.1f, dutyCycle - 0.05f);
#endif
        *data->driveMotorSpeedDec = FALSE;
    }
    *data->driveMotorSpeed = (unsigned short) (dutyCycle * 10.0f); //Update this so we can display it somewhere

    if(!launchedKeypadTask) {
        launchedKeypadTask = TRUE;
        insertNode(&consoleKeypadTCB);
    }

    if(nextPWMRunTime == 0 || systemTime() >= nextPWMRunTime) {
        if(isPWMOn) {
            nextPWMRunTime = systemTime() + PWM_PERIOD * (1.0f - dutyCycle);
            outputPWM = FALSE;
        } else {
            nextPWMRunTime = systemTime() + PWM_PERIOD * (dutyCycle);
            outputPWM = TRUE;
        }
        isPWMOn = !isPWMOn;
#if ARDUINO_ON
        digitalWrite(PWM_OUTPUT_PIN, outputPWM ? HIGH : LOW);
        Serial.println(outputPWM ? "High" : "Low");
#endif
    }

    if(nextCounterRunTime == 0 || systemTime() >= nextCounterRunTime) {
        nextCounterRunTime = systemTime() + PWM_COUNTER_PERIOD;
        elapsedTime += PWM_COUNTER_PERIOD * (DEFAULT_DUTY_CYCLE + dutyCycle);
        if(elapsedTime >= DEPLOY_RETRACT_TIME) {
            //Interupt
#if ARDUINO_ON
            digitalWrite(PWM_OUTPUT_PIN, LOW);
#endif

            //Reset values
            launchedKeypadTask = FALSE;
            isPWMOn = FALSE;
            nextPWMRunTime = 0;
            nextCounterRunTime = 0;
            dutyCycle = DEFAULT_DUTY_CYCLE;
            elapsedTime = 0;

            //Send stop
            removeNode(&solarPanelControlTCB);
            removeNode(&consoleKeypadTCB);

            //Report the new state of the solar panels
            *data->solarPanelState = *data->solarPanelDeploy;
        }
    }
}

//Controls the execution of the Keypad task
void consoleKeypadTask(void *consoleKeypadData) {
    ConsoleKeypadData *data = (ConsoleKeypadData *) consoleKeypadData;
#if ARDUINO_ON
    char signal = LAST_SOLAR_PANEL_CONTROL_CHAR;
    if (signal != NO_SOLAR_PANEL_INPUT) {
        if (INC_CHAR == signal) {
            *data->driveMotorSpeedInc = TRUE;
            *data->driveMotorSpeedDec = FALSE;
        } else if (DEC_CHAR == signal) {
            *data->driveMotorSpeedInc = FALSE;
            *data->driveMotorSpeedDec = TRUE;
        }
        LAST_SOLAR_PANEL_CONTROL_CHAR = NO_SOLAR_PANEL_INPUT;
    }
#endif
}

//Controls the execution of the VehicleComms task
void vehicleCommsTask(void *vehicleCommsData) {
    VehicleCommsData *data = (VehicleCommsData *) vehicleCommsData;
    static char lastProcessedInput = NO_VEHICLE_COMMAND;
#if ARDUINO_ON
    if (lastProcessedInput != LAST_VEHICLE_COMM) {
        Serial1.print(LAST_VEHICLE_COMM);

        lastProcessedInput = LAST_VEHICLE_COMM;
    }
#endif

}

//Controls the execution of the ImageCapture task
void imageCaptureTask(void *imageCaptureData) {

}

//Returns a random integer between low and high inclusively
//Code taken from class website: https://class.ece.uw.edu/474/peckol/assignments/lab2/rand1.c
int randomInteger(int low, int high) {
    double randNum = 1.0;
    int multiplier = 2743;
    int addOn = 5923;
    double max = INT_MAX + 1.0;

    int retVal = 0;

    if (low > high)
        retVal = randomInteger(high, low);
    else {
        retVal = rand() % high + low;
    }

    return retVal;
}

//Prints a string to the tft given text, the length of the text, a color, and a line number
void print(char str[], int length, int color, int line) {
    //To flash the selected line, you must print exact same string black then recolor
#if ARDUINO_ON
    for (int i = 0; i < length; i++) {
        tft.setTextColor(color, NONE);
        tft.setCursor(i * 12, line * 16);
        tft.print(str[i]);
    }
#else
    printf("%s", str);
    printf(" color: %d - line: %d\n", color, line);
#endif
}

//Starts up the system by creating all the objects that are needed to run the system
void printTaskTiming(char taskName[], unsigned long lastRunTime) {
#if ARDUINO_ON
    if (shouldPrintTaskTiming) {
        Serial.print(taskName);
        Serial.print(" - cycle delay: ");
        if (lastRunTime > 0) {
            Serial.println((double)(systemTime() - lastRunTime) / 1000000.0, 4);
        } else {
            Serial.println(0.0, 4);
        }
    }
#endif
}

//Returns the current system time in milliseconds
unsigned long systemTime() {
#if ARDUINO_ON
    return micros();
#else
    return (unsigned long) time(NULL) * 1000000;
#endif
}