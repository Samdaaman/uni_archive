import unittest
from assignment import *

class TestAssignment1(unittest.TestCase):
    def setUp(self):
        model = Model()
        a_one = np.ones((1, 784), dtype=np.float32)  # let's call this a 1
        output = model.call(a_one)
        try:
            model.back_propagation(a_one, output, np.array([[1]]))
            self.label1d = False
        except:
            #expects 1d labels
            self.label1d = True

    def test_call(self):
        model = Model()
        filled_image = np.ones((1,784),dtype=np.float32)
        output = model.call(filled_image)
        self.assertEqual(np.sum(output), 0, "weights and bias should be 0 to start")

    def test_back_propagation(self):
        model = Model()
        a_one = np.ones((1,784),dtype=np.float32) # let's call this a 1
        a_zero = np.zeros((1,784),dtype=np.float32) # let's call this a 0
        output = model.call(a_one)
        # output should still be all zeros
        self.assertEqual(np.sum(output), 0, "weights and bias should be 0 to start")
        # which means argmax would just return 0 which is a 1 which is incorrect
        gradW, gradB = model.back_propagation(a_one, output, np.array([1] if self.label1d else [[1]]))
        # gradients should all be -1 for class 0 and 1 for class 1
        self.assertTrue(np.all(gradW[0,:]==-1))
        self.assertTrue(np.all(gradW[1,:]==1))
        self.assertTrue(np.all(gradB[0,:]==-1))
        self.assertTrue(np.all(gradB[1,:]==1))
        gradW, gradB = model.back_propagation(a_zero, output, np.array([0] if self.label1d else [[0]]))
        # gradients should all be 0 for all classes because weights are 0
        self.assertTrue(np.all(gradW[0,:]==0))
        self.assertTrue(np.all(gradW[1,:]==0))
        self.assertTrue(np.all(gradB[0,:]==0))
        self.assertTrue(np.all(gradB[1,:]==0))

    def test_gradient_descent(self):
        model = Model()
        a_one = np.ones((1,784),dtype=np.float32) # let's call this a 1
        a_zero = np.zeros((1,784),dtype=np.float32) # let's call this a 0
        both = np.append(a_one, a_zero, axis=0)
        output = model.call(both)
        # output should still be all zeros
        self.assertEqual(np.sum(output), 0, "weights and bias should be 0 to start")
        # which means argmax would just return 0 which correct for 0, incorrect for 1,
        # and a correct non-classification for 2:10
        gradW, gradB = model.back_propagation(both, output, np.array([1,0] if self.label1d else [[1, 0]]))
        self.assertTrue(np.all(gradW[0,:]==-0.5))
        self.assertTrue(np.all(gradW[1,:]==0.5))
        self.assertTrue(np.all(gradW[2:10,:]==0))

        model.gradient_descent(gradW, gradB)
        # weights should now all be updated by the gradients * 0.5 learning rate
        self.assertTrue(np.all(model.W[0,:]==-0.25))
        self.assertTrue(np.all(model.W[1,:]==0.25))
        self.assertTrue(np.all(model.W[2:10,:]==0))

if __name__ == '__main__':
    unittest.main()
