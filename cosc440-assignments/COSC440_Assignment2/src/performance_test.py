import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

from convolution import *
from timeit import timeit
import gc
import tensorflow as tf

# Test using a 3x3 filter and increasing the image dimension sizes with a group of 10 images with 3 channels each
# This code shows the impact of using np functions as opposed to python code for convolution and sliding_windows
NUM_IMAGES = 10
NUM_INPUT_CHANNELS = 1
NUM_OUTPUT_CHANNELS = 1
FILTER_SIZE = 3
STRIDES = [1, 1, 1, 1]


results = []

# Run a test image through TF (otherwise the first one takes longer than it should) - something to do with preloading stuff I think
inputs = np.random.randint(255, size=(NUM_IMAGES, 3, 3, NUM_INPUT_CHANNELS)).astype(np.float32) / 255.0
filters = tf.Variable(tf.random.truncated_normal([FILTER_SIZE, FILTER_SIZE, NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS], dtype=tf.float32, stddev=1e-1), name="filters")
tf.nn.conv2d(inputs, filters, strides=STRIDES, padding="VALID")
conv2d(inputs, filters, strides=STRIDES, padding="VALID")

print(f'Starting a performance test with {NUM_IMAGES} images (each with {NUM_INPUT_CHANNELS} channels) and a {"x".join(str(x) for x in filters.shape)} filter')
print('starting timing... (all times below are in seconds)')
print('img_size   |   tf   |  fast  |  med   |  slow  \n' + '-'*50)

for image_size in [5, 10, 15, 20, 30, 50, 75, 100, 150, 300, 500]:
    inputs = np.random.randint(255, size=(NUM_IMAGES, image_size, image_size, NUM_INPUT_CHANNELS)).astype(np.float32) / 255.0
    filters = tf.Variable(tf.random.truncated_normal([FILTER_SIZE, FILTER_SIZE, NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS], dtype=tf.float32, stddev=1e-1), name="filters")

    tf_conv = lambda: tf.nn.conv2d(inputs, filters, strides=STRIDES, padding="VALID")
    my_conv_fast = lambda: conv2d(inputs, filters, strides=STRIDES, padding="VALID", speed=3)
    my_conv_medium = lambda: conv2d(inputs, filters, strides=STRIDES, padding="VALID", speed=2)
    my_conv_slow = lambda: conv2d(inputs, filters, strides=STRIDES, padding="VALID", speed=1)

    time_tf = timeit('tf_conv()', setup='gc.enable()', globals=globals(), number=1)
    time_fast = timeit('my_conv_fast()', setup='gc.enable()', globals=globals(), number=1)
    time_medium = timeit('my_conv_medium()', setup='gc.enable()', globals=globals(), number=1) if image_size <= 150 else 'N/A   '
    time_slow = timeit('my_conv_slow()', setup='gc.enable()', globals=globals(), number=1) if image_size <= 20 else 'N/A   '

    print(f' - {image_size}x{image_size}'.ljust(10) + ' | ' + ' | '.join(str(f)[:6] for f in (time_tf, time_fast, time_medium, time_slow)))