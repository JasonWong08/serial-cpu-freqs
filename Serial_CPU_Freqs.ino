/*
   Simple Sketch for testing HardwareSerial with different CPU Frequencies
   Changing the CPU Frequency may affect peripherals and Wireless functionality
   In ESP32 Arduino, UART shall work correctly in order to let the user see DGB info
   and other application messages.

   CPU Frequency is usually lowered in sleep modes
   and some other Low Power configurations

*/

int cpufreqs = 240;

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n Starting...\n");
  Serial.flush();

  // testing HardwareSerial for all possible CPU/APB Frequencies

  Serial.printf("\n------- Trying CPU Freq = %d ---------\n", cpufreqs);
  Serial.flush();  // wait to empty the UART FIFO before changing the CPU Freq.
  setCpuFrequencyMhz(cpufreqs);

  // initial information
  uint32_t Freq = getCpuFrequencyMhz();
  Serial.print("CPU Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getXtalFrequencyMhz();
  Serial.print("XTAL Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getApbFrequency();
  Serial.print("APB Freq = ");
  Serial.print(Freq);
  Serial.println(" Hz");
  delay(500);
  // Freq = getCpuFrequencyMhz();
  // Serial.print("CPU Freq = ");
  // Serial.print(Freq);
  // Serial.println(" MHz");
  // Freq = getXtalFrequencyMhz();
  // Serial.print("XTAL Freq = ");
  // Serial.print(Freq);
  // Serial.println(" MHz");
  // Freq = getApbFrequency();
  // Serial.print("APB Freq = ");
  // Serial.print(Freq);
  // Serial.println(" Hz");

  Serial.println("\n-------------------\n");
  Serial.println("End of testing...");
  Serial.println("\n-------------------\n");
}

void loop() {
  // Nothing here so far
}
