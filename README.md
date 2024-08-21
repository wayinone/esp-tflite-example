# esp-tflite-example
This repo is used to solve the issue with my build of deploying tflite model on my esp32-cam. 
Currently, the build is successful, but when running on my esp32-cam (ESP32-CAM by AI Thinker). There is an issue about FullyConnectedEval in `esp-tflite-micro` tha causes system abort.

## Issue
After build and upload into the esp32-cam. The monitor shows the following error:
```
abort() was called at PC 0x400e21a8 on core 0
0x400e21a8: tflite::(anonymous namespace)::FullyConnectedEval(TfLiteContext*, TfLiteNode*) at /Users/wayinone/Documents/git-repos/esp-tflite-example/managed_components/espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc:100 (discriminator 1)
```
This error should be due to `node->builtin_data = nullptr` when running it.

## Environment

### ESP Device
esp32-cam (ESP32-CAM by AI Thinker)

### IDE
I am building this esp-idf project on PlatformIO, with esp-idf extension.

### esp-idf version
This is defined in `main/idf_component.yml`, currently I am running with ESP-IDF v5.3

### Python
python3.10

### Tensorflow
Tensorflow 2.15.1.

Tensorflow is used to produce model, the code is in `python_code` folder, where the [readme](python_code/README.md) describe how I generate a tensorflow-lite model and create C array and header file for the model I generated.



## Settings in kconfig.projbuild
* check for `Support for external, SPI-connected RAM`
* Camera Configuration -> Select Camera Pinout -> `ESP32-CAM by AI-Thinker`
