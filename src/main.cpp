/*
 * Rotary Dial Test Program
 * 
 * Reliable test sketch to verify rotary dial wiring and operation.
 * Uses proven pulse detection methods and shunt switch completion.
 * 
 * GPIO Configuration:
 * - GPIO 15: ROTARY_PULSE (pulse switch - counts dial pulses)
 * - GPIO 14: ROTARY_SHUNT (off-normal switch - detects dialing state)
 * 
 * Features:
 * - Counts pulses on HIGH transitions for reliability
 * - Uses shunt switch for immediate completion detection
 * - Proper debouncing (20ms pulse, 50ms shunt)
 * - Safety timeout backup (3 seconds)
 * - Works with both 3-wire and 4-wire rotary dials
 * 
 * How to use:
 * 1. Connect your rotary dial according to the wiring diagram in README.md
 * 2. Upload this sketch to your ESP32
 * 3. Open Serial Monitor at 115200 baud
 * 4. Dial digits and watch the output
 * 
 * Expected behavior:
 * - Dial "1" → "✓ Digit dialed: 1 (1 pulses)"
 * - Dial "5" → "✓ Digit dialed: 5 (5 pulses)"
 * - Dial "0" → "✓ Digit dialed: 0 (10 pulses)"
 * 
 * Results appear immediately when dial returns to rest position.
 */

#include <Arduino.h>

// Pin definitions (same as RetroBell project)
#define ROTARY_PULSE_PIN 15   // Pulse switch (counts rotations)
#define ROTARY_SHUNT_PIN 14   // Shunt/off-normal switch (active while dialing)

// Dial detection variables (simplified like working sketch)
volatile int pulseCount = 0;
volatile bool dialing = false;
volatile unsigned long lastPulseTime = 0;
volatile unsigned long dialingTimeout = 0;

// State tracking
volatile bool lastDialState = HIGH;
volatile bool lastPulseState = HIGH;

// Timing constants (based on working Arduino sketch)
#define PULSE_DEBOUNCE_MS 20         // Debounce time for pulse switch
#define DIAL_DEBOUNCE_MS 50          // Debounce time for dial switch  
#define DIAL_TIMEOUT_MS 1500         // Time after last pulse to consider dialing complete

// Interrupt Service Routines (simplified approach like working sketch)
void IRAM_ATTR onPulse() {
  unsigned long now = millis();
  
  // Debounce
  static unsigned long lastPulseDebounce = 0;
  if (now - lastPulseDebounce < PULSE_DEBOUNCE_MS) {
    return;
  }
  
  bool currentPulseState = digitalRead(ROTARY_PULSE_PIN);
  if (currentPulseState != lastPulseState) {
    lastPulseDebounce = now;
    
    // Count on HIGH transitions (like working Arduino sketch)
    if (dialing && currentPulseState == HIGH) {
      pulseCount++;
      lastPulseTime = now;
      dialingTimeout = now;  // Reset timeout on each pulse
    }
    
    lastPulseState = currentPulseState;
  }
}

void IRAM_ATTR onShuntChange() {
  unsigned long now = millis();
  
  // Debounce
  static unsigned long lastDialDebounce = 0;
  if (now - lastDialDebounce < DIAL_DEBOUNCE_MS) {
    return;
  }
  
  bool currentDialState = digitalRead(ROTARY_SHUNT_PIN);
  if (currentDialState != lastDialState) {
    lastDialDebounce = now;
    
    // Start dialing when shunt goes LOW
    if (!dialing && currentDialState == LOW) {
      dialing = true;
      pulseCount = 0;
      dialingTimeout = now;
      Serial.println("\n[Dial started turning]");
    }
    // End dialing when shunt goes HIGH (dial returned to rest)
    else if (dialing && currentDialState == HIGH) {
      dialing = false;
      Serial.println("\n[Dial returned to rest]");
      
      // Process the digit immediately when dial returns to rest
      if (pulseCount > 0) {
        // Convert pulse count to digit (10 pulses = 0)
        int digit = (pulseCount == 10) ? 0 : pulseCount;
        
        Serial.println();
        Serial.print("✓ Digit dialed: ");
        Serial.print(digit);
        Serial.print(" (");
        Serial.print(pulseCount);
        Serial.println(" pulses)");
        Serial.println();
      }
    }
    
    lastDialState = currentDialState;
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
  
  // Attach interrupts - try CHANGE to catch both edges
  attachInterrupt(digitalPinToInterrupt(ROTARY_PULSE_PIN), onPulse, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_SHUNT_PIN), onShuntChange, CHANGE);
  
  // Show initial switch states for debugging
  Serial.println("Initial switch states:");
  Serial.print("  Pulse switch (GPIO 15): ");
  Serial.println(digitalRead(ROTARY_PULSE_PIN) ? "HIGH" : "LOW");
  Serial.print("  Shunt switch (GPIO 14): ");
  Serial.println(digitalRead(ROTARY_SHUNT_PIN) ? "HIGH" : "LOW");
  Serial.println();
  
  Serial.println("Ready! Start dialing...\n");
}

void loop() {
  unsigned long now = millis();
  
  // Handle pulse display (show dots for visual feedback)
  static int lastDisplayedCount = 0;
  if (dialing && pulseCount > lastDisplayedCount) {
    Serial.print(".");
    Serial.print("[");
    Serial.print(pulseCount);
    Serial.print("]");
    lastDisplayedCount = pulseCount;
  }
  
  // Reset display counter when not dialing
  if (!dialing) {
    lastDisplayedCount = 0;
  }
  
  // Keep timeout as safety backup (in case shunt switch fails)
  if (dialing && (now - dialingTimeout) > (DIAL_TIMEOUT_MS * 2)) {  // 3 seconds as backup
    // Safety timeout reached - something went wrong
    dialing = false;
    
    Serial.println("\n[Safety timeout - dial may be stuck]");
    
    if (pulseCount > 0) {
      // Convert pulse count to digit (10 pulses = 0)
      int digit = (pulseCount == 10) ? 0 : pulseCount;
      
      Serial.println();
      Serial.print("✓ Digit dialed: ");
      Serial.print(digit);
      Serial.print(" (");
      Serial.print(pulseCount);
      Serial.println(" pulses)");
      Serial.println();
    }
  }
  
  delay(10);  // Small delay to prevent tight loop
}
