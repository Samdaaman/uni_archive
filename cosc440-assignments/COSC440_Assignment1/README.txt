# Answer the conceptual questions here
## Q1: Is there anything we need to know to get your code to work? If you did not get your code working or handed in an incomplete solution please let us know what you did complete (0-4 sentences)
Nah should work first time. Note, I had to change the directory which points to the data in the main function

## Q2: Why do we normalize our pixel values between 0-1? (1-3 sentences)
Normalising allows the deep learning algorithm to train itself based of the differences in the data (rather than the differences in the range of the data). It does this by forcing all of the data into a common scale (usually 0-1).

## Q3: Why do we use a bias vector in our forward pass? (1-3 sentences)
A bias vector increases the flexibility of any neural network model and allows it to more accurately fit the data. It allows the network to fit data when all of it's inputs are 0 and can also add a "threshold" value to each perceptron.

## Q4: Why do we separate the functions for the gradient descent update from the calculation of the gradient in back propagation? (2-4 sentences)
So the different functions can be tested independently by a uniitest without updating the model. It also allows us to adjust the learning rate easily if we are using mulitple epochs

## Q5: What are some qualities of MNIST that make it a “good” dataset for a classification problem? (2-3 sentences)
The images are all of the same size and only have one colour present. This simplifies the preprocessing significantly. It appears that all of the input images are valid characters so there is no invalid input to skew the model. There is also great variety of different handwriting styles and it is a large dataset (10s of thousands) which makes it very useful to train a neural network

## Q6: Suppose you are an administrator of the US Postal Service in the 1990s. What positive and/or negative effects would result from deploying an MNIST-trained neural network to recognize handwritten zip codes on mail? (2-4 sentences)
- One of the positive impacts would be a great increase in speed. The model can train itself and classify 10,000 images in mere seconds (a task that would take humans hours).
- One of the negative impacts is the fact that the model might be unsuitable for deployment in the postal service. A vast amount of preprocessing work (such as normalising position and colour) is needed before this model is effective.
- Also a negative, people's handwriting in the 1990s might differ from that in the models MNIST dataset so the model might not be as good as it is tested to be (as it is tested from a similar MNIST dataset).
