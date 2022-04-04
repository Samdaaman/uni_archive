import tensorflow as tf

def random_scale(x):
    x_float = tf.dtypes.cast(x, tf.float32)
    noised = tf.math.multiply(x_float, tf.random.uniform(x_float.shape, minval=0.0, maxval=2.0))
    clipped = tf.clip_by_value(noised, 0, 1)
    return clipped

x = [[0.3,0.1],[0.2,0]]
random_scale(x)