# Use a trained DeepFuse Net to generate

import tensorflow as tf
import numpy as np

from fusion_l1norm import L1_norm
from deep_fuse_net import DeepFuseNet
from utils import get_images, save_images, get_train_images
from tensorflow.python.tools import freeze_graph

def generate(content_path, style_path, model_path, model_pre_path, ssim_weight, index, type='addition', output_path=None):
    
    if type == 'addition':
        outputs = _handler(content_path, style_path, model_path, model_pre_path, ssim_weight, index, output_path=output_path)
        return list(outputs)
    elif type == 'l1':
        outputs = _handler_l1(content_path, style_path, model_path, model_pre_path, ssim_weight, index, output_path=output_path)
        return list(outputs)

def pb_to_tflite(input_frozen_graph, input_array, output_array, output_model_name):
    converter = tf.lite.TFLiteConverter.from_frozen_graph(input_frozen_graph, input_array, output_array)
    tflite_quentized_model = converter.convert()
    open(output_model_name, "wb").write(tflite_quentized_model)

def _handler(content_name, style_name, model_path, model_pre_path, ssim_weight, index, output_path=None):
    content_path = content_name
    style_path = style_name

    content_img = get_train_images(content_path, flag=False)
    style_img   = get_train_images(style_path, flag=False)
    dimension = content_img.shape

    content_img = content_img.reshape([1, dimension[0], dimension[1], dimension[2]])
    style_img   = style_img.reshape([1, dimension[0], dimension[1], dimension[2]])

    content_img = np.transpose(content_img, (0, 2, 1, 3))
    style_img = np.transpose(style_img, (0, 2, 1, 3))

    with tf.Graph().as_default(), tf.Session() as sess:

        # build the dataflow graph
        content = tf.placeholder(
            tf.float32, shape=content_img.shape, name='content')
        style = tf.placeholder(
            tf.float32, shape=style_img.shape, name='style')

        dfn = DeepFuseNet(model_pre_path)

        output_image = dfn.transform_addition(content,style)
        # output_image = dfn.transform_recons(style)
        # output_image = dfn.transform_recons(content)

        # restore the trained model and run the style transferring
        saver = tf.train.Saver()
        saver.restore(sess, model_path)

        output = sess.run(output_image, feed_dict={content: content_img, style: style_img})

        tf.io.write_graph(sess.graph, output_path,'train.pbtxt')
        tf.io.write_graph(sess.graph, output_path , 'train.pb' , as_text=False)
        saver.save(sess,'TEST')
        freeze_graph.freeze_graph(output_path + 'train.pbtxt', "",False,'TEST','BiasAdd_6' ,"","",output_path + 'deepfusion.pb',False,"")

        input_array = ['content','style' ]
        output_array = ['BiasAdd_6']

        pb_to_tflite(output_path + 'deepfusion.pb', input_array, output_array, output_path +  'deepfusion.tflite')

        save_images(content_path, output, output_path,
                    prefix='fused' + str(index), suffix='_deepfuse_addition'+str(ssim_weight))

    return output


def _handler_l1(content_name, style_name, model_path, model_pre_path, ssim_weight, index, output_path=None):
    content_path = content_name
    style_path = style_name

    content_img = get_train_images(content_path, flag=False)
    style_img   = get_train_images(style_path, flag=False)
    dimension = content_img.shape

    content_img = content_img.reshape([1, dimension[0], dimension[1], dimension[2]])
    style_img   = style_img.reshape([1, dimension[0], dimension[1], dimension[2]])

    content_img = np.transpose(content_img, (0, 2, 1, 3))
    style_img = np.transpose(style_img, (0, 2, 1, 3))

    with tf.Graph().as_default(), tf.Session() as sess:

        # build the dataflow graph
        content = tf.placeholder(
            tf.float32, shape=content_img.shape, name='content')
        style = tf.placeholder(
            tf.float32, shape=style_img.shape, name='style')

        dfn = DeepFuseNet(model_pre_path)
                
        enc_ir = dfn.transform_encoder(content)
        enc_vis= dfn.transform_encoder(style)

        target = tf.placeholder(tf.float32, shape=enc_ir.shape, name='target')

        output_image = dfn.transform_decoder(target)

        # restore the trained model and run the style transferring
        saver = tf.train.Saver()
        saver.restore(sess, model_path)
        
        enc_ir_temp, enc_vis_temp = sess.run([enc_ir, enc_vis], feed_dict={content: content_img, style: style_img})
        feature = L1_norm(enc_ir_temp, enc_vis_temp)
        
        output = sess.run(output_image, feed_dict={target: feature})

        tf.io.write_graph(sess.graph, output_path,'train.pbtxt')
        tf.io.write_graph(sess.graph, output_path , 'train_l1norm.pb' , as_text=False)
        saver.save(sess,'TEST')
        freeze_graph.freeze_graph(output_path + 'train.pbtxt', "",False,'TEST','BiasAdd_6' ,"","", 'deepfusion_l1norm.pb',False,"")

        """
        output_image = dfn.transform_l1norm(content,style)
        # output_image = dfn.transform_recons(style)
        # output_image = dfn.transform_recons(content)

        # restore the trained model and run the style transferring
        saver = tf.train.Saver()
        saver.restore(sess, model_path)
        """
        """
        input_array = ['content','style' ]
        output_array = ['BiasAdd_6']
        pb_to_tflite('./deepfusion.pb', input_array, output_array, 'deepfusion.tflite')
        """
        
        save_images(content_path, output, output_path,
                    prefix='fused' + str(index), suffix='_deepfuse_l1norm_'+str(ssim_weight))
        
    return output
