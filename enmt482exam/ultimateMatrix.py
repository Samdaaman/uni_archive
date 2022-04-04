"""
file: ultimateMatrix.py
Author: Hank Wu 30 Oct 2021
"""
from math import pi
from sympy import Matrix, Number, factor, trigsimp, simplify, sympify, nsimplify, pretty, Eq, nsolve, eye
from typing import List


def round_expr(expr, num_digits):
    return expr.xreplace({n : round(n, num_digits) for n in expr.atoms(Number)})

def matrix_printf3x1(m: Matrix) -> str:
    mystr = "\n\t\t\t--vector--\n"
    for i in m:
        k = simplify(i)
        k = factor(k)
        k = trigsimp(k)
        k = nsimplify(k, rational = True)
        k = round_expr(k, 4)
        k = pretty(k)
        mystr += f"\t\t\t{k}\n"
    mystr += "\t\t\t----------\n"
    return mystr

class tMatrix():
    """transformation matrix generator"""
    def __init__(self, theta, d, alpha, a, print_precision = 3) -> None:
        self.precision = print_precision
        self.my_symbles = []
        theta_mat: Matrix = self.get_theta_mat(theta,d)
        alpha_mat: Matrix = self.get_alpha_mat(alpha, a)
        self.my_T: Matrix = theta_mat * alpha_mat
        pass

    def __str__(self) -> str:
        my_str = ""
        strings = []
        padding = 5
        min_col_width = 0
        col_widths = []
        print_by_row = False
        print_by_row_thresh = 150
        for i in range(4):
            for j in range(4):
                m = simplify(self.my_T[i,j])
                m = factor(m)
                m = trigsimp(m)
                m = nsimplify(m, rational = True)
                m = round_expr(m, self.precision)
                m = pretty(m)
                #m = N(m, 4)
                this_str = f"{m}"
                strings.append(this_str)
                
        max_string_num = 0
        for i in range(4):
            for j in range(4):
                this_str  = strings[j * 4 + i]
                if len(this_str) > max_string_num:
                    max_string_num = len(this_str)
            if max_string_num < min_col_width:
                max_string_num = min_col_width
            col_widths.append(max_string_num+padding)
            max_string_num = 0

        

        for i in range(4):
            for j in range(4):
                this_str  += strings[i * 4 + j]
            if len(this_str) > max_string_num:
                max_string_num = len(this_str) + padding * 4
            this_str = ""

        if max_string_num > print_by_row_thresh:
            print_by_row = True
            max_string_num = 50

        if print_by_row:
            my_str += "_"*int(max_string_num/2 - 4) + "_uMatrix" + "_"*int(max_string_num/2 - 4) + "_"*10
        else:
            my_str += "_"*int(max_string_num/2 - 4) + "_uMatrix" + "_"*int(max_string_num/2 - 4)
        my_str += "\n\n"
        
        for i in range(4):
            for j in range(4):
                if print_by_row:
                    this_str  = strings[i * 4 + j]
                    my_str += f"row {i}, col{j}: {this_str}\n"
                else:
                    this_str  = strings[i * 4 + j]
                    len_diff = col_widths[j] - len(this_str)
                    my_str += this_str
                    for k in range(len_diff):
                        my_str += " "
                    # my_str += "\t"
            my_str += "\n"

        if print_by_row:
            my_str += "_" * max_string_num + "_"*10
        else:
            my_str += "_" * max_string_num

        my_str += "\n"

        return my_str

    
    def __mul__(self, other: 'tMatrix') -> 'tMatrix':
        retval = tMatrix(0,0,0,0)
        retval.my_T = self.my_T * other.my_T
        retval.my_symbles = self.my_symbles + other.my_symbles
        return retval
    
    def get_theta_mat(self, theta, d) -> Matrix:
        if isinstance(theta, str):
            theta_r = theta
        else:
            theta_r = theta * pi / 180

        e00 = sympify(f"cos({theta_r})")
        for i in e00.free_symbols:
            self.my_symbles.append(i)
        e01 = sympify(f"-sin({theta_r})")
        e10 = sympify(f"sin({theta_r})")
        e11 = sympify(f"cos({theta_r})")
        e02 = sympify("0")
        e03 = sympify("0")
        e12 = sympify("0")
        e13 = sympify("0")
        e20 = sympify("0")
        e21 = sympify("0")
        e22 = sympify("1")
        e23 = sympify(f"{d}")
        for i in e23.free_symbols:
            self.my_symbles.append(i)
        e30 = sympify("0")
        e31 = sympify("0")
        e32 = sympify("0")
        e33 = sympify("1")
        my_mat = [[e00,e01,e02,e03],[e10,e11,e12,e13],[e20,e21,e22,e23],[e30,e31,e32,e33]]
        return Matrix(my_mat)

    def get_alpha_mat(self,alpha,a):
        if isinstance(alpha, str):
            alpha_r = alpha
        else:
            alpha_r = alpha * pi / 180
        e11 = sympify(f"cos({alpha_r})")
        for i in e11.free_symbols:
            self.my_symbles.append(i)
        e12 = sympify(f"-sin({alpha_r})")
        e21 = sympify(f"sin({alpha_r})")
        e22 = sympify(f"cos({alpha_r})")


        e00 = sympify("1")
        e01 = sympify("0")
        e02 = sympify("0")
        
        e03 = sympify(f"{a}")
        for i in e11.free_symbols:
            self.my_symbles.append(i)
        e10 = sympify("0")
        
        e13 = sympify("0")
        e20 = sympify("0")

        e23 = sympify("0")
        e30 = sympify("0")
        e31 = sympify("0")
        e32 = sympify("0")
        e33 = sympify("1")
        test = [[e00,e01,e02,e03],[e10,e11,e12,e13],[e20,e21,e22,e23],[e30,e31,e32,e33]]
        return Matrix(test)

    def ik(self, target) -> dict:
        eqs = []
        syms = []
        init_guess = []
        for i in range(4):
            for j in range(4):
                # print(f"{i} {j} :{self.my_T[i, j].is_real}")
                if self.my_T[i, j].is_real:
                    pass
                else:
                    eq = Eq(self.my_T[i, j], target[i][j])
                    a = self.my_T[i, j].free_symbols
                    b = []
                    for k in a:
                        b.append(k)
                    syms.append(b)
                    # print(eq)
                    eqs.append(eq)
                    init_guess.append(0.5)
        # print(eqs[2])
        # print(self.my_symbles)
        num_syms = len(self.my_symbles)
        init_guess = []
        for i in range(num_syms):
            init_guess.append(0.5)
        results = nsolve(eqs, self.my_symbles, init_guess, verify = False)
        retval = {}
        for i, result in enumerate(results):
            retval[self.my_symbles[i]] = result
        return retval


