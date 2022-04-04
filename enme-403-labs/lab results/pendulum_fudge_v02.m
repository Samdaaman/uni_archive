clear
clc
close all

setup_1.M_p = 0.215;
setup_1.L_p = 0.314;
setup_1.I_0 = 7.06*10^-3;
setup_1.C = -250; % from pendulum_fudge_v01.m

%% Plotting old gains aggainst fudged system
plot_response_to_gains(setup_1, [-5.48, -24.5, -9.55, -6.00], -5.48) % g1 low perf
plot_response_to_gains(setup_1, [-70.7107 -109.1248  -43.0149  -26.8460], -70.7107) % g2 high perf
plot_response_to_gains(setup_1, [-84.85284 -115.2631 -48.12292 -21.4768], -84.8528) % g3 extremly mild hinf from high perf
plot_response_to_gains(setup_1, [-7.3067  -18.8039   -8.3562   -4.0000], -7.3067) % g4 mild hinf from low perf


%% Finding and plotting new gains with LQR
[A, B_1, C_1] = get_state_space(setup_1);
Q = [
    1500, 0, 0, 0;
    0, 1000, 0, 0;
    0, 0, 10^-8, 0;
    0, 0, 0, 10^-8;
];
R = 1;
[K] = lqr(A, B_1, Q, R)

C_ref = [1 , 0, 0, 0]; % from the lab recommendation
N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1

plot_response_to_gains(setup_1, K, N);

%% Functions
function plot_response_to_gains(setup, K, N)
    [A, B_1, C_1] = get_state_space(setup);   
    A_CL = A - B_1*K; % the closed loop system SS matrix
    B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

    sys = ss(A_CL, B_hat, C_1, 0);
    poles = pole(sys)
    [y, t] = step(sys, 7);  % this y matrix has one coloumn for x and one for y, 5 seconds was chosen
    y_x = y(:, 1);
    y_theta = y(:, 2);
    
    figure()
    plot(t, y_x)
    hold on
    yyaxis right
    plot(t, y_theta * 180 / pi)
    hold off
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