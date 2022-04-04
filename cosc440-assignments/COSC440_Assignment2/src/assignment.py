from __future__ import absolute_import
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
from typing import Literal, Optional, Tuple, Union
from matplotlib import pyplot as plt
from preprocess import get_data
import tensorflow as tf
import numpy as np
import time
from convolution import conv2d


def linear_unit(x, W, b):
  return tf.matmul(x, W) + b

class ModelPart0:
    def __init__(self):
        """
        This model class contains a single layer network similar to Assignment 1.
        """

        self.batch_size = 64
        self.optimizer = tf.keras.optimizers.Adam(learning_rate=0.001)

        input = 32 * 32 * 3
        output = 2

        self.W1 = tf.Variable(tf.random.truncated_normal([input, output], dtype=tf.float32, stddev=0.1), name="W1")
        self.B1 = tf.Variable(tf.random.truncated_normal([1, output], dtype=tf.float32, stddev=0.1), name="B1")

        self.trainable_variables = [self.W1, self.B1]


    def call(self, inputs):
        """
        Runs a forward pass on an input batch of images.
        :param inputs: images, shape of (num_inputs, 32, 32, 3); during training, the shape is (batch_size, 32, 32, 3)
        :return: logits - a matrix of shape (num_inputs, num_classes); during training, it would be (batch_size, 2)
        """
        inputs = np.reshape(inputs, [inputs.shape[0],-1])
        y = linear_unit(inputs, self.W1, self.B1)
        return y


class ModelPart1:
    def __init__(self):
        """
        This model class contains a single layer network similar to Assignment 1.
        """
        self.batch_size = 64
        self.optimizer = tf.keras.optimizers.Adam(learning_rate=0.001)

        input = 32 * 32 * 3
        h = 256
        output = 2

        self.W1 = tf.Variable(tf.random.truncated_normal([input, h], dtype=tf.float32, stddev=0.1), name="W1")
        self.B1 = tf.Variable(tf.random.truncated_normal([1, h], dtype=tf.float32, stddev=0.1), name="B1")
        self.W2 = tf.Variable(tf.random.truncated_normal([h, output], dtype=tf.float32, stddev=0.1), name="W2")
        self.B2 = tf.Variable(tf.random.truncated_normal([1, output], dtype=tf.float32, stddev=0.1), name="B2")

        self.trainable_variables = [self.W1, self.B1, self.W2, self.B2]


    def call(self, inputs):
        """
        Runs a forward pass on an input batch of images.
        :param inputs: images, shape of (num_inputs, 32, 32, 3); during training, the shape is (batch_size, 32, 32, 3)
        :return: logits - a matrix of shape (num_inputs, num_classes); during training, it would be (batch_size, 2)
        """
        inputs = tf.reshape(inputs, [inputs.shape[0],-1])
        y_1 = linear_unit(inputs, self.W1, self.B1)
        y_1_activated = tf.where(y_1 > 0, y_1, 0)
        y_2 = linear_unit(y_1_activated, self.W2, self.B2)
        return y_2


class Conv2DLayer:
    filters: tf.Variable
    input_shape: Tuple[int, int, int]
    _kernel_size: int
    _strides: Tuple[int, int]
    _padding: Literal['VALID', 'SAME']

    def __init__(self, filters: int, kernel_size: int, input_shape: Optional[Tuple[int, int, int]] = None, strides: Tuple[int, int] = (1,1), padding: Literal['VALID', 'SAME'] = 'VALID') -> None:
        global _last_layer_output_shape
        self.input_shape = input_shape if input_shape is not None else _last_layer_output_shape
        self.filters = tf.Variable(tf.random.truncated_normal([kernel_size, kernel_size, self.input_shape[2], filters], dtype=tf.float32, stddev=0.1))
        self._kernel_size = kernel_size
        self._strides = strides
        self._padding = padding

        if self._padding == 'VALID':
            padding = 0
        else:
            if self._strides != (1,1):
                raise NotImplementedError('SAME padding must use strides of 1')
            elif self._kernel_size % 2 != 1:
                raise NotImplementedError('kernel_size must be odd with SAME padding')
            padding = (self._kernel_size - 1) // 2

        self.output_shape = (
            (self.input_shape[0] + 2*padding - self._kernel_size) // self._strides[0] + 1,
            (self.input_shape[1] + 2*padding - self._kernel_size) // self._strides[1] + 1,
            self.filters.shape[3]
        )

        _last_layer_output_shape = self.output_shape

    def __call__(self, inputs: tf.Tensor) -> tf.Tensor:
        y = tf.nn.conv2d(inputs, self.filters, self._strides, self._padding)
        return tf.where(y > 0, y, 0) # apply relu activation


