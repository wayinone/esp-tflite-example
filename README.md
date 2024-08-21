# esp-tflite-example
This repo is used to seek a way to resolve the issue with my build of deploying tflite model on my esp32-cam. 
Currently, the code can build and upload into my esp32-cam device. However, there is an issue about FullyConnectedEval in `esp-tflite-micro` tha causes system abort.

## Issue
After build and upload into the esp32-cam. The monitor shows the following error:
```
abort() was called at PC 0x400e21a8 on core 0
0x400e21a8: tflite::(anonymous namespace)::FullyConnectedEval(TfLiteContext*, TfLiteNode*) at ***/esp-tflite-example/managed_components/espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc:100 (discriminator 1)
```
This error should be due to `node->builtin_data = nullptr` in `espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc:100`

## Configuration

### ESP Device
esp32-cam (ESP32-CAM by AI Thinker). In PlatformIO's esp-idf extension, the target is `espe32`


### IDE
I am building this esp-idf project on PlatformIO, with esp-idf extension.

### esp-idf version
This is defined in `main/idf_component.yml`, currently I am running with ESP-IDF v5.3.0

### Python
Python3.10

### Tensorflow
Tensorflow 2.15.1.

Tensorflow is used to produce model, and the code is in `python_code` folder. One can view the [readme](python_code/README.md) to understand the following:
  * Generate a tensorflow-lite (tflite) model.
  * From a tflite model, create C array and header file, and use it in esp-idf code.


## Settings in kconfig.projbuild
* check for `Support for external, SPI-connected RAM`
* Camera Configuration -> Select Camera Pinout -> `ESP32-CAM by AI-Thinker`
