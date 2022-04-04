clear
clc
close all

%% Plot entire response (for finding a good section to zoom in on)
filepath = "pendulum lab 2/sammikaelzach2.mat";
data = struct2cell(load(filepath)); % get rid of pesty zsp10.. struct
data = data{1}; % get rid of pesty zsp10.. struct

t = data.X.Data;

cart_position = data.Y(1).Data;
cart_position_gain_final = data.Y(2).Data;
cart_velocity = data.Y(3).Data;
cart_velocity_gain_final = data.Y(4).Data;
tracking_gain_final = data.Y(5).Data;
cart_position_command = data.Y(6).Data;
pendulum_postion_gain_final = data.Y(7).Data;
pendulum_position_up = data.Y(8).Data; % radians
pendulum_velocity = data.Y(9).Data;
pendulum_velocity_gain_final = data.Y(10).Data;
raw_cart_motor_voltage = data.Y(11).Data;

figure()
subplot(2,1,1)
plot(t, cart_position)
hold on
plot(t, cart_position_command)
yyaxis right
plot(t, pendulum_position_up * 180 / pi)
legend(["CP", "CPC", "PPU"]);
hold off

subplot(2,1,2)
plot(t, cart_position)
hold on
yyaxis right
plot(t, raw_cart_motor_voltage)
legend(["CP", "MV"])
hold off

%% Plot proper graph parameters
t_start = 18.145;
t_end = t_start + 5;
y_x_exp_offset = 0.1;
step_size = 0.2;

i_start = find((abs(t-t_start) < 0.001) & (t > t_start)); i_start = i_start(1);
i_end = find(abs(t-t_end) < 0.001); i_end = i_end(1);

t_exp = t(i_start: i_end) - t_start;
y_x_exp = cart_position(i_start: i_end) + y_x_exp_offset;
y_theta_exp = pendulum_position_up(i_start: i_end);

K_bad = [-70.7107 -109.1248  -43.0149  -26.8460]; % second set from first lab
K_good = [-4.6,-64.0,-23.4,-10.1]; % second set from second lab

[t_bad, y_x_bad, y_theta_bad] = get_step_response(K_bad, t_end - t_start, step_size);
[t_good, y_x_good, y_theta_good] = get_step_response(K_good, t_end - t_start, step_size);

figure()
yline(step_size, 'k--')
hold on
plot(t_bad, y_x_bad, 'r')
plot(t_good, y_x_good, 'b');
plot(t_exp, y_x_exp, 'Color', '#77AC30');
ylabel("Cart position (m)")
yyaxis right
plot(t_bad, y_theta_bad * 180/pi, 'r--')
plot(t_good, y_theta_good * 180/pi, 'b--');
plot(t_exp, y_theta_exp * 180/pi, '--', 'Color', '#77AC30');
legend([
    "Step input",
    "Cart position theortical: K_{initial}",
    "Cart position theortical: K_{improved}",
    "Cart position experimental: K_{improved}",
    "Pendulum angle theoretical: K_{initial}",
    "Pendulum angle theoretical: K_{improved}",
    "Pendulum angle experimental: K_{improved}",
])
ylabel("Pendulum angle (deg)")
xlabel("Time (s)")
title("Contrasting theoretical angle control with expermental results")
hold off


function [t, y_x, y_theta] = get_step_response(K, duration, step_size)
    setup_1.M_p = 0.215;
    setup_1.L_p = 0.314;
    setup_1.I_0 = 7.06*10^-3;
    setup_1.C = -250; % from pendulum_fudge_v01.m

    [A, B_1, C_1] = get_state_space(setup_1);

    C_ref = [1 , 0, 0, 0]; % from the lab recommendation
    N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1;

    A_CL = A - B_1*K; % the closed loop system SS matrix
    B_hat = B_1*N*step_size; % input matrix with reference tracking and step size (0.1) compensation

    sys = ss(A_CL, B_hat, C_1, 0);
    [y, t] = step(sys, duration);  % this y matrix has one coloumn for x and one for y, 5 seconds was chosen
    y_x = y(:, 1);
    y_theta = y(:, 2);
end

function [A, B_1, C_1] = get_state_space(setup)
    M_c = 1.608; % mass of cart
    R = 0.16; % motor terminal resistance
    r = 0.0184; % radius of pinion
    k_g = 3.71; % gearing ratio
    k_m = 1.68 * 10^-2; % back EMF constant
    g = 9.81; % some newton kid made this up

    A = [
        0, 0, 1, 0;
        0, 0, 0, 1;
        0, (-setup.M_p^2 * setup.L_p^2 * g) / ((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2), ((setup.I_0 + setup.M_p*setup.L_p^2) * (setup.C*R*r^2 - k_m^2*k_g^2)) / (((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2) * R*r^2), 0;
        0, ((M_c+setup.M_p) * setup.M_p*setup.L_p*g) / ((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2), (-setup.M_p*setup.L_p * (setup.C*R*r^2 - k_m^2*k_g^2)) / (((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2) * R*r^2), 0;
    ];

    B_1 = [
        0;
        0;
        ((setup.I_0 + setup.M_p*setup.L_p^2) * k_m*k_g) / (((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2) * R*r);
        (-setup.M_p*setup.L_p*k_m*k_g) / (((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2) * R*r);
    ];

    C_1 = [
        1, 0, 0, 0;
        0, 1, 0, 0;
    ];
end



