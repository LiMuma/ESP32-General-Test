#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h> 

#define I2S_SAMPLE_RATE 44100 // Sample rate in Hz
#define I2S_DOUT 14
#define I2S_BCK 13
#define I2S_LRC 12

void playSineTone(float frequency, int duration_ms) {
  const int amplitude = 3000;
  const int samples = I2S_SAMPLE_RATE * duration_ms / 1000;
  const float increment = (2 * M_PI * frequency) / I2S_SAMPLE_RATE;
  
  int16_t buffer[samples];
  for (int i = 0; i < samples; i++) {
    double sample = sin(i * increment);
    int16_t value = (int16_t)(amplitude * sample);
    int32_t sample_stereo = (value << 16) | (value & 0xFFFF); // Create stereo sample
    size_t bytes_written;
    i2s_write(I2S_NUM_0, &sample_stereo, sizeof(sample_stereo), &bytes_written, portMAX_DELAY);
  }
}

void initAudio() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0, // Default interrupt flags
    .dma_buf_count = 8, // Number of DMA buffers
    .dma_buf_len = 1024, // Length of each DMA buffer
    .use_apll = false, // Do not use APLL
    .tx_desc_auto_clear = true, // Auto clear TX descriptor on underflow
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK, // Bit Clock pin
    .ws_io_num = I2S_LRC, // Word Select pin
    .data_out_num = I2S_DOUT, // Data Out pin
    .data_in_num = I2S_PIN_NO_CHANGE, // No Data In pin
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  
  playSineTone(440.0, 2000); // Play a 440 Hz sine tone for 2 seconds
}
void playAudio() {
  // put your main code here, to run repeatedly:
  delay(1000); // Delay for 1 second
  playSineTone(261.63, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(293.66, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(329.63, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(349.23, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(392.00, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(440.00, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(493.88, 2000); // Play a 440 Hz sine tone for 2 seconds
  playSineTone(523.25, 2000); // Play a 440 Hz sine tone for 2 seconds
}
