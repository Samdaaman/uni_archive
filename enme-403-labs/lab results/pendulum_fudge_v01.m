clear
clc
close all

%%
% Plot response
filepath = "pendulum/zsp10g3.mat";
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
plot(t(islocalmax(cart_position)), cart_position(islocalmax(cart_position)), "r*")
yyaxis right
plot(t, raw_cart_motor_voltage)
legend(["CP", "CP-localmax", "MV"])
hold off


%% Fudging Time
% Experimental data: Find point where voltage is saturated for easy
% calculation of system dynamics [at t = 8.6950 or t(8696)]
i_start = 8696; % t = 8.6950s
i_end = 9251; % t = 9.2500s

t_trim = t(i_start:i_end);
cart_position_trim = cart_position(i_start:i_end);
x0 = [cart_position(i_start), pendulum_position_up(i_start), cart_velocity(i_start), pendulum_velocity(i_start)]; % initial state vector for lsim

% Analytical response, plot system with -12V input starting from initial
% cart position with zero velocity (as it is local maxima)
response_no_damping = get_theoretical_response(0, t_trim, x0);
response_some_damping = get_theoretical_response(-50, t_trim, x0);
response_perfect_damping = get_theoretical_response(-250, t_trim, x0);
response_lots_damping = get_theoretical_response(-500, t_trim, x0);

% Plot results
figure()
plot(t_trim, cart_position_trim);
hold on
plot(t_trim, response_no_damping, "--");
plot(t_trim, response_some_damping, "--");
plot(t_trim, response_perfect_damping, "--");
plot(t_trim, response_lots_damping, "--");
legend(["CP-ex", "no damping", "some damping", "'perfect' damping", "lots damping"])
axis([t_trim(1) t_trim(end) -inf inf])
hold off

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

function cart_position_theoretical = get_theoretical_response(C, t_trim, x0)
    setup_1.M_p = 0.215;
    setup_1.L_p = 0.314;
    setup_1.I_0 = 7.06*10^-3;
    setup_1.C = C;

    [A, B_1, C_1] = get_state_space(setup_1);
    sys = ss(A, B_1, C_1, 0);
    
    result = lsim(sys, -12*ones(1, length(t_trim)), t_trim - t_trim(1), x0);
    cart_position_theoretical = result(:, 1); % extract postition (who cares about angle)
end




