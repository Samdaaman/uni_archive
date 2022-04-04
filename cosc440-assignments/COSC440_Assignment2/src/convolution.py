from __future__ import absolute_import
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
import numpy as np
import tensorflow as tf

try:
    np.lib.stride_tricks.sliding_window_view
except:
    raise Exception('Must be run with numpy 1.21 or later, see the Readme')


def _conv_2d_of_preprocessed(inputs: np.ndarray, filters: np.ndarray, speed: int):
    """
    Performs 2D convolution given 4D inputs, filters and outputs (as np.ndarrays)
    Inputs must be preprocessed (includes any padding and transposing)
    :param inputs: np.ndarray with shape [num_examples, in_channels, in_height, in_width] (note different order from other function)
    :param filters: tensor with shape [filter_height, filter_width, in_channels, out_channels]
    :param fast: bool whether to use the fast or slow code (for performance comparisons - see performance_test.py)
    :return: outputs: np.ndarray with shape [num_examples, output_height, output_height, 1]
    """
    num_images, input_height, input_width, num_input_channels = inputs.shape
    filter_height, filter_width, _, num_output_channels = filters.shape
    output_height, output_width = (in_dim - filter_dim + 1 for in_dim, filter_dim in ((input_height, filter_height), (input_width, filter_width)))

    if speed == 3:
        # This is the FAST code (took me a while to remove for loops one by one - inspired by https://scicomp.stackexchange.com/a/34720)
        inputs_expanded = np.lib.stride_tricks.sliding_window_view(inputs, (filter_height, filter_width), (1,2))
        return tf.einsum('ijklmn,mnlo->ijko', inputs_expanded, filters).numpy()

    else:
        outputs = np.zeros((inputs.shape[0], output_height, output_width, num_output_channels), dtype=np.float32) # only works with one output channel
        
        if speed == 2:
            # This is MEDIUM speed code that is used to compare in performance_test.py
            inputs_expanded = np.lib.stride_tricks.sliding_window_view(inputs, (filter_height, filter_width), (1,2))
            for input_i, input in enumerate(inputs):
                for input_channel_i in range(num_input_channels):
                    for output_channel_i in range(num_output_channels):
                        channel_filter = filters[:, :, input_channel_i, output_channel_i]
                        for o_y in range(output_height):
                            for o_x in range(output_width):
                                outputs[input_i][o_y][o_x][output_channel_i] += np.tensordot(inputs_expanded[input_i][o_y][o_x][input_channel_i], channel_filter)
        else:
            # This is crap SLOW code that is used to compare in performance_test.py
            for input_i, input in enumerate(inputs):
                for input_channel_i in range(num_input_channels):
                    channel = input[:, :, input_channel_i]
                    for output_channel_i in range(num_output_channels):
                        channel_filter = filters[:, :, input_channel_i, output_channel_i]
                        for o_y in range(output_height):
                            for o_x in range(output_width):
                                for f_y in range(filter_height):
                                    for f_x in range(filter_width):
                                        outputs[input_i][o_y][o_x][output_channel_i] += channel[o_y + f_y][o_x + f_x] * channel_filter[f_y][f_x]
        return outputs

def conv2d(inputs: np.ndarray, filters: np.ndarray, strides: np.ndarray, padding: np.ndarray, speed=3):
    """
    Performs 2D convolution given 4D inputs and filter Tensors.
    :param inputs: tensor with shape [num_examples, in_height, in_width, in_channels]
    :param filters: tensor with shape [filter_height, filter_width, in_channels, out_channels]
    :param strides: MUST BE [1, 1, 1, 1] - list of strides, with each stride corresponding to each dimension in input
    :param padding: either "SAME" or "VALID", capitalization matters
    :return: outputs, NumPy array or Tensor with shape [num_examples, output_height, output_width, output_channels]
    """
    num_examples, in_height, in_width, input_in_channels = inputs.shape
    filter_height, filter_width, filter_in_channels, filter_out_channels = filters.shape
    num_examples_stride, strideY, strideX, channels_stride = strides

    # Check arguments
    assert input_in_channels == filter_in_channels # one filter for each channel
    assert num_examples_stride == 1 and strideX == 1 and strideY == 1 and channels_stride == 1 # only works with a stride of 1 (in all directions)
    
    if padding == 'SAME':
        padding_height, padding_width = [(filter_dim - 1) // 2 for filter_dim in [filter_height, filter_width]]
        inputs_padded = np.pad(inputs, ((0,), (padding_height,), (padding_width,), (0,)))
        return _conv_2d_of_preprocessed(inputs_padded, filters, speed)

    elif padding == 'VALID':
        return _conv_2d_of_preprocessed(inputs, filters, speed)

    else:
        raise NotImplementedError('padding must be VALID or SAME')


if __name__ == '__main__':
    import assignment_better_tests
    for speed in range(1, 4):
        print(f'Testing speed={speed}')
        imgs = np.array([[3, 5, 3, 3], [5, 1, 4, 5], [2, 5, 0, 1], [3, 3, 2, 1]], dtype=np.float32)
        imgs = np.reshape(imgs, (1, 4, 4, 1))
        filters = np.eye(3, 3).reshape((3, 3, 1, 1))
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME", speed=speed)
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        assignment_better_tests.TestAssignment2().assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())