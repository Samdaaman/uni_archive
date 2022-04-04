import numpy as np
import sys
import time


def poisson_neumann(n: int, iterations: int, delta: int, new_output_format: bool = False, output_nothing = False):
    f = np.zeros(n*n*n)
    f[(n*n*n // 2)] = 1  # single 1 in the middle

    V = np.zeros([iterations+1, n*n*n])

    for i in range(iterations):
        # Helper function
        def get_V(x_i, y_i, z_i):
            return V[i][((z_i * n) + y_i) * n + x_i]

        for x in range(n):
            for y in range(n):
                for z in range(n):
                    if x != n - 1:
                        term_1 = get_V(x+1, y, z)
                    else:
                        term_1 = get_V(x-1, y, z)

                    if x != 0:
                        term_2 = get_V(x-1, y, z)
                    else:
                        term_2 = get_V(x+1, y, z)

                    if y != n-1:
                        term_3 = get_V(x, y+1, z)
                    else:
                        term_3 = get_V(x, y-1, z)

                    if y != 0:
                        term_4 = get_V(x, y-1, z)
                    else:
                        term_4 = get_V(x, y+1, z)

                    if z != n-1:
                        term_5 = get_V(x, y, z+1)
                    else:
                        term_5 = get_V(x, y, z-1)

                    if z != 0:
                        term_6 = get_V(x, y, z-1)
                    else:
                        term_6 = get_V(x, y, z+1)
                    
                    V[i+1][((z * n) + y) * n + x] = 1 / 6 * (term_1 + term_2 + term_3 + term_4 + term_5 + term_6 - (delta ** 2) * f[((z * n) + y) * n + x])

    output = ''

    if new_output_format:
        # New "reference2" way
        # Print out the 1st, 2nd, 3rd, 4th, middle-1, middle, middle+1, 2nd last, last slices
        for z in range(n):
            if (z < 4 or z == n//2-1 or z == n//2 or z == n//2+1 or z > n-3):
                output += "--------------------z=%i--------------------\n" % z;
                for x in range(n):
                    for y in range(n):
                        output += '%0.5f ' % V[iterations][(z * n + y) * n + x]
                    output += '\n'
    else:
        # Old "reference" way
        # Print out the middle slice (ie Z cross-section) of the cube for validation
        for x in range(n):
            for y in range(n):
                output += '%0.5f ' % V[iterations][((n//2) * n + y) * n + x]
            output += '\n'
    
    return output


def compare_with_file(filename: str, output: str):
    print(output)
    with open(filename) as fh:
        test_output = fh.read()

    for i in range(len(output)):
        if output[i] != test_output[i]:
            print(f'[{filename}] failed: difference at index {i} {output[i]} != {test_output[i]}')
            break
    else:
        print(f'[{filename}] Success')

def fill_test_folder(n_array = [7, 15, 21, 31, 41, 51]):
    for n in n_array:
        print(f'Generating model answer for {n}x{n}x{n}')
        output = poisson_neumann(n, 300, 1, new_output_format=True)
        with open(f'../reference2/{n}.txt', 'w') as fh:
            fh.write(output)
    print('Done')


def benchmark():
    start = time.perf_counter() * 10**9
    poisson_neumann(int(sys.argv[1]), int(sys.argv[2]), 1)
    time_diff = time.perf_counter() * 10**9 - start

    seconds = int(time_diff // 10**9)
    nanoseconds = int(time_diff % 10**9)

    print(f'{nanoseconds} {seconds}')


def main():
    # # test 1: ./poisson -n 7 -i 300
    # compare_with_file('reference/7.txt', poisson_neumann(7, 300, 1))
    
    # # test 2: ./poisson -n 15 -i 300
    # compare_with_file('reference/15.txt', poisson_neumann(15, 300, 1))

    # # test 3: ./poisson -n 51 -i 300
    # compare_with_file('reference/51.txt', poisson_neumann(51, 300, 1))

    benchmark()


if __name__ == '__main__':
    main()