class MaxPool2DLayer:
    input_shape: Tuple[int, int, int]
    _pool_size: Tuple[int, int]
    _strides: Tuple[int, int]
    _padding: Literal['VALID', 'SAME']

    def __init__(self, pool_size: Tuple[int, int], input_shape: Optional[Tuple[int, int, int]] = None, strides: Optional[Tuple[int, int]] = None, padding: Literal['VALID', 'SAME'] = 'VALID'):
        global _last_layer_output_shape
        self._pool_size = pool_size
        self.input_shape = input_shape if input_shape is not None else _last_layer_output_shape
        self._strides = strides if strides is not None else pool_size
        self._padding = padding

        if self._padding == 'VALID':
            self.output_shape = (
                (self.input_shape[0] - self._pool_size[0]) // self._strides[0] + 1,
                (self.input_shape[1] - self._pool_size[1]) // self._strides[1] + 1,
                self.input_shape[2]
            )
        else:
            raise NotImplementedError()

        _last_layer_output_shape = self.output_shape

    def __call__(self, inputs: tf.Tensor) -> tf.Tensor:
        return tf.nn.max_pool2d(inputs, self._pool_size, self._strides, self._padding)


class LinearLayer:
    input_size: int
    output_size: int
    w: tf.Variable
    b: tf.Variable
    _activation: bool

    def __init__(self, output_size: int, activation: bool = False):
        global _last_layer_output_shape
        self.input_size = np.prod(_last_layer_output_shape)
        _last_layer_output_shape = output_size
        self.output_size = output_size
        self.w = tf.Variable(tf.random.truncated_normal([self.input_size, output_size], dtype=tf.float32, stddev=0.1))
        self.b = tf.Variable(tf.random.truncated_normal([1, output_size], dtype=tf.float32, stddev=0.1))
        self._activation = activation

    def __call__(self, inputs: tf.Tensor) -> tf.Tensor:
        if inputs.shape[1:] != (self.input_size,):
            if np.prod(inputs.shape[1:]) == self.input_size:
                # Automatically flatten input (ie if it came from a convolution or max_pool layer)
                inputs = tf.reshape(inputs, (inputs.shape[0], -1))
            else:
                raise Exception(f"Can't feed {inputs.shape[1:]} into linear unit with input_size {self.input_size}")
        
        y = inputs @ self.w + self.b
        if self._activation:
            return tf.where(y > 0, y, 0)
        else:
            return y
        

_last_layer_output_shape = None

