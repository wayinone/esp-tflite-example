# esp-tflite-example
This repo is to provide a bare minimum code to embed a tflite model into esp32 device. From official examples or some internet resources, there is yet a complete process of creating model from python, turn into tflite model, encode it, and deploy into an esp32 device.

In the meantime I discover there is a bug in the official `esp-tflite-micro` for the fully connected layer (in Tensorflow or Keras, this is `Dense` layer). The issue and solution are discussed in this read me.

The example provide here can also be debug by ESP Prog. One can check the "Debug of Esp prog" section for more detail.

## esp-idf code
In `main` folder you can find the code that reproduce the error. The code is trying to mimic the layout of the official example [person_detection](https://github.com/espressif/esp-tflite-micro/tree/master/examples/person_detection). I can run all 3 official example successfully on my device. 

Here is the brief description of the process in the `main` (which calls `main_functions.cc`):
1. Initiate tflite interpreter in `setup()` with the model located in `./main/include/model`
   * The input of the model should be a 32x96x1 (H x W x Channels) grayscale image, with dtype as uint8
2. In the `loop()`
   * grab the image from `./main/test_image/image_data.c`
   * invoke the interpreter for prediction 

### The `component` folder
The component folder put a customized `espressif__esp-tflite-micro v1.3.1` code. The distribution, license and usage of the code is under Espressif original [esp-tflite-micro](https://github.com/espressif/esp-tflite-micro)

## Build and Flash ESP Device
Check the Configuration section on how I run esp-idf frame work on vs code. The build and flash is through the usual 

```
idf.py build
```
and
```
idf.py flash
``` 

## Issue log

### espressif__esp-tflite-micro v1.3.1 issue about the fully connected layer:

After build and upload into the esp32-cam. The monitor shows the following error:
```
abort() was called at PC 0x400e21a8 on core 0
0x400e21a8: tflite::(anonymous namespace)::FullyConnectedEval(TfLiteContext*, TfLiteNode*)
at ***/esp-tflite-example/managed_components/espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc:100 (discriminator 1)
```

After using Esp prog, the panic `abort()` is caused by Ln 165 of `***/esp-tflite-example/managed_components/espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc`
```
TFLITE_DCHECK_LE(output_depth, filter_shape.Dims(filter_dim_count - 2));
```

Solution:
* move the entire folder `/espressif__esp-tflite-micro` to `/component` folder, and comment ln 156 - 187 to remove ESP_NN section (just comment out Ln 165 will lead to wrong prediction).
i.e. 
```
// At ***/esp-tflite-example/managed_components/espressif__esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc
// #if ESP_NN
//           const RuntimeShape& filter_shape = tflite::micro::GetTensorShape(filter);
//           const RuntimeShape& output_shape = tflite::micro::GetTensorShape(output);

//           TFLITE_DCHECK_GE(filter_shape.DimensionsCount(), 2);
//           TFLITE_DCHECK_GE(output_shape.DimensionsCount(), 1);
//           const int filter_dim_count = filter_shape.DimensionsCount();
//           const int batches = output_shape.Dims(0);
//           const int output_depth = output_shape.Dims(1);
//           TFLITE_DCHECK_LE(output_depth, filter_shape.Dims(filter_dim_count - 2));
//           const int accum_depth = filter_shape.Dims(filter_dim_count - 1);

//           const int32_t* bias_data =
//               tflite::micro::GetOptionalTensorData<int32_t>(bias);

//           const int8_t *input_data = tflite::micro::GetTensorData<int8_t>(input);
//           int8_t *output_data = tflite::micro::GetTensorData<int8_t>(output);
//           const int8_t *filter_data = tflite::micro::GetTensorData<int8_t>(filter);

//           for (int b = 0; b < batches; ++b) {
//             esp_nn_fully_connected_s8(input_data, -data.input_zero_point,
//                                       accum_depth,
//                                       filter_data, -data.filter_zero_point,
//                                       bias_data, output_data, output_depth,
//                                       data.output_zero_point,
//                                       data.output_shift, data.output_multiplier,
//                                       data.output_activation_min,
//                                       data.output_activation_max);
//             input_data += accum_depth;
//             output_data += output_depth;
//           }
// #else
```

## Configuration

#### ESP Device
esp32-cam (ESP32-CAM by AI Thinker). In PlatformIO's esp-idf extension, the target is `esp32`

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


# Debug of Esp prog
* Necessary setting is in `./vscode/launch.json`
* Connect USB to the ESP Prog
* Change the flash method to `JTAG` (you can either click the star icon in the bottom or change in `./vscode/setting.json` -> `idf.flashType`)
* Open an ESP_IDF Terminal
  * run `idf.py openocd`
    * if something wrong about already running openocd in the background:
      * Find the pid by  `ps aux | grep openocd`
      * `kill -9 [pid]
* Now you should be able to flash directly from JTAG (`F1` -> `ESP-IDF: Flash your Project`)
  * If failed, unplug your usb and try again.
* You can now debug by click the `Run and Debug` icon on the left.
  * You can directly set the stop point in the vs studio code (thanks to the setting in `./vscode/launch.json`)
  * I found you usually have to click twice to kick it off.

