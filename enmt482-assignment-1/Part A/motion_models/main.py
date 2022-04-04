from models import *
import matplotlib.pyplot as plt

# Define models
# sonar1_model = SonarModel([1.007, 0.01]) # TODO make this a quadratic model
sonar1_model = SonarModel([0.99, 0]) # TODO make this a quadratic model

ir1_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
ir2_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
# ir4_model = TODO

filename = 'training1.csv'
_, time, distances, velocity_command, ir1_data, ir2_data, ir3_data, ir4_data, sonar1_data, sonar2_data = np.loadtxt(filename, delimiter=',', skiprows=1).T

sonar1_distances = sonar1_model.h_inv(sonar1_data)
ir1_distances = ir1_model.h_inv(ir1_data, sonar1_distances)
ir2_distances = ir2_model.h_inv(ir2_data, sonar1_distances)
ir3_distances = ir3_model.h_inv(ir3_data, sonar1_distances)
# ir4_distances = ir4_model.h_inv(ir1_data, sonar1_distances)

# plt.plot(time, ir1_distances, '.', label='ir1')
# plt.plot(time, ir2_distances, '.', label='ir2')
# plt.plot(time, ir3_distances, '.', label='ir3')

fig, axs = plt.subplots(2, 2)
axs = axs.flatten()

for i, (sensor_name, sensor_data, sensor_distances) in enumerate([
    ('sonar1', sonar1_data, sonar1_distances),
    ('ir1', ir1_data, ir1_distances),
    ('ir2', ir2_data, ir2_distances),
    ('ir3', ir3_data, ir3_distances),
]):
    axs[i].plot(time, sensor_distances, '.', label=f'{sensor_name}_pred')
    axs[i].plot(time, distances, label='distances')
    axs[i].set_xlabel('Time (t)')
    axs[i].set_ylabel('Range (m)')
    axs[i].legend()

fig.tight_layout()
plt.show()