class ModelPart3:
    def __init__(self):
        """
        This model class contains Convolutional Neural Network.
        """
        self.batch_size = 200
        self.num_classes = 2
        self.optimizer = tf.keras.optimizers.Adam(learning_rate=0.001)

        self.layers = (
            Conv2DLayer(filters=3**2, kernel_size=3, input_shape=(32, 32, 3)),
            MaxPool2DLayer(pool_size=(2,2)),
            Conv2DLayer(filters=3**3, kernel_size=3),
            MaxPool2DLayer(pool_size=(2,2)),
            Conv2DLayer(filters=3**4, kernel_size=3),
            LinearLayer(output_size=256, activation=True),
            LinearLayer(output_size=2)

            # Conv2DLayer(filters=3*4, kernel_size=5, input_shape=(32, 32, 3), padding='SAME'),
            # # Conv2DLayer(filters=3*4, kernel_size=5),
            # MaxPool2DLayer(pool_size=(2,2)),
            # # Conv2DLayer(filters=3*8, kernel_size=5, padding='SAME'),
            # Conv2DLayer(filters=3*8, kernel_size=5, padding='SAME'),
            # MaxPool2DLayer(pool_size=(2,2)),
            # # Conv2DLayer(filters=3*16, kernel_size=5, padding='SAME'),
            # Conv2DLayer(filters=3*16, kernel_size=5, padding='SAME'),
            # LinearLayer(output_size=3072, activation=True),
            # LinearLayer(output_size=3072, activation=True),
            # LinearLayer(output_size=1000, activation=True),
            # LinearLayer(output_size=2)

            # Conv2DLayer(filters=8, kernel_size=3, input_shape=(32, 32, 3), padding='SAME'),
            # MaxPool2DLayer(pool_size=(2,2)),
            # Conv2DLayer(filters=16, kernel_size=3, padding='SAME'),
            # MaxPool2DLayer(pool_size=(2,2)),
            # Conv2DLayer(filters=32, kernel_size=3, padding='SAME'),
            # MaxPool2DLayer(pool_size=(2,2)),
            # Conv2DLayer(filters=64, kernel_size=3, padding='SAME'),
            # Conv2DLayer(filters=3**4, kernel_size=3, padding='SAME'),
            # MaxPool2DLayer(pool_size=(2,2)),
            # LinearLayer(output_size=100 , activation=True),
            # LinearLayer(output_size=1728, activation=True),
            # LinearLayer(output_size=2048, activation=True),
            # LinearLayer(output_size=2)
        )
        
        self.trainable_variables = []
        for layer in self.layers:
            if isinstance(layer, Conv2DLayer):
                self.trainable_variables.append(layer.filters)
            elif isinstance(layer, LinearLayer):
                self.trainable_variables.append(layer.w)
                self.trainable_variables.append(layer.b)

        print("-"*70)
        print(f'  NUM  |   TYPE    |  INPUT   |  OUTPUT  | TRAINABLE PARAMETERS')
        print("-"*70)

        for i, layer in enumerate(self.layers):
            if isinstance(layer, Conv2DLayer):
                print(f'  #{str(i + 1).ljust(2)}  | Conv2d    | {"x".join(str(x) for x in layer.input_shape).ljust(8)} | {"x".join(str(x) for x in layer.output_shape).ljust(8)} | {np.prod(layer.filters.shape)} ({"x".join(str(x) for x in layer.filters.shape)} filter)')
            
            elif isinstance(layer, MaxPool2DLayer):
                print(f'  #{str(i + 1).ljust(2)}  | MaxPool2d | {"x".join(str(x) for x in layer.input_shape).ljust(8)} | {"x".join(str(x) for x in layer.output_shape).ljust(8)} | {"0".ljust(20)}')
            
            elif isinstance(layer, LinearLayer):
                print(f'  #{str(i + 1).ljust(2)}  | Linear    | {str(layer.input_size).ljust(8)} | {str(layer.output_size).ljust(8)} | {(layer.input_size + 1) * layer.output_size} ({layer.input_size}x{layer.output_size} + 1x{layer.output_size})')

        num_trainable_variables = 0
        for trainable_variable in self.trainable_variables:
            num_trainable_variables += np.prod(trainable_variable.shape)
        print(f'\nTotal trainable variables is {num_trainable_variables}')

    def call(self, inputs):
        """
        Runs a forward pass on an input batch of images.
        :param inputs: images, shape of (num_inputs, 32, 32, 3); during training, the shape is (batch_size, 32, 32, 3)
        :return: logits - a matrix of shape (num_inputs, num_classes); during training, it would be (batch_size, 2)
        """
        for layer in self.layers:
            inputs = layer(inputs)
        return inputs


def loss(logits, labels):
    """
    Calculates the cross-entropy loss after one forward pass.
    :param logits: during training, a matrix of shape (batch_size, self.num_classes)
    containing the result of multiple convolution and feed forward layers
    Softmax is applied in this function.
    :param labels: during training, matrix of shape (batch_size, self.num_classes) containing the train labels
    :return: the loss of the model as a Tensor
    """
    return tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(labels, logits))


def accuracy(logits, labels):
    """
    Calculates the model's prediction accuracy by comparing
    logits to correct labels – no need to modify this.
    :param logits: a matrix of size (num_inputs, self.num_classes); during training, this will be (batch_size, self.num_classes)
    containing the result of multiple convolution and feed forward layers
    :param labels: matrix of size (num_labels, self.num_classes) containing the answers, during training, this will be (batch_size, self.num_classes)

    NOTE: DO NOT EDIT

    :return: the accuracy of the model as a Tensor
    """
    correct_predictions = tf.equal(tf.argmax(logits, 1), tf.argmax(labels, 1))
    return tf.reduce_mean(tf.cast(correct_predictions, tf.float32))

