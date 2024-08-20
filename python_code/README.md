## Python Code to generate Tensorflow model and create C array for esp32 device

### Create Python virtual environment (under `python_code/environment`)
Running the following command under folder `python_code`
```
sh script/create_venv.sh
```

### Create model C array and header file
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

### Viewing tflite model
Once the model file generated, one can view the model with:
```
netron model_output/model.tflite  
```