from __future__ import absolute_import
from matplotlib import pyplot as plt
import numpy as np
from preprocess import get_data, IMAGE_SIZE


class Model:
    """
    This model class will contain the architecture for
    your single layer Neural Network for classifying MNIST with
    batched learning. Please implement the TODOs for the entire
    model but do not change the method and constructor arguments.
    Make sure that your Model class works with multiple batch
    sizes. Additionally, please exclusively use NumPy and
    Python built-in functions for your implementation.
    """

    def __init__(self):
        # TODO: Initialize all hyperparametrs
        self.input_size = IMAGE_SIZE  # Size of image vectors
        self.num_classes = 10  # Number of classes/possible labels (0-9)
        self.batch_size = 100  # from assignment doc
        self.learning_rate = 0.5  # from assignment doc

        # TODO: Initialize weights and biases
        self.W = np.zeros([10, IMAGE_SIZE*IMAGE_SIZE])
        self.b = np.zeros([10, 1])

    def call(self, inputs):
        """
        Does the forward pass on an batch of input images.
        :param inputs: normalized (0.0 to 1.0) batch of images,
                       (batch_size x 784) (2D), where batch can be any number.
        :return: output, unscaled output values for each class per image # (batch_size x 10)
        """
        # TODO: Write the forward pass logic for your model
        return inputs @ self.W.T + self.b.reshape(1, len(self.b))

    def back_propagation(self, inputs, outputs, labels):
        """
        Returns the gradients for model's weights and biases
        after one forward pass. The learning algorithm for updating weights
        and biases is the Perceptron Learning Algorithm discussed in
        lecture (and described in the assignment writeup). This function should
        handle a batch_size number of inputs by taking the average of the gradients
        across all inputs in the batch.
        :param inputs: batch inputs (a batch of images)
        :param outputs: matrix that contains the unscaled output values of each
        class for each image
        :param labels: true labels
        :return: gradient for weights, and gradient for biases
        """
        # TODO: calculate the gradients for the weights and the gradients for the bias with respect to average loss
        # HINT: np.argmax(outputs, axis=1) will give the index of the largest output
        assert labels.ndim == 1
        num_images = len(inputs)
        classes = np.array(range(self.num_classes)).reshape((self.num_classes, 1))
        guesses = np.argmax(outputs, axis=1)

        incorrect_guesses = (labels != guesses)
        false_positives = incorrect_guesses * (classes == guesses) # incorrect guess and location of guess
        false_negatives = incorrect_guesses * (classes == labels)  # incorrect guess and location of label

        y = false_negatives.astype(int) - false_positives.astype(int)
        gradients_w = y @ inputs / num_images
        gradients_b = y.mean(axis=1).reshape((self.num_classes, 1))
        return gradients_w, gradients_b

    def accuracy(self, outputs, labels):
        """
        Calculates the model's accuracy by comparing the number
        of correct predictions with the correct answers.
        :param outputs: result of running model.call() on test inputs
        :param labels: test set labels
        :return: Float (0,1) that contains batch accuracy
        """
        # TODO: calculate the batch accuracy
        guesses = np.argmax(outputs, axis=1)
        correct_guesses = guesses == labels
        return correct_guesses.mean()

    def gradient_descent(self, gradients_w, gradients_b):
        """
        Given the gradients for outputs weights and biases, does gradient
        descent on the Model's parameters.
        :param gradients_w: gradient for weights
        :param gradients_b: gradient for biases
        :return: None
        """
        # TODO: change the weights and biases of the model to descent the gradient
        self.W += gradients_w * self.learning_rate
        self.b += gradients_b * self.learning_rate


def train(model, train_inputs, train_labels):
    """
    Trains the model on all of the inputs and labels.
    :param model: the initialized model to use for the forward
    pass and backward pass
    :param train_inputs: train inputs (all inputs to use for training)
    :param train_labels: train labels (all labels to use for training)
    :return: None
    """

    # TODO: Iterate over the training inputs and labels, in model.batch_size increments
    for start in range(0, len(train_inputs), model.batch_size):
        inputs = train_inputs[start:start+model.batch_size]
        labels = train_labels[start:start+model.batch_size]

        # TODO: For every batch, compute then descend the gradients for the model's weights
        probabilities = model.call(inputs)
        gradients_w, gradients_b = model.back_propagation(inputs, probabilities, labels)
        model.gradient_descent(gradients_w, gradients_b)


def test(model, test_inputs, test_labels):
    """
    Tests the model on the test inputs and labels. For this assignment,
    the inputs should be the entire test set, but in the future we will
    ask you to batch it instead.
    :param test_inputs: MNIST test data (all images to be tested)
    :param test_labels: MNIST test labels (all corresponding labels)
    :return: accuracy - Float (0,1)
    """

    # TODO: Iterate over the testing inputs and labels
    # TODO: Return accuracy across testing set
    test_outputs = model.call(test_inputs)
    return model.accuracy(test_outputs, test_labels)


def visualize_results(image_inputs, probabilities, image_labels):
    """
    Uses Matplotlib to visualize the results of our model.
    :param image_inputs: image data from get_data()
    :param probabilities: the output of model.call()
    :param image_labels: the labels from get_data()

    NOTE: DO NOT EDIT

    :return: doesn't return anything, a plot should pop-up
    """
    images = np.reshape(image_inputs, (-1, 28, 28))
    predicted_labels = np.argmax(probabilities, axis=1)
    num_images = images.shape[0]

    fig, axs = plt.subplots(ncols=num_images)
    fig.suptitle("PL = Predicted Label\nAL = Actual Label")
    for ind, ax in enumerate(axs):
        ax.imshow(images[ind], cmap="Greys")
        ax.set(title="PL: {}\nAL: {}".format(predicted_labels[ind], image_labels[ind]))
        plt.setp(ax.get_xticklabels(), visible=False)
        plt.setp(ax.get_yticklabels(), visible=False)
        ax.tick_params(axis='both', which='both', length=0)
    plt.show()


def main(mnist_data_folder):
    """
    Read in MNIST data, initialize your model, and train and test your model
    for one epoch. The number of training steps should be your the number of
    batches you run through in a single epoch. You should receive a final accuracy on the testing examples of > 80%.
    :return: None
    """
    # TODO: load MNIST train and test examples into train_inputs, train_labels, test_inputs, test_labels
    train_inputs, train_labels = get_data(f'{mnist_data_folder}/train-images-idx3-ubyte.gz', f'{mnist_data_folder}/train-labels-idx1-ubyte.gz', 60000)
    # TODO: Create Model
    model = Model()

    # TODO: Train model by calling train() ONCE on all data
    train(model, train_inputs, train_labels)

    # TODO: Test the accuracy by calling test() after running train()
    test_inputs, test_labels = get_data(f'{mnist_data_folder}/t10k-images-idx3-ubyte.gz', f'{mnist_data_folder}/t10k-labels-idx1-ubyte.gz', 10000)
    accuracy = test(model, test_inputs, test_labels)
    print(f'Accuracy of model is {accuracy * 100}%')

    # TODO: Visualize the data by using visualize_results()
    num_visual = 10
    visual_inputs = test_inputs[:num_visual]
    visual_outputs = model.call(visual_inputs)
    visual_labels = test_labels[:num_visual]
    visualize_results(visual_inputs, visual_outputs, visual_labels)

    print("end of assignment 1")


if __name__ == '__main__':
    main("COSC440_Assignment1/MNIST_data")
