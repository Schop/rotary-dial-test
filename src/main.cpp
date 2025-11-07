/*
 * Rotary Dial Test Program
 * 
 * Simple test sketch to verify rotary dial wiring and operation.
 * Uses the same GPIO pins as the RetroBell project.
 * 
 * GPIO Configuration:
 * - GPIO 15: ROTARY_PULSE (pulse switch)
 * - GPIO 14: ROTARY_ACTIVE (shunt/off-normal switch)
 * 
 * How to use:
 * 1. Connect your rotary dial according to the wiring diagram
 * 2. Upload this sketch
 * 3. Open Serial Monitor at 115200 baud
 * 4. Dial digits and watch the output
 * 
 * Expected behavior:
 * - Dial "1" → prints "Digit: 1" (1 pulse)
 * - Dial "5" → prints "Digit: 5" (5 pulses)
 * - Dial "0" → prints "Digit: 0" (10 pulses)
 */

#include <Arduino.h>

// Pin definitions (same as RetroBell project)
#define ROTARY_PULSE_PIN 15   // Pulse switch (counts rotations)
#define ROTARY_SHUNT_PIN 14   // Shunt/off-normal switch (active while dialing)

// Dial detection variables
volatile int pulseCount = 0;
volatile bool dialing = false;
volatile unsigned long lastPulseTime = 0;
volatile unsigned long dialStartTime = 0;

// Timing constants
#define PULSE_TIMEOUT_MS 100      // Time between pulses
#define DIAL_COMPLETE_TIMEOUT_MS 200  // Time after last pulse to consider digit complete
#define DEBOUNCE_MS 10            // Debounce time for switches

// Interrupt Service Routines
void IRAM_ATTR onPulse() {
  unsigned long now = millis();
  
  // Debounce: ignore pulses that come too quickly
  if (now - lastPulseTime < DEBOUNCE_MS) {
    return;
  }
  
  // Only count pulses when we're actively dialing
  if (dialing && digitalRead(ROTARY_PULSE_PIN) == LOW) {
    pulseCount++;
    lastPulseTime = now;
    Serial.print(".");  // Visual feedback for each pulse
  }
}

void IRAM_ATTR onShuntChange() {
  unsigned long now = millis();
  
  // Debounce
  static unsigned long lastShuntChange = 0;
  if (now - lastShuntChange < DEBOUNCE_MS) {
    return;
  }
  lastShuntChange = now;
  
  // Check if dial is being turned (shunt opens)
  if (digitalRead(ROTARY_SHUNT_PIN) == HIGH) {
    // Dial started turning
    dialing = true;
    pulseCount = 0;
    dialStartTime = now;
    Serial.println("\n[Dial started turning]");
  } else {
    // Dial returned to rest (shunt closes)
    // Don't immediately end dialing - wait for timeout
    Serial.println("\n[Dial returned to rest]");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("    Rotary Dial Test Program");
  Serial.println("========================================");
  Serial.println();
  Serial.println("GPIO Configuration:");
  Serial.println("  GPIO 15: ROTARY_PULSE (pulse switch)");
  Serial.println("  GPIO 14: ROTARY_SHUNT (off-normal switch)");
  Serial.println();
  Serial.println("Dial a digit and watch the output!");
  Serial.println("----------------------------------------");
  Serial.println();
  
  // Configure pins with internal pull-ups
  pinMode(ROTARY_PULSE_PIN, INPUT_PULLUP);
  pinMode(ROTARY_SHUNT_PIN, INPUT_PULLUP);
  
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(ROTARY_PULSE_PIN), onPulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_SHUNT_PIN), onShuntChange, CHANGE);
  
  Serial.println("Ready! Start dialing...\n");
}

void loop() {
  unsigned long now = millis();
  
  // Check if we've finished dialing a digit
  if (dialing && pulseCount > 0) {
    // If enough time has passed since the last pulse, consider the digit complete
    if (now - lastPulseTime > DIAL_COMPLETE_TIMEOUT_MS) {
      // Convert pulse count to digit (10 pulses = 0)
      int digit = (pulseCount == 10) ? 0 : pulseCount;
      
      Serial.println();
      Serial.print("✓ Digit dialed: ");
      Serial.print(digit);
      Serial.print(" (");
      Serial.print(pulseCount);
      Serial.println(" pulses)");
      Serial.println();
      
      // Reset for next digit
      pulseCount = 0;
      dialing = false;
    }
  }
  
  // Timeout if dial was started but no pulses came
  if (dialing && pulseCount == 0) {
    if (now - dialStartTime > 2000) {
      Serial.println("[Timeout - no pulses detected]");
      Serial.println();
      dialing = false;
    }
  }
  
  delay(10);  // Small delay to prevent tight loop
}