def train(model: Union[ModelPart0, ModelPart1, ModelPart3], train_inputs, train_labels):
    '''
    Trains the model on all of the inputs and labels for one epoch. You should shuffle your inputs
    and labels - ensure that they are shuffled in the same order using tf.gather.
    You should batch your inputs.
    :param model: the initialized model to use for the forward pass and backward pass
    :param train_inputs: train inputs (all inputs to use for training),
    shape (num_inputs, width, height, num_channels)
    :param train_labels: train labels (all labels to use for training),
    shape (num_labels, num_classes)
    :return: None
    '''
    start_time = time.perf_counter()
    for epoch_i in range(NUM_EPOCHS):
        print(f'\rEpoch: #{epoch_i+1}/{NUM_EPOCHS}  t={int(time.perf_counter() - start_time)}s    ', end='', flush=True)

        # Run model.call with batches of images (adapted from assignment 1 code)
        for start in range(0, len(train_inputs), model.batch_size):
            batch_inputs = train_inputs[start:start+model.batch_size]
            batch_labels = train_labels[start:start+model.batch_size]


            # code adapted from  https://colab.research.google.com/drive/1Gco5Ng8H2bnXZhp1QKnRuGWru08uErQW
            with tf.GradientTape() as tape:
                y = model.call(batch_inputs)
                batch_loss = loss(y, batch_labels)
                
            gradients = tape.gradient(batch_loss, model.trainable_variables)
            model.optimizer.apply_gradients(zip(gradients, model.trainable_variables)) 
            print('\x08\x08\x08-<=>', end='', flush=True)

        print('\x08\x08\x08---', end='', flush=True)

        # shuffle inputs and labels
        indices = tf.random.shuffle(range(len(train_inputs)))
        train_inputs = tf.gather(train_inputs, indices)
        train_labels = tf.gather(train_labels, indices)
    print(f'\rDone {NUM_EPOCHS} epochs in {int((time.perf_counter() - start_time))} seconds')


def test(model, test_inputs, test_labels):
    """
    Tests the model on the test inputs and labels.
    :param test_inputs: test data (all images to be tested),
    shape (num_inputs, width, height, num_channels)
    :param test_labels: test labels (all corresponding labels),
    shape (num_labels, num_classes)
    :return: test accuracy - this can be the average accuracy across
    all batches or the sum as long as you eventually divide it by batch_size
    """
    return accuracy(model.call(test_inputs), test_labels)


def visualize_results(image_inputs, probabilities, image_labels, first_label, second_label):
    """
    Uses Matplotlib to visualize the results of our model.
    :param image_inputs: image data from get_data(), limited to 10 images, shape (10, 32, 32, 3)
    :param probabilities: the output of model.call(), shape (10, num_classes)
    :param image_labels: the labels from get_data(), shape (10, num_classes)
    :param first_label: the name of the first class, "dog"
    :param second_label: the name of the second class, "cat"

    NOTE: DO NOT EDIT

    :return: doesn't return anything, a plot should pop-up
    """
    predicted_labels = np.argmax(probabilities, axis=1)
    num_images = image_inputs.shape[0]

    fig, axs = plt.subplots(ncols=num_images)
    fig.suptitle("PL = Predicted Label\nAL = Actual Label")
    for ind, ax in enumerate(axs):
            ax.imshow(image_inputs[ind], cmap="Greys")
            pl = first_label if predicted_labels[ind] == 0.0 else second_label
            al = first_label if np.argmax(image_labels[ind], axis=0) == 0 else second_label
            ax.set(title="PL: {}\nAL: {}".format(pl, al))
            plt.setp(ax.get_xticklabels(), visible=False)
            plt.setp(ax.get_yticklabels(), visible=False)
            ax.tick_params(axis='both', which='both', length=0)
    plt.show()


CLASS_CAT = 3
CLASS_DOG = 5
NUM_EPOCHS = 25

def main(cifar_data_folder):
    '''
    Read in CIFAR10 data (limited to 2 classes), initialize your model, and train and
    test your model for a number of epochs. We recommend that you train for
    25 epochs.
    You should receive a final accuracy on the testing examples for cat and dog
    of ~60% for Part1 and ~70% for Part3.
    :return: None
    '''
    train_inputs, train_labels = get_data(os.path.join(cifar_data_folder, 'train'), CLASS_CAT, CLASS_DOG)

    print(f'Loaded {len(train_inputs)} images')
    # model = ModelPart0()
    # model = ModelPart1()
    model = ModelPart3()

    print(f'{"-" * 50}Training{"-" * 50}')
    train(model, train_inputs, train_labels)
    
    print(f'{"-" * 50}Testing{"-" * 50}')
    test_inputs, test_labels = get_data(os.path.join(cifar_data_folder, 'test'), CLASS_CAT, CLASS_DOG)
    test_accuracy = test(model, test_inputs, test_labels)
    print(f'Test accuracy is {test_accuracy * 100.0}%')

    print(f'{"-" * 50}Visualising{"-" * 50}')
    visual_inputs = test_inputs[:10]
    visual_probabilities = model.call(visual_inputs)
    visual_labels = test_labels[:len(visual_inputs)]
    visualize_results(visual_inputs, visual_probabilities, visual_labels, "Cat", "Dog")


if __name__ == '__main__':
    # local_home = os.path.expanduser("~")  # on my system this is /Users/jat171
    # cifar_data_folder = local_home + '/CIFAR_data/'
    cifar_data_folder = os.path.join(os.path.dirname(__file__), 'CIFAR_data') # find CIFAR_data folder in same dir as script
    main(cifar_data_folder)
