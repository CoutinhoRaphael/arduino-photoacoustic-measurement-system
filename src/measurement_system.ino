#include <AccelStepper.h>

/*
 * Ultrasonic Measurement Positioning System
 *
 * This program controls a two-axis positioning system using stepper motors.
 * A laser is moved across predefined positions while a microphone monitors
 * the acoustic response generated during the measurement process.
 *
 * The system performs:
 * - Automatic homing using limit switches
 * - X-axis positioning and scanning
 * - Y-axis row advancement
 * - Laser activation at predefined X-axis positions
 * - Continuous microphone signal acquisition
 * - Basic impact detection using an adjustable threshold
 */

/* ========== MICROPHONE CONFIGURATION ========== */

// Analog input connected to the microphone module
const int micPin = A1;

// Number of samples used to calculate the moving average
const int NUM_SAMPLES = 10;

// Circular buffer used to store the most recent microphone readings
int micReadings[NUM_SAMPLES] = {0};
int sampleIndex = 0;

// Threshold used for basic impact detection
// This value should be adjusted according to the microphone and environment
const int MIC_THRESHOLD = 600;

bool impactoDetectado = false;


/* ========== PIN CONFIGURATION ========== */

// X-axis stepper motor
#define STEP_X 2
#define DIR_X  5

// Y-axis stepper motor
#define STEP_Y 3
#define DIR_Y  6

// Limit switches
// END_F: final position
// END_I: initial position
#define END_F  9
#define END_I  10

// Laser control output
#define LASER  11


/* ========== STEPPER MOTOR CONFIGURATION ========== */

// Stepper motor drivers configured in STEP/DIRECTION mode
AccelStepper stepperX(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepperY(AccelStepper::DRIVER, STEP_Y, DIR_Y);


/* ========== SYSTEM PARAMETERS ========== */

// Number of motor steps between consecutive laser positions
const long LASER_STEP = 725;

// Maximum X-axis position
// The system performs 9 movement intervals along the X-axis
const long X_MAX = LASER_STEP * 9;

// Number of motor steps used to advance one row along the Y-axis
const long Y_STEP = 725;

// Laser activation and cooldown times in milliseconds
const int LASER_ON_TIME = 100;
const int LASER_OFF_TIME = 200;

// Total number of rows in the scanning sequence
const int TOTAL_LINHAS = 12;


/* ========== SCAN CONTROL ========== */

// Current Y-axis row
int linhaAtual = 0;

// Stores the last laser position that was triggered
// This prevents the laser from being activated repeatedly
// while the motor remains within the same position
long ultimoIndiceDisparo = -1;


/* ========== SYSTEM STATES ========== */

// Finite state machine used to control the scanning sequence
//
// HOMING: Returns both axes to the initial position
// X_POS: Moves the X-axis in the positive direction
// Y_POS: Advances one row along the Y-axis
// X_NEG: Moves the X-axis in the negative direction
enum Estado {
  HOMING,
  X_POS,
  Y_POS,
  X_NEG
};

Estado estadoAtual = HOMING;


/* ========== MICROPHONE PROCESSING ========== */

/*
 * Calculates the average value of the most recent microphone readings.
 *
 * A moving average is used to reduce short-term fluctuations in the
 * analog signal and provide a more stable value for threshold detection.
 */
int getMicAverage() {
  long total = 0;

  for (int i = 0; i < NUM_SAMPLES; i++)
    total += micReadings[i];

  return total / NUM_SAMPLES;
}


/* ========== HOMING PROCEDURE ========== */

/*
 * Moves both stepper motors toward the initial limit switch.
 *
 * Once the initial position is reached:
 * - Both motor positions are reset to zero
 * - The row counter is reset
 * - The laser trigger index is cleared
 * - The system transitions to the X-axis scanning state
 */
void homing() {
  stepperX.setSpeed(-800);
  stepperY.setSpeed(-800);

  while (digitalRead(END_I) == HIGH) {
    stepperX.runSpeed();
    stepperY.runSpeed();
  }

  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);

  linhaAtual = 0;
  ultimoIndiceDisparo = -1;

  estadoAtual = X_POS;
}


/* ========== LASER CONTROL ========== */

/*
 * Controls laser activation according to the current X-axis position.
 *
 * The X-axis is divided into predefined positions using LASER_STEP.
 * When the system reaches a valid position, the laser is activated
 * for a defined period and then turned off.
 *
 * The last triggered position is stored to prevent multiple laser
 * activations at the same position.
 */
void controleLaser(long posAtual) {
  long indiceAtual;

  if (stepperX.targetPosition() > stepperX.currentPosition()) {
    // X-axis is moving in the positive direction
    indiceAtual = (posAtual + LASER_STEP - 1) / LASER_STEP;
  } else {
    // X-axis is moving in the negative direction
    indiceAtual = (posAtual + LASER_STEP - 1) / LASER_STEP;
  }

  // Trigger the laser only at positions 1 through 8
  // and only if this position has not already been triggered
  if (indiceAtual != ultimoIndiceDisparo &&
      indiceAtual >= 1 &&
      indiceAtual <= 8) {

    digitalWrite(LASER, HIGH);
    delay(LASER_ON_TIME);

    digitalWrite(LASER, LOW);
    delay(LASER_OFF_TIME);

    ultimoIndiceDisparo = indiceAtual;
  }
}


