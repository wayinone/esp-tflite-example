# Generate Model from Tensorflow
This folder contains Python code to:
1. Generate tflite (TensorflowLite) model from Tensorflow
2. create C array and header file from the tflite model for esp32 device

## How to run the python code here:
This section describe how to run the code here. It will:
1. Generate tflite (TensorflowLite) model from Tensorflow,
   * and save into `./model_output` directory.
2. create C array and header file,
   * and save into `../main` directory, which is part of esp-idf build.
   
### 1. Create Python virtual environment (under `python_code/environment`)
Running the following command under folder `python_code`
```
sh script/create_venv.sh
```

### 2. Create model C array and header file
Running the following command under folder `python_code`

```
. environment/bin/activate
python3.10 -m src.main
```
Note this command will create the following:
1. For viewing the model with netron: `python_code/model_output/model.tflite`
2. For using in ESP device: 
   * model C array: `main/include/model/model.cc`
   * header file: `main/include/model/model.h`

### 3. Viewing tflite model
Once the model file generated, one can view the model with:
```
netron model_output/model.tflite  
```
Viewing the model with `netron` is neccessary to determine which operator to use in esp-tflite-micro package (in eps-idf build)