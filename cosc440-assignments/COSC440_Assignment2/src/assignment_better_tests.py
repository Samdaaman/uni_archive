import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

import tensorflow as tf
import unittest
from assignment import *

class TestAssignment2(unittest.TestCase):
    def assertSequenceEqual(self, it1, it2):
        super().assertSequenceEqual(tuple(it1), tuple(it2))

    # ignore_extra_dims_on_mat2, is used to compare nxm by ixj where i>n and j>m.
    # It only compares mat1 (nxm) to the top left nxm corner of mat2 (ixj)
    def assertMatrixEqual(self, mat1: np.ndarray, mat2: np.ndarray, ignore_extra_dims_on_mat2=False):
        if not (mat1.ndim == 2 and mat2.ndim == 2):
            raise AssertionError(f'mat1.ndim ({mat1.ndim}) != mat2.ndim ({mat2.ndim})')
        if not ignore_extra_dims_on_mat2 and not mat1.shape == mat2.shape:
            raise AssertionError(f'mat1.shape ({mat1.shape}) != mat2.shape ({mat2.shape})')
        try:
            for i in range(mat1.shape[0]):
                for j in range(mat1.shape[1]):
                    self.assertAlmostEqual(mat1[i][j], mat2[i][j], places=5)
        except AssertionError as ex:
            print(f'mat1:\n{mat1}')
            print(f'\nmat2:\n{mat2}')
            raise ex

    def test_custom_matrix_equal(self):
        self.assertMatrixEqual(
            np.arange(9).reshape((3,3)),
            np.arange(9).reshape((3,3))
        )

    def test_custom_same0(self):
        imgs = np.array([[3, 5, 3, 3], [5, 1, 4, 5], [2, 5, 0, 1], [3, 3, 2, 1]], dtype=np.float32)
        imgs = np.reshape(imgs, (1, 4, 4, 1))
        filters = np.eye(3, 3).reshape((3, 3, 1, 1))
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())


    def test_custom_same1(self):
        imgs = np.array([[3, 5, 3, 3], [5, 1, 4, 5], [2, 5, 0, 1], [3, 3, 2, 1]], dtype=np.float32)
        imgs = np.reshape(imgs, (1, 4, 4, 1))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, 1, 1], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())


    def test_custom_2_image(self):
        img1 = np.array([[2, 2, 3, 3], [0, 1, 3, 0], [2, 3, 0, 1], [3, 3, 2, 1]], dtype=np.float32)
        img2 = np.array([[3, 5, 3, 3], [5, 1, 4, 5], [2, 5, 0, 1], [3, 3, 2, 1]], dtype=np.float32)
        imgs = np.array([img1, img2], dtype=np.float32)
        imgs = np.reshape(imgs, (2, 4, 4, 1))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, 1, 1], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())
        self.assertMatrixEqual(my_conv[1,:,:,0], tf_conv[1,:,:,0].numpy())

    def test_custom_multi_image(self):
        imgs = np.random.randint(5, size=(10, 4, 4, 1))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, 1, 1], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        for i in range(imgs.shape[0]):
            self.assertMatrixEqual(my_conv[i,:,:,0], tf_conv[i,:,:,0].numpy())

    def test_custom_multi_input_channel(self):
        imgs = np.random.randint(5, size=(1, 4, 4, 10)) # 10 channels coz why not
        filters = tf.Variable(tf.random.truncated_normal([3, 3, imgs.shape[3], 1], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        assert my_conv.shape == tf_conv.shape
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())

    def test_custom_multi_output_channel(self):
        imgs = np.random.randint(5, size=(1, 4, 4, 1))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, 1, 10], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        assert my_conv.shape == tf_conv.shape
        for i in range(filters.shape[3]):
            self.assertMatrixEqual(my_conv[0,:,:,i], tf_conv[0,:,:,i].numpy())

    def test_custom_multi_channel_and_image_same(self):
        imgs = np.random.randint(5, size=(10, 4, 4, 3))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, imgs.shape[3], 5], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="SAME")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="SAME")
        assert my_conv.shape == tf_conv.shape
        for i in range(len(imgs)):
            for j in range(filters.shape[3]):
                self.assertMatrixEqual(my_conv[i,:,:,j], tf_conv[i,:,:,j].numpy())

    def test_custom_multi_channel_and_image_valid(self):
        imgs = np.random.randint(5, size=(10, 4, 4, 3))
        filters = tf.Variable(tf.random.truncated_normal([3, 3, imgs.shape[3], 5], dtype=tf.float32, stddev=1e-1), name="filters")
        my_conv = conv2d(imgs, filters, strides=[1, 1, 1, 1], padding="VALID")
        tf_conv = tf.nn.conv2d(imgs, filters, [1, 1, 1, 1], padding="VALID")
        assert my_conv.shape == tf_conv.shape
        for i in range(len(imgs)):
            for j in range(filters.shape[3]):
                self.assertMatrixEqual(my_conv[i,:,:,j], tf_conv[i,:,:,j].numpy())

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
        # self.assertSequenceEqual(my_conv[0][0][0], tf_conv[0][0][0].numpy())
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy(), ignore_extra_dims_on_mat2=True)

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
        # self.assertSequenceEqual(my_conv[0][0], tf_conv[0][0].numpy())
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())


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
        # self.assertSequenceEqual(my_conv[0][0], tf_conv[0][0].numpy())
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())


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
        # self.assertSequenceEqual(my_conv[0][0], tf_conv[0][0].numpy())
        self.assertMatrixEqual(my_conv[0,:,:,0], tf_conv[0,:,:,0].numpy())


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


