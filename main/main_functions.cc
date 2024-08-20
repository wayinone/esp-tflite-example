

#include "main_functions.h"

#include "include/model/model.h"
#include "test_image/image_data.c"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_log.h>
#include "esp_main.h"

void cast_uint8_arr_to_float32_arr(const uint8_t *in, int size, float *out) {   
  for (int i = 0; i < size; i++) {
    out[i] = (float)in[i];
  }
}

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  // An area of memory to use for input, output, and intermediate arrays.
  constexpr int kTensorArenaSize = 300 * 1024;
  // static uint8_t *tensor_arena;//[kTensorArenaSize]; // Maybe we should move this to external
  static uint8_t *tensor_arena;//[kTensorArenaSize];
  constexpr size_t input_length = 32*96*1;
}  // namespace


void setup() {
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
                "version %d.", model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  if (tensor_arena == NULL) {
    tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    printf("%s: free RAM size: %d, INTERNAL: %d, PSRAM: %d\n", "tag", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM));

  }
  if (tensor_arena == NULL) {
    printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
    return;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddFullyConnected();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddUnidirectionalSequenceLSTM();
    micro_op_resolver.AddSoftmax();
    micro_op_resolver.AddAdd();
    

  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  printf("AllocateTensors...\n");

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
  printf("TF Lite init done\n");
}


void loop() {
  // cast_uint8_arr_to_float32_arr(image_data, size, input->data.f);
  for (int i = 0; i < input_length; i++) {
    input->data.int8[i] = image_data[i] ^ 0x80;
  }

  uint64_t start_time = esp_timer_get_time();
  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke()) {
    MicroPrintf("Invoke failed.");
  }

  uint64_t end_time = esp_timer_get_time();
  MicroPrintf("Inference time: %llu second", (end_time - start_time) / 1000000);

  TfLiteTensor* output = interpreter->output(0);

  MicroPrintf("Output (just the first element): %d\n", output->data.int8[0]);

  vTaskDelay(1); // to avoid watchdog trigger
}

// #if defined(COLLECT_CPU_STATS)
//   long long total_time = 0;
//   long long start_time = 0;
//   extern long long softmax_total_time;
//   extern long long dc_total_time;
//   extern long long conv_total_time;
//   extern long long fc_total_time;
//   extern long long pooling_total_time;
//   extern long long add_total_time;
//   extern long long mul_total_time;
// #endif

