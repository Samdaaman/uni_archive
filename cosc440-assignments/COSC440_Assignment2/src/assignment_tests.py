import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

import unittest
from assignment import *

class TestAssignment2(unittest.TestCase):
    def assertSequenceEqual(self, it1, it2):
        super().assertSequenceEqual(tuple(it1), tuple(it2))

    def test_same_0(self):
        '''
        Simple test using SAME padding to check out differences between
        own convolution function and TensorFlow's convolution function.

        NOTE: DO NOT EDIT
        '''
        imgs = np.array([[2, 2, 3, 3, 3], [0, 1, 3, 0, 3], [2, 3, 0, 1, 3], [3, 3, 2, 1, 2], [3, 3, 0, 2, 3]],
                        dtype=np.float32)
        imgs = np.reshape(imgs, (1, 5, 5, 1))
        filters = tf.Variable(tf.random.truncated_normal([2, 2, 1, 1],
                                                         dtype=tf.float32,
                                                         stddev=1e-1),
                              name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        self.assertSequenceEqual(my_conv[0][0][0], tf_conv[0][0][0].numpy())

    def test_valid_0(self):
        '''
        Simple test using VALID padding to check out differences between
        own convolution function and TensorFlow's convolution function.

        NOTE: DO NOT EDIT
        '''
        imgs = np.array([[2, 2, 3, 3, 3], [0, 1, 3, 0, 3], [2, 3, 0, 1, 3], [3, 3, 2, 1, 2], [3, 3, 0, 2, 3]],
                        dtype=np.float32)
        imgs = np.reshape(imgs, (1, 5, 5, 1))
        filters = tf.Variable(tf.random.truncated_normal([2, 2, 1, 1],
                                                         dtype=tf.float32,
                                                         stddev=1e-1),
                              name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="VALID")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="VALID")
        self.assertSequenceEqual(my_conv[0][0], tf_conv[0][0].numpy())

    def test_valid_1(self):
        '''
        Simple test using VALID padding to check out differences between
        own convolution function and TensorFlow's convolution function.

        NOTE: DO NOT EDIT
        '''
        imgs = np.array([[3, 5, 3, 3], [5, 1, 4, 5], [2, 5, 0, 1], [3, 3, 2, 1]], dtype=np.float32)
        imgs = np.reshape(imgs, (1, 4, 4, 1))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, 1, 1],
                                                         dtype=tf.float32,
                                                         stddev=1e-1),
                              name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="VALID")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="VALID")
        self.assertSequenceEqual(my_conv[0][0], tf_conv[0][0].numpy())

    def test_valid_2(self):
        '''
        Simple test using VALID padding to check out differences between
        own convolution function and TensorFlow's convolution function.

        NOTE: DO NOT EDIT
        '''
        imgs = np.array([[1, 3, 2, 1], [1, 3, 3, 1], [2, 1, 1, 3], [3, 2, 3, 3]], dtype=np.float32)
        imgs = np.reshape(imgs, (1, 4, 4, 1))
        filters = np.array([[1, 2, 3], [0, 1, 0], [2, 1, 2]]).reshape((3, 3, 1, 1)).astype(np.float32)
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="VALID")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="VALID")
        self.assertSequenceEqual(my_conv[0][0], tf_conv[0][0].numpy())

    def test_loss(self):
        '''
        Simple test to make sure loss function is the average softmax cross-entropy loss

        NOTE: DO NOT EDIT
        '''
        labels = tf.constant([[1.0, 0.0]])
        logits = tf.constant([[1.0, 0.0]])
        self.assertAlmostEqual(loss(logits, labels), 0.31326166)
        logits = tf.constant([[1.0, 0.0], [0.0, 1.0]])
        self.assertAlmostEqual(loss(logits, labels), 0.8132616281509399)

if __name__ == '__main__':
    unittest.main()


