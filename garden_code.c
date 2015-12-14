#include <Stepper.h>

//sensors
const int BACK_LIGHT = A15;
const int FRONT_LIGHT = A13;
const int RIGHT_LIGHT = A14;
const int LEFT_LIGHT = A11;
const int MOISTURE = A0;
const int MOISTURE_KNOB = A3;
const int TOGGLE = 14;
const int MOTOR_RED = 10;
const int MOTOR_YELLOW = 11;
const int MOTOR_GREEN  = 13;
const int MOTOR_GRAY   = 12;
const int TEST_LED     = 8;

//actuators
int PUMP = 2;
int stepsPerRevolution = 200;
Stepper MOTOR(stepsPerRevolution, MOTOR_YELLOW,MOTOR_RED,MOTOR_GRAY,MOTOR_GREEN);

//settings
static const int iterdelay = 15;
static const int TURN_RIGHT = 1;
static const int TURN_LEFT  = 1;
static const int DONT_TURN  = 0;
static const int cycle_total = 60;
const int16_t stepsPerIteration = 10;
const int verifyThreshold = 1000/iterdelay;


//for a filter on the light values
static const float alpha = 0.3;

//water states
typedef enum {
  DETECT_MOISTURE,
  WATERON,
  WATEROFF
} WATER;

//swivel states
typedef enum {
  CALIBRATE,
  IDLE,
  TURN,
  SWIVEL_OFF,
  VERIFY
} SWIVEL;

//#################
//static variables
//#################
//Water state variables
static WATER waterstate;
static int duty_cycle;
static int count;


//Swiveling state variables
static SWIVEL swivelstate;
static int rotationSteps;
static int verifyCount;
static int turnDirection; //-1, 0, or 1

//calibrated values for sensors? since they're all different, may have different values...
//let calibrated be the (maximum - minimum) light per sensor after turning 360
//moisture might not need calibration
static int calibrated_moisture = 0;
static int calibrated_front = 0;
static int calibrated_left = 0;
static int calibrated_right = 0;
static int calibrated_back = 0;

//vals for the running average
static float running_front = 0;
static float running_left = 0;
static float running_right = 0;
static float running_back = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  //initialize states
  waterstate = DETECT_MOISTURE;
  swivelstate = CALIBRATE;

  //intialize state variables
  count = 0;
  duty_cycle = 0;

  //measurements from each of four light sensors and starting ‘angle’ is zero
  running_front = read_light(FRONT_LIGHT);
  running_back  = read_light(BACK_LIGHT);
  running_left  = read_light(LEFT_LIGHT);
  running_right = read_light(RIGHT_LIGHT);
  //stepsPerIteration = 1;
  rotationSteps = 0; //rotationSteps is between 0 and 199 inclusive

  turnDirection = 0;
 //toggle is an input; motor and pump are our outputs
  MOTOR.setSpeed(5);
  pinMode(TOGGLE, INPUT);
//  pinMode(MOTOR, OUTPUT);
  pinMode(TEST_LED, OUTPUT);
  pinMode(PUMP, OUTPUT);
}

//pass in 1 for ON, 0 for turning pump off
void pump(int on_off) {
  if (on_off) {
    digitalWrite(PUMP, HIGH);
  } else {
    digitalWrite(PUMP, LOW);
  }
}



//returns an int that is the moisture reading
int read_moisture(){
  return analogRead(MOISTURE);
}

//returns a filtered value reading from a light sensor
//pass in pin number of light sensor
int read_light(int light_sensor){
  //todo: test
  int val = 1024 - analogRead(light_sensor);
  switch (light_sensor) {
  case FRONT_LIGHT:
    running_front = (alpha * ((float) val)) + ((1 - alpha) * (running_front));
    return (int) running_front;
  case BACK_LIGHT:
    running_back = (alpha * ((float) val)) + ((1 - alpha) * (running_back));
    return (int) running_back;
  case RIGHT_LIGHT:
    running_right = (alpha * ((float) val)) + ((1 - alpha) * (running_right));
    return (int) running_right;
  case LEFT_LIGHT:
    running_left = (alpha * ((float) val)) + ((1 - alpha) * (running_left));
    return (int) running_left;
  default:
    Serial.println("Something is horribly horribly wrong!\n");
    return 20000;
  }
}

//toggle between having the box swivel towards the light or not
//pass in value of toggle; ON is depressed. OFF is unpressed
bool shouldSwivel() {
  if (digitalRead(TOGGLE) == HIGH) {
    return true;
  }
  return false;
}

//returns number of degrees box should swivel to maximize light exposure
//not using?
int get_number_of_degrees_to_swivel(){
  //todo: implement this
  return 0;
}

//returns amount of time(in ms) pump should be on to satisfy moisture threshold
int pump_on_for(int moisture_reading, int desired) {
  int time_to_water;
  //moisture_reading below desired level, is too wet
  if (moisture_reading < desired) {
    return 0;
  //moisture reading above desired level, too dry
  //todo: verify this setup
  } else {
    return moisture_reading - desired > 1000 ? 100 : (moisture_reading - desired)/10;
  }
}

//returns desired moisture level based on knob
int desired_moisture_level(){
  /*
  knob: 100 kOhm
     X
  X     .
  100kohm resistor comes first
  */
  //todo: process val
  int32_t val = analogRead(MOISTURE_KNOB);
  val = (val * 1000) / (1024 - val); //a value between 0 and 500 or so
  
  return (1024 - val); //(500 = high, 0 = low)
}

