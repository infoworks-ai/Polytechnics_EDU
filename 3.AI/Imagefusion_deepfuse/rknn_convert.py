import numpy as np
import re
import math
import random
import cv2

from rknn.api import RKNN

INPUT_SIZE_WIDTH = 640
INPUT_SIZE_HEIGHT =480

if __name__ == '__main__':

    ssim_weight = 1
    print('\nBegin to generate pictures ...\n')
    path = 'images/IV_images/'
    infrared = path + 'IR' + str(1) +'.png'
    visible = path + 'VIS' + str(1) +'.png'
    fusion_type = 'addition'

    # Create RKNN object
    rknn = RKNN(verbose=False)

    # Config for Model Input PreProcess
    rknn.config(channel_mean_value='128 128 128 128#128 128 128 128', reorder_channel='0 1 2#0 1 2')


    # Load TensorFlow Model
    print('--> Loading model')

    ret = rknn.load_tflite(model='./outputs/deepfusion.tflite')
	
    print('done')
    
    # Build Model
    print('--> Building model')
    rknn.build(do_quantization=True, dataset='./dataset.txt')
    print('done')
    
    # Export RKNN Model
    ret = rknn.export_rknn('./outputs/deepfusion.rknn')
    


    # Direct Load RKNN Model
    rknn.load_rknn('./outputs/deepfusion.rknn')

    # Set inputs
    infrared_input_temp = cv2.imread(infrared,cv2.IMREAD_GRAYSCALE)
    #infrared_input = cv2.cvtColor(infrared_input_temp, cv2.COLOR_BGR2RGB)
    infrared_input = cv2.resize(infrared_input_temp, (INPUT_SIZE_WIDTH, INPUT_SIZE_HEIGHT), interpolation=cv2.INTER_CUBIC)
    visible_input_temp = cv2.imread(visible,cv2.IMREAD_GRAYSCALE)
    #visible_input = cv2.cvtColor(visible_input_temp, cv2.COLOR_BGR2RGB)
    visible_input = cv2.resize(visible_input_temp, (INPUT_SIZE_WIDTH, INPUT_SIZE_HEIGHT), interpolation=cv2.INTER_CUBIC)




    # init runtime environment
    print('--> Init runtime environment')
    ret = rknn.init_runtime()
    if ret != 0:
        print('Init runtime environment failed')
        exit(ret)
    print('done')

    # Inference
    print('--> Running model')
    outputs = rknn.inference(inputs=[visible_input, infrared_input])
    print('done')
    print('inference result: ', outputs)
  
    img = np.array(outputs[0])
    print(img.shape)
    print('img.min = ')
    print(np.min(img))
    #img=img-np.min(img)
    print(np.max(img))
    img = img/np.max(img)*255

    img = np.reshape(img,(INPUT_SIZE_WIDTH,INPUT_SIZE_HEIGHT))
    print(img.shape)
    print(visible_input.shape)

    cv2.imwrite("out.jpg",img)

    # Evaluate Perf on Simulator
    rknn.eval_perf(inputs=[visible_input], is_print=True)

    # Release RKNN Context
    rknn.release()
   
