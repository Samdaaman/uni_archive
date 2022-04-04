% ENME403 - Pole Analysis
% Author: Zach Preston, Mikael Ewans, Sam Hogan
% Last Edited: 12/05/21
% Description: Recipe for disaster.

clc
clear

setup_1.M_p = 0.215;
setup_1.L_p = 0.314;
setup_1.I_0 = 7.06*10^-3;
setup_1.C = -250; % from pendulum_fudge_v01.m

setup_2.M_p = 0.425;
setup_2.L_p = 0.477;
setup_2.I_0 = 18.74*10^-3;
setup_2.C = -250; % from pendulum_fudge_v01.m

setup_3.M_p = 0.530;
setup_3.L_p = 0.516;
setup_3.I_0 = 21.89*10^-3;
setup_3.C = -250; % from pendulum_fudge_v01.m

setup_4.M_p = 0.642;
setup_4.L_p = 0.546;
setup_4.I_0 = 24.62*10^-3;
setup_4.C = -250; % from pendulum_fudge_v01.m

% define desired poles
poles = [-193.53521133071, -8.12498999636844, -0.92 + 1.6i, -0.92 - 1.6i];

% get state space and place poles
[A, B_1, C_1] = get_state_space(setup_1);
K = place(A, B_1, poles)*1.05;
C_ref = [1 , 0, 0, 0]; % from the lab recommendation
N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1;

% plot
plot_response_to_gains(setup_1, K, N)
plot_response_to_gains(setup_2, K, N)
plot_response_to_gains(setup_3, K, N)
plot_response_to_gains(setup_4, K, N)

%% Functions
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
    plot(t, (y_theta * 180 / pi))
    legend("y\_x", "y\_theta")
    title('Response Plot')
    hold off
end

