% Places poles using the LQR method using an abitarily chosen Q matrix

%% Defining constants (one case only)
M_p = 0.215; % mass of pendulum
M_c = 1.608; % mass of cart
L_p = 0.314; % effective half-length of pendulum
I_0 = 7.06 * 10^-3; % pendulum intertia about C of G
R = 0.16; % motor terminal resistance
r = 0.0184; % radius of pinion
k_g = 3.71; % gearing ratio
k_m = 1.68 * 10^-2; % back EMF constant
g = 9.81; % some newton kid made this up

C = 0; % TODO recalculate but assume damping is zero initially https://learn.canterbury.ac.nz/mod/hsuforum/discuss.php?d=6666#p18220

A = [
    0, 0, 1, 0;
    0, 0, 0, 1;
    0, (-M_p^2 * L_p^2 * g) / ((M_c+M_p)*I_0 + M_c*M_p*L_p^2), ((I_0 + M_p*L_p^2) * (C*R*r^2 - k_m^2*k_g^2)) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r^2), 0;
    0, ((M_c+M_p) * M_p*L_p*g) / ((M_c+M_p)*I_0 + M_c*M_p*L_p^2), (-M_p*L_p * (C*R*r^2 - k_m^2*k_g^2)) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r^2), 0;
];

B_1 = [
    0;
    0;
    ((I_0 + M_p*L_p^2) * k_m*k_g) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r);
    (-M_p*L_p*k_m*k_g) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r);
];

C_1 = [
    1, 0, 0, 0;
    0, 1, 0, 0;
];

%% TODO, optimise the weightings in the Q matrix
Q = [
    100, 0, 0, 0;
    0, 1, 0, 0;
    0, 0, 1, 0;
    0, 0, 0, 1;
];

%% Do some LQR VODO
R = 1;

[K] = lqr(A, B_1, Q, R);

C_ref = [1 , 0, 0, 0]; % from the lab recommendation
N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1;

A_CL = A - B_1*K; % the closed loop system SS matrix
B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

sys = ss(A_CL, B_hat, C_1, 0);
[y, t] = step(sys, 10);  % this y matrix has one coloumn for x and one for y, 10 seconds was chosen
y_x = y(:, 1);
y_theta = y(:, 2);

%% Calculating performance metrics
overshoot_theta = max(y_theta) * 180 / pi;
steady_state_error_x = abs(0.1 - y_x(end)) / 0.1;
steady_state_error_theta = abs(0.1 - y_theta(end)) / 0.1;
rise_time_x = -1;
settling_time_x = -1;
settling_time_theta = -1;
for i = 1:length(y_x)
    if rise_time_x == -1 && y_x(i) >= 0.09
        rise_time_x = t(i);
    end
    if settling_time_x == -1 && max(abs(y_x(end) - y_x(i:end))) / abs(y_x(end)) < 0.01
        settling_time_x = t(i);
    end
    if settling_time_theta == -1 && max(abs(y_theta(i:end))) < pi / 180 * 1
        settling_time_theta = t(i);
    end
end

%% Plotting
figure(1)
plot(t, y_x, "b")
hold on
yyaxis right
plot(t, y_theta, "r")
xline(rise_time_x, "b--")
xline(settling_time_x, "b:")
xline(settling_time_theta, "r--")
legend(["y\_x", "y\_theta", strcat("rise\_time\_x: ", string(rise_time_x)), strcat("settling\_time\_x: ", string(settling_time_x)), strcat("settling\_time\_theta: ", string(settling_time_theta))])
hold off