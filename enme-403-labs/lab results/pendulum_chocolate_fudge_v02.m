% ENME403 - Pendulum
% Author: Zach Preston, Mikael Ewans, Sam Hogan
% Last Edited: 12/05/21
% Description: Chocolate fudge recipe.

% Best gains so far...
% [-27.0,-85.1,-35.5,-15.2] - Video 1 - 45 degrees with larger i
% [-4.6,-64.0,-23.4,-10.1]  - Video 2 - 50 degrees with slightly smaller i
% [-18.8,-75.9,-28.5,-12.3] - Video 3 - large i slow reponse

clear
clc
close all
format longG % helps get a feel for the numbers

setup_1.M_p = 0.215;
setup_1.L_p = 0.314;
setup_1.I_0 = 7.06*10^-3;
setup_1.C = -250; % from pendulum_fudge_v01.m

%% Plotting old gains aggainst fudged system
% plot_response_to_gains(setup_1, [-5.48, -24.5, -9.55, -6.00], -5.48) % g1 low perf
% plot_response_to_gains(setup_1, [-70.7107 -109.1248  -43.0149  -26.8460], -70.7107) % g2 high perf
% plot_response_to_gains(setup_1, [-84.85284 -115.2631 -48.12292 -21.4768], -84.8528) % g3 extremly mild hinf from high perf
% plot_response_to_gains(setup_1, [-7.3067  -18.8039   -8.3562   -4.0000], -7.3067) % g4 mild hinf from low perf

%% Plotting placed gains
% plot_response_to_gains(setup_1, [-3.4,-54.9,-16.4,-7.1], -3.4); % 5 Degrees
% plot_response_to_gains(setup_1, [-3.5,-55.8,-17.1,-7.4], -3.5); % 10 Degrees
% plot_response_to_gains(setup_1, [-3.6,-56.8,-17.9,-7.8], -3.6); % 15 Degrees
% plot_response_to_gains(setup_1, [-3.8,-58.0,-18.7,-8.1], -3.8); % 20 Degrees
% plot_response_to_gains(setup_1, [-4.1,-59.1,-19.6,-8.5], -4.1); % 25 Degrees
% plot_response_to_gains(setup_1, [-4.5,-60.5,-20.6,-8.9], -4.5); % 30 Degrees
% plot_response_to_gains(setup_1, [-5.0,-62.1,-21.7,-9.4], -5.0); % 35 Degrees
% plot_response_to_gains(setup_1, [-5.7,-63.8,-23.0,-9.9], -5.7); % 40 Degrees
plot_response_to_gains(setup_1, [-6.7,-66.0,-24.5,-10.6], -6.7); % 45 Degrees
plot_response_to_gains(setup_1, [-1.7,-58.5,-19.6,-8.5], -1.7); % 45 Degrees, less i
plot_response_to_gains(setup_1, [-27.0,-85.1,-35.5,-15.2], -27.0); % 45 Degrees, more i
plot_response_to_gains(setup_1, [-13.6,-60.1,-18.4,-8.0], -13.6); % 5 Degrees, more i
plot_response_to_gains(setup_1, [-3.8,-62.1,-22.0,-9.5], -3.8); % 45 Degrees, medium i
plot_response_to_gains(setup_1, [-4.6,-64.0,-23.4,-10.1], -4.6); % 50 Degrees, medium i
%plot_response_to_gains(setup_1, , );
%plot_response_to_gains(setup_1, , );
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

% plot_response_to_gains(setup_1, K, N);

%% Functions
function plot_response_to_gains(setup, K, N)
    [A, B_1, C_1] = get_state_space(setup);   
    A_CL = A - B_1*K; % the closed loop system SS matrix
    B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

    sys = ss(A_CL, B_hat, C_1, 0);
    poles = pole(sys)
    angle = (atan(real(poles(3))/imag(poles(3)))/(2*pi))*360
    [y, t] = step(sys, 7);  % this y matrix has one coloumn for x and one for y, 5 seconds was chosen
    y_x = y(:, 1);
    y_theta = y(:, 2);
    
%     figure()
%     plot(poles, 'x')
%     hold on
%     title('Poles Plot')
%     grid on
%     hold off
    
    figure()
    plot(t, y_x)
    hold on
    yyaxis right
    plot(t, y_theta * 180 / pi)
    legend("y\_x", "y\_theta")
    title('Response Plot')
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