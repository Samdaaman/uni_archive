import gzip
import numpy as np

IMAGE_SIZE = 28


def get_data(inputs_file_path, labels_file_path, num_examples):
    """
    Takes in an inputs file path and labels file path, unzips both files,
    normalizes the inputs, and returns (NumPy array of inputs, NumPy
    array of labels). Read the data of the file into a buffer and use
    np.frombuffer to turn the data into a NumPy array. Keep in mind that
    each file has a header of a certain size. This method should be called
    within the main function of the assignment.py file to get BOTH the train and
    test data. If you change this method and/or write up separate methods for
    both train and test data, we will deduct points.

    Hint: look at the writeup for sample code on using the gzip library

    :param inputs_file_path: file path for inputs, something like
    'MNIST_data/t10k-images-idx3-ubyte.gz'
    :param labels_file_path: file path for labels, something like
    'MNIST_data/t10k-labels-idx1-ubyte.gz'
    :param num_examples: used to read from the bytestream into a buffer. Rather
    than hardcoding a number to read from the bytestream, keep in mind that each image
    (example) is 28 * 28, with a header of a certain number.
    :return: NumPy array of inputs as float32 and labels as int8
    """

    with gzip.open(inputs_file_path) as input_file:
        input_file.read(16)  # clear the header
        buffer = input_file.read(IMAGE_SIZE * IMAGE_SIZE * num_examples)
        input_data_flat = np.frombuffer(buffer, dtype=np.uint8)
        input_data_non_normalised = input_data_flat.reshape(num_examples, IMAGE_SIZE * IMAGE_SIZE)
        input_data = input_data_non_normalised.astype(np.float32)

    with gzip.open(labels_file_path) as label_file:
        label_file.read(8)
        buffer = label_file.read(num_examples)
        label_data = np.frombuffer(buffer, dtype=np.uint8).astype(np.uint64)

    return input_data, label_data

# Code for testing import
# if __name__ == '__main__':
#     input_data, input_labels = get_data('MNIST_data/train-images-idx3-ubyte.gz', 'MNIST_data/train-labels-idx1-ubyte.gz', 10)
#     print(input_labels)
#     import matplotlib.pyplot as plt
#
#     image = np.asarray(input_data[5]).squeeze()
#     plt.imshow(image)
#     plt.show()
