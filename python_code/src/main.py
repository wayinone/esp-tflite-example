import logging
import tensorflow as tf
from tensorflow.keras import Model
from tensorflow.keras.layers import (
    LSTM,  Conv2D, Lambda, Input, Reshape, 
    Dense, Softmax
)
import subprocess
from textwrap import dedent
from pathlib import Path

# configure logging to info level
logging.basicConfig(level=logging.INFO)

ROOT_REPO_DIR = 'esp-tflite-example'
MODEL_FILTE_NAME = "model"
TFLITE_MODEL_PATH= f"./model_output/{MODEL_FILTE_NAME}.tflite"
C_MODEL_REL_PATH = "main/include/model"  # relative path to the ROOT_REPO_DIR

def create_model() -> Model:
    """
    This creates model with the following spec:
        input image shape: (32, 96, 1), this is (w, h, channels)
        output dimension (time_steps, number_of_classes): (70, 11)
        The input dtype should be float32.
        The output dtype should be float32.

    @return: the tf model created.

    """
    inputs = Input(shape=[32, 96, 1], name="input")
    x = Lambda(lambda x: x / 255)(inputs)
    x = Conv2D(32, (5, 5), activation='relu', padding='same', strides=[7,7])(x)
    x = Reshape((x.shape[-3] * x.shape[-2], x.shape[-1]))(x)
    lstm = LSTM(16, return_sequences=True)(x)
    fully_connected = Dense(11)(lstm)
    output = Softmax()(fully_connected)
    model = Model(inputs=inputs, outputs=output)
    return model

def get_tflite_model(model: Model, tflite_path: str):
    """
    This function converts the model to tflite model and saved to `tflite_path`.

    @param model: Model, the model to be converted to tflite
    @param tflite_path: str, the path to save the tflite model
    """
    def representative_dataset():
        for _ in range(500):
            yield [
                tf.random.uniform(
                    shape=(1, 32, 96, 1),
                    dtype=float,
                    minval=0,
                    maxval=255
                )
            ]
    # convert model to concrete function so that it can define input and output type
    run_model = tf.function(lambda x: model(x))    
    concrete_function = run_model.get_concrete_function(
        tf.TensorSpec(shape=(1, 32, 96, 1), dtype=tf.float32)
    )

    converter = tf.lite.TFLiteConverter.from_concrete_functions([concrete_function], model)
    converter.representative_dataset = representative_dataset
    converter.optimizations = [tf.lite.Optimize.DEFAULT]  
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8] 

    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    tflite_model = converter.convert()
    with open(tflite_path, 'wb') as f:
        f.write(tflite_model)

def convert_to_tflite_c_model(tflite_model_path: str, c_file_path: str, h_file_path: str):
    """
    This function converts the tflite model to C array. 

    @param tflite_model_path: str, path to the tflite model
    @param c_file_path: str, path to the C file
    @param h_file_path: str, path to the header file

    """
    model_data_param_name = "model_data"

    command = ["xxd", "-i", "-n", model_data_param_name, tflite_model_path, c_file_path]

    raw_h_file_path = h_file_path.split("/")[-1]
    call_status = subprocess.run(command)
    if call_status.returncode != 0:
        raise ValueError(f"Failed to save the model to C file. Command: {command}")
    with open(c_file_path, 'r') as f:
        c_content = f.read()
        c_content = c_content.replace("unsigned char", "alignas(8) const unsigned char")
        c_content = c_content.replace("unsigned int", "const int")
    with open(c_file_path, 'w') as f:
        f.write(f"#include \"{raw_h_file_path}\"\n\n")
        f.write(c_content)
    with open(h_file_path, 'w') as f:
        f.write(
            dedent(
            f"""
            extern const unsigned char {model_data_param_name}[];
            extern const int {model_data_param_name}_len;
            """
            )
    )

def find_root_dir(dir_name: str = ROOT_REPO_DIR) -> Path:
    """
    """
    current_dir = Path(__file__).resolve().parent
    while current_dir.name != dir_name:
        current_dir = current_dir.parent
    return current_dir

c_model_dir = (find_root_dir() / C_MODEL_REL_PATH).as_posix()
c_model_path = f"{c_model_dir}/{MODEL_FILTE_NAME}.cc"
h_model_path = f"{c_model_dir}/{MODEL_FILTE_NAME}.h"

if __name__ == "__main__":
    logging.info("Creating Tensorflow model...\n")
    model = create_model()    

    logging.info("Tensorflow model created, converting to tflite...\n")
    get_tflite_model(model, TFLITE_MODEL_PATH)

    logging.info(f"tflite model created and saved into {TFLITE_MODEL_PATH}, converting to C array...\n")
    convert_to_tflite_c_model(TFLITE_MODEL_PATH, c_model_path, h_model_path)

    logging.info(f"model C array saved to {c_model_path}, and header file saved to {h_model_path}\n")