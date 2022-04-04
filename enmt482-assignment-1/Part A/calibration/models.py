from abc import ABC, abstractmethod
import numpy as np
from bisect import bisect
from itertools import permutations
from typing import List, Optional

# Definition of meta-class
class Model(ABC):
    k: np.ndarray = None

    def __init__(self, k: Optional[np.ndarray] = None) -> None:
        if k is None:
            self.k = np.zeros(self.k_len())
        else:
            self.k = k

    @abstractmethod
    def k_len():
        pass

    @abstractmethod
    def h(self, x_array: np.ndarray):
        pass

    @abstractmethod
    def get_jacobian(self, x: float):
        pass

    def nonlinear_least_squares_fit(self, x_array: np.ndarray, distance_array: np.ndarray, iterations=50):
        N = len(x_array)
        self.k = np.zeros((self.k_len(),)) # preallocated k array (same size as jacobian)
        A = np.empty((len(x_array), len(self.k)))

        for _ in range(iterations):
            # Calculate Jacobians for current estimate of parameters.
            for i, x in enumerate(x_array):
                A[i] = self.get_jacobian(x)
                
            # Use least squares to estimate the parameters.
            deltak, res, rank, s = np.linalg.lstsq(A, distance_array - self.h(x_array), rcond=None)
            self.k += deltak

        # error = abs(self.get_distance(x_array) - distance_array)
        # not_outliers = np.where(error < 0.15, 1, 0)
        # x_array = x_array.compress(not_outliers)
        # distance_array = distance_array.compress(not_outliers)

        # self.k = np.zeros((self.k_len(),)) # preallocated k array (same size as jacobian)
        # A = np.empty((len(x_array), len(self.k)))
        # for _ in range(iterations):
        #     # Calculate Jacobians for current estimate of parameters.
        #     for i, x in enumerate(x_array):
        #         A[i] = self.get_jacobian(x)
                
        #     # Use least squares to estimate the parameters.
        #     deltak, res, rank, s = np.linalg.lstsq(A, distance_array - self.get_distance(x_array), rcond=None)
        #     self.k += deltak
    
class IRBasicModel(Model):
    def k_len(self):
        return 3

    def h(self, x_array: np.ndarray):
        return self.k[0] + self.k[1] / (x_array + self.k[2])

    def get_jacobian(self, x: float):
        return np.array((
            1,
            1 / (x + self.k[2]),
            -self.k[1] / (x + self.k[2])**2
        ))


class IRBetterModel(Model):
    def k_len(self):
        return 4

    def h(self, x_array: np.ndarray):
        return sum((
            self.k[0],
            self.k[1] * x_array,
            self.k[-2] / (x_array + self.k[-1])
        ))

    def get_jacobian(self, x: float):
        return np.array((
            1,
            x,
            1 / (x + self.k[-1]),
            -self.k[-2] / (x + self.k[-1])**2
        ))


class IREvenBetterModel(Model):
    def k_len(self):
        return 5

    def h(self, x_array: np.ndarray):
        return sum((
            self.k[0],
            self.k[1] * x_array,
            self.k[2] * x_array**2,
            self.k[-2] / (x_array + self.k[-1])
        ))

    def get_jacobian(self, x: float):
        return np.array((
            1,
            x,
            x**2,
            1 / (x + self.k[-1]),
            -self.k[-2] / (x + self.k[-1])**2
        ))

# Unused
# class IRTwoPartModel(Model):
#     split_x_value: float

#     def k_len(self):
#         raise NotImplementedError()

#     def get_distance(self, x_array: np.ndarray):
#         return np.where(x_array < self.split_x_value,
#             self.k[0] + self.k[1] * x_array + self.k[2] / (x_array + self.k[3]),
#             self.k[4] + self.k[5] * x_array + self.k[6] / (x_array + self.k[7])
#         )

#     def get_jacobian(self, x: float):
#         if x < self.split_x_value:
#             return np.array((
#                 1,
#                 x,
#                 1 / (x + self.k[3]),
#                 -self.k[2] / (x + self.k[3])**2
#             ))
#         else:
#             return np.array((
#                 1,
#                 x,
#                 1 / (x + self.k[7]),
#                 -self.k[6] / (x + self.k[7])**2
#             ))

#     def nonlinear_least_squares_fit(self, x_array, distance_array, iterations=1):
#         best_k = None
#         lowest_error_2_norm = np.inf

