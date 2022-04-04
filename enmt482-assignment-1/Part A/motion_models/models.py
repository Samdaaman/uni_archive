from abc import ABC, abstractmethod
from typing import Iterable, Optional

import numpy as np
from numpy.lib.shape_base import split
from scipy.optimize import minimize, fsolve


class SensorModel(ABC):
    def __init__(self, h: Iterable[float]) -> None:
        self._h = tuple(h)

    @abstractmethod
    def h(self, x_array: np.ndarray):
        """
        Model function returns Z = h(x)
        
        Parameters:
            x_array: np.array (or list/tuple)
        """
        pass

    @abstractmethod
    def h_inv(self, z_array: np.ndarray, guesses: Optional[np.ndarray] = None):
        """
        Inverse of model function returns the x which statifies Z = h(x)
        
        Parameters:
            z_array: np.array (or list/tuple)
            guesses: Optiona[np.array] (or list/tuple) of the best guesses for x (used if multiple values for x are possible due to ambiguities in the sensor model)
        """
        pass


class IRBetterModel(SensorModel):
    def __init__(self, h: Iterable[float]) -> None:
        super().__init__(h)
        self._smallest_distance = minimize(self.h, 1).x

    def h(self, x_array: np.ndarray):
        """
        Uses Z = h(x) where h(x) = h_0 + h_1*x + h_2 / (x + h_3)
        """
        return sum((
            self._h[0],
            self._h[1] * x_array,
            self._h[2] / (x_array + self._h[3])
        ))

    def h_inv(self, z_array: np.ndarray, guesses: np.ndarray):
        """
        Uses the below equation to invert Z = h(x). Guesses array is required as it is a quadratic (two roots)

        (h_1)*x^2  +  (h_0 + h_1*h_3 - Z)*x  +  (h_0*h_3 + h_2 - Z*h_3)  =  0
        """
        z_roots_array = [np.roots([self._h[1], self._h[0] + self._h[1]*self._h[3] - z, self._h[0]*self._h[3] + self._h[2] - z*self._h[3]]) for z in z_array]
        x_array = np.empty((len(z_array)))
        
        for i, z_roots in enumerate(z_roots_array):
            if len(z_roots) == 1:
                x_array[i] = z_roots[0]
            elif np.iscomplex(z_roots[0]):
                x_array[i] = self._smallest_distance
            else:
                x_array[i] = z_roots[0] if abs(guesses[i] - z_roots[0]) < abs(guesses[i] - z_roots[1]) else z_roots[1]

        return x_array


class IRThreePartModel(SensorModel):
    def __init__(self, h: Iterable[float], splits: Iterable[float]) -> None:
        super().__init__(h)
        self._splits = tuple(splits)

    def _h_one(self, x: float):
        """
        Uses Z = h(x) where h(x) = h_0 + h_1*x + h_2*x^2 + h_4 / (x + h_5)
        """
        # select which parameters to use for each point (h_0-4, h_5-9 or h_10-14)
        if x < self._splits[0]:
            h_selected = self._h[0:5]
        elif x < self._splits[1]:
            h_selected = self._h[5:10]
        else:
            h_selected = self._h[10:15]

        return sum((
            h_selected[0],
            h_selected[1] * x,
            h_selected[2] * x ** 2,
            h_selected[3] / (x + h_selected[4])
        ))


    def h(self, x_array: np.ndarray):
        """
        Uses Z = h(x) where h(x) = h_0 + h_1*x + h_2*x^2 + h_4 / (x + h_5)
        """
        return np.array([self._h_one(x) for x in x_array])

    def h_inv(self, z_array: np.ndarray, guesses: np.ndarray):
        """
        Does some magic
        """
        # z_roots_array = [np.roots([self._h[1], self._h[0] + self._h[1]*self._h[3] - z, self._h[0]*self._h[3] + self._h[2] - z*self._h[3]]) for z in z_array]
        # x_array = np.empty((len(z_array)))
        
        # for i, z_roots in enumerate(z_roots_array):
        #     if len(z_roots) == 1:
        #         x_array[i] = z_roots[0]
        #     elif np.iscomplex(z_roots[0]):
        #         x_array[i] = self._smallest_distance
        #     else:
        #         x_array[i] = z_roots[0] if abs(guesses[i] - z_roots[0]) < abs(guesses[i] - z_roots[1]) else z_roots[1]

        # return x_array

        def h_with_selected(x: float, h_selected: np.ndarray):
            return sum((
                h_selected[0],
                h_selected[1] * x,
                h_selected[2] * x ** 2,
                h_selected[3] / (x + h_selected[4])
            ))

        x_array = np.empty(len(z_array))
        for i, (z, guess) in enumerate(zip(z_array, guesses)):
            fsolve_guesses = [
                fsolve(lambda x: z - h_with_selected(x, self._h[0:5]), guess),
                fsolve(lambda x: z - h_with_selected(x, self._h[5:10]), guess),
                fsolve(lambda x: z - h_with_selected(x, self._h[10:15]), guess)
            ]
            best_guess_index = np.argmin(abs(guess - fsolve_guesses))
            x_array[i] = fsolve_guesses[best_guess_index]
        return x_array

class SonarModel(SensorModel):
    def h(self, x_array: np.ndarray):
        """
        Returns Z = h(x) where h(x) = h_0 * x + h_1
        TODO make this a quadratic
        """
        return self._h[0] * x_array + self._h[1]

    def h_inv(self, z_array: np.ndarray, guesses: Optional[np.ndarray] = None):
        """
        Uses the below equation to invert Z = h(x). Guesses array is not required as it is a linear (one root)
        TODO make this a quadratic (will require guesses, see IRBetterModel.h_inv() for example)
        x = (Z - h_1) / h_0
        """
        return np.array([(z - self._h[1]) / self._h[0] for z in z_array])