// the loop function runs over and over again forever
void loop() {
  /*
  digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);                  // wait for a second
  digitalWrite(9, LOW); // turn the LED off by making the voltage LOW
  delay(2000);                  // wait for a second
  */
  delay(iterdelay); //delays the loop
  
  //check the sensor values
  int check_front = read_light(FRONT_LIGHT);
  int check_back  = read_light(BACK_LIGHT);
  int check_left  = read_light(LEFT_LIGHT);
  int check_right = read_light(RIGHT_LIGHT);
  int check_moisture = read_moisture();
  int check_toggle = digitalRead(TOGGLE);
  int check_desired = desired_moisture_level();

  //print check values

  Serial.println("check_front");
  Serial.println(check_front);

  Serial.println("check_back");
  Serial.println(check_back);
  
  Serial.println("check_left");
  Serial.println(check_left);
  
  Serial.println("check_right");
  Serial.println(check_right);



  Serial.println("Check moisture");
  Serial.println(check_moisture);
 
  Serial.println("check_toggle");
  Serial.println(check_toggle);
 
  Serial.println("check_desired");
  Serial.println(check_desired);
  
  Serial.println("duty_cycle");
  Serial.println(duty_cycle);
  
  Serial.println("count");
  Serial.println(count);
 
  Serial.println("verifyCount");
  Serial.println(verifyCount);

  Serial.println("Steps per iteration");
  Serial.println(stepsPerIteration);
  
  
  switch(waterstate) {
  case DETECT_MOISTURE:
    {
      Serial.println("DETECT_MOISTURE");
      int moisture_reading = read_moisture();
      int desired = desired_moisture_level();
      
      duty_cycle = (int)(((float) pump_on_for(moisture_reading, desired)) * cycle_total / 100);
      //if duty_cycle is 0, never go into watering state
      if (duty_cycle == 0) {
        waterstate = WATEROFF;
        pump(0);
      } 
      else {
        waterstate = WATERON;
        pump(1);
      }
      count = 1;
      break;
    }
  case WATERON:
    Serial.println("WATERON");
    //DETECT_MOISTURE is 1 iteration, if dutycycle = 100, then should transition on 99 so it's still on for iteration 100
    //if dutycycle = 99, pump should turn off and go to DETECT_MOISTURE so iteration 100 is off
    if (count == cycle_total - 1) { //at 99th time step of machine
      waterstate = DETECT_MOISTURE;
      if (duty_cycle == cycle_total - 1) {
        pump(0);
      }
    } else {
      if (count >= duty_cycle) {
        waterstate = WATEROFF;
        pump(0);
      }
      count++;
    }
    break;
  
  case WATEROFF:
    pump(0);
    Serial.println("WATEROFF");
    if (count >= cycle_total - 1) {
      waterstate = DETECT_MOISTURE;
    }
    else {
      count++;
    }
    break;
  }
 
  //FSM for swiveling
  if (!shouldSwivel()) {
    swivelstate = SWIVEL_OFF;
    digitalWrite(TEST_LED, LOW);
  } else {
    digitalWrite(TEST_LED, HIGH);
  }
  
  switch(swivelstate) {
  case CALIBRATE:
    //todo: implement later if needed
    Serial.println("CALIBRATE");
    swivelstate = IDLE;
    break;
 
  case IDLE:
    {
      Serial.println("IDLE");
      //todo: finish
      int front = read_light(FRONT_LIGHT);
      int back  = read_light(BACK_LIGHT);
      int left  = read_light(LEFT_LIGHT);
      int right = read_light(RIGHT_LIGHT);
      if (front < right || front < back || front < left) {
        swivelstate = VERIFY;
        verifyCount = 0;
      }
      break;
    }
  case VERIFY:
  {
    Serial.println("VERIFY");
    int front = read_light(FRONT_LIGHT);
    int back  = read_light(BACK_LIGHT);
    int left  = read_light(LEFT_LIGHT);
    int right = read_light(RIGHT_LIGHT);
    if (front < right || front < back || front < left) {
      verifyCount++;
    } else {
      verifyCount--;
    }
    if (verifyCount > verifyThreshold) {
      swivelstate = TURN;
    } else if (verifyCount <= 0) {
      swivelstate = IDLE;
    }
    break;
  }
  case TURN:
    Serial.println("TURN");
    //todo: implement
    {
    int front = read_light(FRONT_LIGHT);
    int back  = read_light(BACK_LIGHT);
    int left  = read_light(LEFT_LIGHT);
    int right = read_light(RIGHT_LIGHT);
      
    //front is brightest
    if (front > right && front > back && front > left) {
      swivelstate = IDLE;
      turnDirection = 0;
      //front is less than some others
    } else if (right > left) {
      turnDirection = -1;
      Serial.println("Turning right");
    } else {
      turnDirection = 1;
      Serial.println("Turning left");
    }
    MOTOR.step(stepsPerIteration * turnDirection);
    break;
    }
  case SWIVEL_OFF:
    //todo: implement
    Serial.println("SWIVEL_OFF");
    if (shouldSwivel()) {
      swivelstate = CALIBRATE;
    }
    break;
  }

  Serial.println("--------------------------");
}
