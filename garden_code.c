
//sensors
int BACK_LIGHT_LIGHT = 0;
int FRONT_LIGHT = 0;
int RIGHT_LIGHT = 0;
int LEFT_LIGHT = 0;
int MOISTURE = A0;
int MOISTURE_KNOB = 0;
int TOGGLE = 0;
//actuators
int MOTOR = 0;
int PUMP = 13;
//settings
static const int iterdelay = 20;
static const int TURN_RIGHT = 1;
static const int TURN_LEFT  = 1;
static const int DONT_TURN  = 0;
//for a filter on the light values
static const float alpha = 0.3;

//water states
typedef enum {
  DETECT_MOISTURE,
  WATERON,
  WATEROFF
} WATER;
typedef enum {
  CALIBRATE,
  IDLE,
  TURN_LEFT,
  TURN_RIGHT,
  SWIVEL_OFF
} SWIVEL;
//#################
//static variables
//#################
//states
static WATER waterstate = DETECT_MOISTURE;
static int duty_cycle = 0;
static int count = 0;
//swiveling state variables
static SWIVEL swivelstate = IDLE;
static int angle = 0;
//calibrated values for sensors? since they're all different, may have different values...
//let calibrated be the minimum light per sensor after turning 360
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
  //start by detecting moisture and calibrating swiveling/light sensors
  waterstate = DETECT_MOISTURE;
  swivelstate = CALIBRATE;
  count = 0;
  duty_cycle = 0;
  //measurements from each of four light sensors and starting ‘angle’ is zero
  running_front = analogRead(FRONT_LIGHT);
  running_back  = analogRead(BACK_LIGHT);
  running_left  = analogRead(LEFT_LIGHT);
  running_right = analogRead(RIGHT_LIGHT);
  angle = 0;
 //toggle is an input; motor and pump are our outputs
  pinMode(TOGGLE, INPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(PUMP, OUTPUT);
 
}

//pass in 1 for ON, 0 for turning pump off
void pump(int on_off){
  if (on_off) {
	digitalWrite(PUMP, HIGH);
  } else {
	digitalWrite(PUMP, LOW);
  }
}

//pass in number of degrees to turn box from 0 to 360
//for now, 0 = turn, 1 = don't
//will need to change later
void motor(int degrees){
 //todo
 if ()degrees == 0) {
   digitalWrite(MOTOR, HIGH);
 } else {
   digitalWrite(MOTOR, LOW);
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
  int val = analogRead(light_sensor);
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
void toggle_swivel(int on_off) {
  if (on_off) {
    static SWIVEL swivelstate = SWIVEL_OFF;
  }
  else {
    static SWIVEL swivelstate = CALIBRATE;
}

//returns number of degrees box should swivel to maximize light exposure
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
	return moisture_reading - desired > 500 ? 100 : (moisture_reading - desired)/5;
  }
}

//returns desired moisture level based on knob
int desired_moisture_level(){
  val = analogRead(MOISTURE_KNOB);
  //todo: process val
  return val
}

// the loop function runs over and over again forever
void loop() {
  /*
  digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);          	// wait for a second
  digitalWrite(9, LOW);	// turn the LED off by making the voltage LOW
  delay(2000);          	// wait for a second
  */
  delay(iterdelay);
  switch(waterstate) {
  case DETECT_MOISTURE:
  {
	int moisture_reading = read_moisture();
 int desired = desired_moisture_level();
	Serial.println(moist);
 duty_cycle = pump_on_for(moisture_reading, desired);
 //if duty_cycle is 0, never go into watering state
	if (duty_cycle == 0) {
  waterstate = WATEROFF;
 } else {
  waterstate = WATERON;
  pump(1);
 }
	count = 1;
	break;
  }
  case WATERON:
 //DETECT_MOISTURE is 1 iteration, if dutycycle = 100, then should transition on 99 so it's still on for iteration 100
 //if dutycycle = 99, pump should turn off and go to DETECT_MOISTURE so iteration 100 is off
 if (count == 99) {
  waterstate = DETECT_MOISTURE;
  if (duty_cycle == 99) {
   pump(0);
  }
 } else if (count >= duty_cycle) {
  	waterstate = WATEROFF;
  	pump(0);
   count++;
	} else {
  	count++;
	}
	break;
  case WATEROFF:
	if (count >= 99) {
  	waterstate = DETECT_MOISTURE;
	} else {
  	count++;
	}
	break;
  }
 
  //FSM for swiveling
  switch(swivelstate) {
   case CALIBRATE:
	//todo: implement
 	break;
  
   case IDLE:
   {
	//todo: finish
  int front = read_light(FRONT_LIGHT){
  int back  = read_light(BACK_LIGHT);
  int left  = read_light(LEFT_LIGHT);
  int right = read_light(RIGHT_LIGHT);
  if (front < right) {
   
  }
  
  
  break;
   }
   case TURNLEFT:
	//todo: implement
 	break;
   case TURNRIGHT:
	//todo: implement
 	break;
   case SWIVELOFF:
	//todo: implement
 	break;
  }
 
}


