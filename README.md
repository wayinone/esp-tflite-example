# esp-tflite-example
This repo is used to seek a way to resolve the issue with my build of deploying tflite model on my esp32-cam. 
Currently, the code can build and upload into my esp32-cam device. However, there is an issue about `FullyConnectedEval` in `esp-tflite-micro` tha causes system abort while running the device.

## esp-idf code
In `main` folder you can find the code that reproduce the error. The code is trying to mimic the layout of the official example [person_detection](https://github.com/espressif/esp-tflite-micro/tree/master/examples/person_detection). I can run all 3 official example successfully on my device. 

Here is the brief description of the process in the `main` (which calls `main_functions.cc`):
1. Initiate tflite interpreter in `setup()` with the model located in `./main/include/model`
   * The input of the model should be a 32x96x1 (H x W x Channels) grayscal image, with dtype as uint8
2. In the `loop()`
   * grab the image from `./main/test_image/image_data.c`
   * invoke the interpreter for prediction 

## Issue
After build and upload into the esp32-cam. The monitor shows the following error:
```
abort() was called at PC 0x400e21a8 on core 0
0x400e21a8: tflite::(anonymous namespace)::FullyConnectedEval(TfLiteContext*, TfLiteNode*)
at ***/esp-tflite-example/managed_components/espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc:100 (discriminator 1)
```
This error should be due to `node->builtin_data = nullptr` in `espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc:100`

## Configuration

#### ESP Device
esp32-cam (ESP32-CAM by AI Thinker). In PlatformIO's esp-idf extension, the target is `espe32`

#### IDE
I am building this esp-idf project on PlatformIO, with esp-idf extension.

#### esp-idf version
This is defined in `main/idf_component.yml`, currently I am running with ESP-IDF v5.3.0

#### Python (for generating model and converting into tflite model)
Python3.10

#### Tensorflow
Tensorflow 2.15.1.

Tensorflow is used to produce model, and the code is in `python_code` folder. One can view the [readme](python_code/README.md) to understand the following:
  * Generate a tensorflow-lite (tflite) model.
  * From a tflite model, create C array and header file, and use it in esp-idf code.

## Settings in kconfig.projbuild
(access via the gear icon (`ESP-IDF:SDK Configuration Editor` ) in the bottom of IDE, i.e. the `esp-idf.py menuconfig`)
* check for `Support for external, SPI-connected RAM`
* `Camera Configuration` -> Select `Camera Pinout` -> `ESP32-CAM by AI-Thinker`
