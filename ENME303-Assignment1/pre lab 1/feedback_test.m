plant = tf([1], [1, 1])
cont = tf([1, 0], [1])

sys = feedback(cont * plant, 1)