/* ========== ARDUINO SETUP ========== */

/*
 * Initializes serial communication, I/O pins, laser output,
 * and the operating parameters of both stepper motors.
 */
void setup() {
  Serial.begin(115200);

  // Configure laser and limit switch pins
  pinMode(LASER, OUTPUT);
  pinMode(END_F, INPUT_PULLUP);
  pinMode(END_I, INPUT_PULLUP);

  // Ensure the laser starts turned off
  digitalWrite(LASER, LOW);

  // Configure X-axis speed and acceleration
  stepperX.setMaxSpeed(2000);
  stepperX.setAcceleration(1000);

  // Configure Y-axis speed and acceleration
  stepperY.setMaxSpeed(2000);
  stepperY.setAcceleration(1000);
}


/* ========== MAIN CONTROL LOOP ========== */

void loop() {

  /* ========== CONTINUOUS MICROPHONE ACQUISITION ========== */

  // Read the microphone's analog signal
  micReadings[sampleIndex] = analogRead(micPin);

  // Move to the next position in the circular sample buffer
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

  // Calculate the average of the most recent samples
  int micAvg = getMicAverage();


  /* ========== IMPACT DETECTION ========== */

  // Detect an impact when the averaged microphone signal
  // exceeds the configured threshold
  if (micAvg > MIC_THRESHOLD && !impactoDetectado) {
    impactoDetectado = true;
    Serial.println("Impact detected!");
  }

  // Reset the detection flag when the signal falls sufficiently
  // below the threshold.
  //
  // The 50-unit difference provides hysteresis and helps prevent
  // repeated detections caused by small signal fluctuations.
  if (micAvg < MIC_THRESHOLD - 50) {
    impactoDetectado = false;
  }

  // Output the current averaged microphone value
  // to the Serial Monitor for monitoring and debugging
  Serial.println(micAvg);


  /* ========== LIMIT SWITCH MONITORING ========== */

  // If the final limit switch is reached:
  // - Turn off the laser
  // - Return the system to the homing state
  if (digitalRead(END_F) == LOW) {
    digitalWrite(LASER, LOW);
    estadoAtual = HOMING;
  }


  /* ========== MOTOR AND LASER CONTROL ========== */

  // The state machine determines which movement operation
  // should be executed during the current iteration.
  switch (estadoAtual) {

    /* ---------- HOMING ---------- */

    case HOMING:
      homing();
      break;


    /* ---------- X-AXIS POSITIVE MOVEMENT ---------- */

    case X_POS:

      // Move the X-axis toward its maximum position
      stepperX.moveTo(X_MAX);
      stepperX.run();

      // Check whether the current position requires a laser trigger
      controleLaser(stepperX.currentPosition());

      // Once the X-axis reaches its target:
      // - Reset the laser trigger index
      // - Advance to the next Y-axis row
      if (!stepperX.isRunning()) {
        ultimoIndiceDisparo = -1;
        estadoAtual = Y_POS;
      }

      break;


    /* ---------- X-AXIS NEGATIVE MOVEMENT ---------- */

    case X_NEG:

      // Move the X-axis back toward the initial position
      stepperX.moveTo(0);
      stepperX.run();

      // Check whether the current position requires a laser trigger
      controleLaser(stepperX.currentPosition());

      // Once the X-axis reaches its target:
      // - Reset the laser trigger index
      // - Advance to the next Y-axis row
      if (!stepperX.isRunning()) {
        ultimoIndiceDisparo = -1;
        estadoAtual = Y_POS;
      }

      break;


    /* ---------- Y-AXIS MOVEMENT ---------- */

    case Y_POS:

      // Check whether all rows have already been scanned.
      // If so, restart the homing procedure.
      if (linhaAtual >= TOTAL_LINHAS - 1) {
        estadoAtual = HOMING;
        break;
      }

      // Once the Y-axis is stationary, calculate the next row
      // and update the current row counter.
      if (!stepperY.isRunning()) {
        long novoDestino = stepperY.currentPosition() + Y_STEP;
        stepperY.moveTo(novoDestino);
        linhaAtual++;
      }

      // Execute the Y-axis movement
      stepperY.run();

      // After completing the Y-axis movement, determine the
      // direction of the next X-axis scan.
      //
      // If X is at position 0, scan toward X_MAX.
      // Otherwise, scan back toward position 0.
      if (!stepperY.isRunning()) {
        estadoAtual = (stepperX.currentPosition() == 0) ? X_POS : X_NEG;
      }

      break;
  }


  /*
   * Small delay used to maintain an approximate microphone
   * sampling rate of 200 Hz while allowing the control loop
   * to continue operating.
   */
  delay(5);
}
