from scipy.spatial.transform import Rotation as R
from numpy import matrix, concatenate

class rMatrix:

    def __init__(self, orders:str, rotation:list) -> None:
        rot = matrix([[1,0,0],[0,1,0],[0,0,1]])
        for i, order in enumerate(orders):
            rot = rot * R.from_euler(order, rotation[i], degrees=True).as_matrix()
        self.m: matrix = rot
        pass

    def __str__(self) -> str:
        my_str = ""
        for j, row in enumerate(self.m.tolist()):
            for i, column in enumerate(row):
                if abs(column) < 1e-6:
                    my_str += "0.00\t"
                else:
                    my_str += f"{column:.2f}\t"
            my_str += "\n"
        return my_str


if __name__ == "__main__":
    r = rMatrix('xz',[-90,180])
    print(r)
    
    