#         # for split_index in range(1, len(x_array)):
#         for split_index in [507]:
#             x_array_head = x_array[:split_index]
#             x_array_tail = x_array[split_index:]
#             distance_array_head = distance_array[:split_index]
#             distance_array_tail = distance_array[split_index:]

#             self.split_x_value = x_array[split_index]

#             model_head = IRBetterModel()
#             model_head.nonlinear_least_squares_fit(x_array_head, distance_array_head, iterations)
#             fitted_head = model_head.get_distance(x_array_head)
#             model_tail = IRBetterModel()
#             model_tail.nonlinear_least_squares_fit(x_array_tail, distance_array_tail, iterations)
#             fitted_tail = model_tail.get_distance(x_array_tail)
            
#             error = x_array - np.concatenate((fitted_head, fitted_tail))
#             error_2_norm = np.linalg.norm(error, ord=2)

#             if error_2_norm < lowest_error_2_norm:
#                 lowest_error_2_norm = error_2_norm
#                 best_k = np.concatenate((model_head.k, model_tail.k))
#                 print(f'{split_index} / {len(x_array)}')

#         self.k = best_k

class IRTThreePartModel(Model):
    split_x_values: np.ndarray
    split_x_value_guesses: List[float]
    split_x_value_threshold: float

    def __init__(self, split_x_value_guesses: List[float], split_x_value_threshold: float) -> None:
        self.split_x_value_guesses = split_x_value_guesses
        self.split_x_value_threshold = split_x_value_threshold

    def k_len(self):
        raise NotImplementedError()

    def h(self, x_array: np.ndarray):
        def h_one(x: float):
            k_index = bisect(self.split_x_values, x)
            return sum((
                self.k[k_index][0],
                self.k[k_index][1] * x,
                self.k[k_index][2] * x**2,
                self.k[k_index][-2] / (x + self.k[k_index][-1])
            ))
        return np.array([h_one(x) for x in x_array])
        
    def get_jacobian(self, x: float):
        raise NotImplementedError()

    def nonlinear_least_squares_fit(self, x_array, distance_array, iterations=5):
        split_index_ranges = tuple(range(
            bisect(x_array, split_x_value_guess - self.split_x_value_threshold),
            bisect(x_array, split_x_value_guess + self.split_x_value_threshold),
        ) for split_x_value_guess in self.split_x_value_guesses)

        lowest_error_2_norm = np.inf

        print(f'calculating best split out of {len(split_index_ranges[0]) * len(split_index_ranges[1])} options')
        print(f'split is between ({x_array[split_index_ranges[0][0]]},{x_array[split_index_ranges[0][-1]]}) and ({x_array[split_index_ranges[1][0]]},{x_array[split_index_ranges[1][-1]]})')

        for split_index0 in split_index_ranges[0]:
            for split_index1 in split_index_ranges[1]:
                x_array_head = x_array[:split_index0]
                x_array_middle = x_array[split_index0: split_index1]
                x_array_tail = x_array[split_index1:]

                distance_array_head = distance_array[:split_index0]
                distance_array_middle = distance_array[split_index0: split_index1]
                distance_array_tail = distance_array[split_index1:]

                model_head = IREvenBetterModel()
                model_head.nonlinear_least_squares_fit(x_array_head, distance_array_head, iterations)
                fitted_head = model_head.h(x_array_head)
                model_middle = IREvenBetterModel()
                model_middle.nonlinear_least_squares_fit(x_array_middle, distance_array_middle, iterations)
                fitted_middle = model_middle.h(x_array_middle)
                model_tail = IREvenBetterModel()
                model_tail.nonlinear_least_squares_fit(x_array_tail, distance_array_tail, iterations)
                fitted_tail = model_tail.h(x_array_tail)
                
                error = x_array - np.concatenate((fitted_head, fitted_middle, fitted_tail))
                error_2_norm = np.linalg.norm(error, ord=2)

                if error_2_norm < lowest_error_2_norm:
                    lowest_error_2_norm = error_2_norm
                    self.k = (
                        model_head.k,
                        model_middle.k,
                        model_tail.k
                    )
                    self.split_x_values = [
                        x_array[split_index0],
                        x_array[split_index1]
                    ]

        print(f'best split is {self.split_x_values[0]} and {self.split_x_values[1]}')