def jacobian_god_function(t_matrices_list: List[tMatrix], is_prismatic_list: List[bool], spacing = 30) -> None:
    T_n_from_0 = eye(4)
    for i in range(len(t_matrices_list)):
        T_n_from_0 = T_n_from_0 * t_matrices_list[i].my_T

    P_n_from_0 = Matrix([T_n_from_0.row(0).col(3), T_n_from_0.row(1).col(3), T_n_from_0.row(2).col(3)])

    cols = []

    for i, t_matrix_n_from_0 in enumerate(t_matrices_list[:-1]):
        T_i_from_0 = eye(4)
        for j in range(i+1):
            T_i_from_0 = T_i_from_0 * t_matrices_list[j].my_T

        Z_i_from_0 = Matrix([T_i_from_0.row(0).col(2), T_i_from_0.row(1).col(2), T_i_from_0.row(2).col(2)])

        P_i_from_0 = Matrix([T_i_from_0.row(0).col(3), T_i_from_0.row(1).col(3), T_i_from_0.row(2).col(3)])
        r_i_from_0 = P_n_from_0 - P_i_from_0

        z_cross_r = Z_i_from_0.cross(r_i_from_0)

        if is_prismatic_list[i]:
            col_i = [Z_i_from_0.row(0).col(0), Z_i_from_0.row(1).col(0), Z_i_from_0.row(2).col(0), 0, 0, 0]

        else:
            col_i = [z_cross_r.row(0), z_cross_r.row(1), z_cross_r.row(2), Z_i_from_0.row(0), Z_i_from_0.row(1), Z_i_from_0.row(2)]

        if True:  # for debug
            hank = tMatrix(0,0,0,0)
            hank.my_T = T_i_from_0
            print('i = ', i, '_' * 90)
            print(f'T{i}   =\n{hank}')
            print(f'Z{i}   =', matrix_printf3x1(Z_i_from_0))
            print(f'P{i}   =', matrix_printf3x1(P_i_from_0))
            print(f'r{i}   =', matrix_printf3x1(r_i_from_0))
            print(f'zxr{i} =', matrix_printf3x1(z_cross_r))
            print(f'col{i} =', matrix_printf3x1(Matrix(col_i)))
            print('_' * 100)


        cols.append(col_i)

    jacobian = Matrix(cols).T
    jacobian = nsimplify(jacobian, rational = True)
    jacobian = round_expr(jacobian, 4)
    jacobian = factor(jacobian)

    
    print('=' * 100)
    print('JACOBIAN BELOW')
    print('=' * 100)

    for i in range(jacobian.shape[0]):
        line_parts = []
        for j in range(jacobian.shape[1]):
            m = jacobian[i,j]
            m = simplify(m)
            line_parts.append(pretty(m).ljust(spacing))
        print(' | '.join(line_parts))







if __name__ == "__main__":
    # print("2020 Exam")
    # t1 = tMatrix(45, 0, 0, 250)
    # t2 = tMatrix(-45, 0, 0, 400)
    # t3 = tMatrix(0, 100, 0, 0)
    # t4 = tMatrix(45, 0, 0, 0)
    # overall = t1*t2*t3*t4
    # print(overall)

    # theta = "th1"
    # d = 0
    # alpha = 90
    # a = "L1"
    # h = tMatrix(theta, d, alpha, a)
    # print(h)

    # t1 = tMatrix("th1", "L1", 90, 0)
    # t2 = tMatrix("th2", 0, -90, 0)
    # t3 = tMatrix("th3", "L2", 0, 0)

    # print(t1)
    # print(t2)
    # print(t3)
    # print(t1*t2*t3)

    # t1 = tMatrix("Hank", 0, 0, 250)
    # t2 = tMatrix("Jeff", 0, 0, 400)
    # t3 = tMatrix(0, "Henry", 0, 0)
    # t4 = tMatrix("Zach", 0, 0, 0)
    # print(t1)
    # print(t2)
    # print(t3)
    # print(t4)

    # overall = t1*t2*t3*t4
    # print(overall)

    print("2019 Exam Q2")
    t1 = tMatrix("theta1", 1, 90, 0)
    t2 = tMatrix("theta2", 0,0,1)
    t3 = tMatrix("theta3", 0,0,0)
    t4 = tMatrix(0,0,0,"L3")
    overall = t1*t2*t3*t4
    print(t1)
    print(t2)
    print(t3)
    print(overall)

    print("2019 Exam Q2b")
    t0 = tMatrix(0, 0, 0, 0)
    t1 = tMatrix("theta1", 1, 90, 0)
    t2 = tMatrix("theta2", 0,0,1)
    t3 = tMatrix("theta3", 0,0,0)
    # t4 = tMatrix(0,0,0,"L3")
    overall = t1*t2*t3
    print(overall)

    jacobian = jacobian_god_function([t0, t1, t2, t3], (False, False, False))
