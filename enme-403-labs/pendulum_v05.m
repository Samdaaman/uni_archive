% Uses LQR to get optimised gains for each lab setup. Then plots the
% responses of all the lab setups for the optimised gains
% The objective was minimal controller effort

clc
close all
plot_results = @func_plot_results;
get_state_space = @func_get_state_space;

%% Defining constants
setup_1.M_p = 0.215;
setup_1.L_p = 0.314;
setup_1.I_0 = 7.06*10^-3;

setup_2.M_p = 0.425;
setup_2.L_p = 0.477;
setup_2.I_0 = 18.74*10^-3;

setup_3.M_p = 0.530;
setup_3.L_p = 0.516;
setup_3.I_0 = 21.89*10^-3;

setup_4.M_p = 0.642;
setup_4.L_p = 0.546;
setup_4.I_0 = 24.62*10^-3;

setups = [setup_1, setup_2, setup_3, setup_4];

p = [-8, -8.1, -8.2, -8.3];

[A, B_1] = get_state_space(setups(3));
K = place(A, B_1, p);

figure(1)
for jj = 1:4
    setup_test = setups(jj);
    subplot(2, 2, jj)
    hold on
    title(strcat("Setup #", string(jj)))
    [A_test, B_1_test, C_1_test] = get_state_space(setups(jj));
    plot_results(A_test, B_1_test, C_1_test, K)
end
disp(K)
    
% % Old LQR Code
% for ii = 1:4
%     [A, B_1] = get_state_space(setups(ii));
% 
%     Q = [
%         30, 0, 0, 0;
%         0, 0, 0, 0;
%         0, 0, 0, 0;
%         0, 0, 0, 0;
%     ];
% 
%     R = 1;
% 
%     [K] = lqr(A, B_1, Q, R);
% 
%     figure(ii)
%     for jj = 1:4
%         setup_test = setups(jj);
%         subplot(2, 2, jj)
%         hold on
%         title(strcat("Setup #", string(jj)))
%         subtitle(strcat("Using optimised gains from setup #", string(ii)))
%         [A_test, B_1_test, C_1_test] = get_state_space(setups(jj));
%         plot_results(A_test, B_1_test, C_1_test, K)
%     end
%     
%     disp(strcat("Optimised gains for setup #", string(ii), " are:"))
%     disp(K)
% end

function [A, B_1, C_1] = func_get_state_space(setup)
    M_c = 1.608; % mass of cart
    R = 0.16; % motor terminal resistance
    r = 0.0184; % radius of pinion
    k_g = 3.71; % gearing ratio
    k_m = 1.68 * 10^-2; % back EMF constant
    g = 9.81; % some newton kid made this up

    C = 0; % TODO recalculate but assume damping is zero initially https://learn.canterbury.ac.nz/mod/hsuforum/discuss.php?d=6666#p18220

    A = [
        0, 0, 1, 0;
        0, 0, 0, 1;
        0, (-setup.M_p^2 * setup.L_p^2 * g) / ((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2), ((setup.I_0 + setup.M_p*setup.L_p^2) * (C*R*r^2 - k_m^2*k_g^2)) / (((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2) * R*r^2), 0;
        0, ((M_c+setup.M_p) * setup.M_p*setup.L_p*g) / ((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2), (-setup.M_p*setup.L_p * (C*R*r^2 - k_m^2*k_g^2)) / (((M_c+setup.M_p)*setup.I_0 + M_c*setup.M_p*setup.L_p^2) * R*r^2), 0;
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

function func_plot_results(A, B_1, C_1, K)
    C_ref = [1 , 0, 0, 0]; % from the lab recommendation
    N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1;

    A_CL = A - B_1*K; % the closed loop system SS matrix
    B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

    sys = ss(A_CL, B_hat, C_1, 0);
    [y, t, x] = step(sys, 7);  % this y matrix has one coloumn for x and one for y, 5 seconds was chosen
    y_x = y(:, 1);
    y_theta = y(:, 2);
    u = [];
    d_u = [];
    for ii = 1:length(t)
        u(ii) = -K * x(ii, :)';
        if ii == 1
            d_u(ii) = 0;
        else
            d_u(ii) = (u(ii) - u(ii - 1)) / (t(ii) - t(ii - 1));
        end
    end
    

    %% Calculating performance metrics
    overshoot_theta = max(y_theta) * 180 / pi;
    steady_state_error_x = abs(0.1 - y_x(end)) / 0.1;
    steady_state_error_theta = abs(0.1 - y_theta(end)) / 0.1;
    rise_time_x = -1;
    settling_time_x = -1;
    settling_time_theta = -1;
    for ii = 1:length(y_x)
        if rise_time_x == -1 && y_x(ii) >= 0.09
            rise_time_x = t(ii);
        end
        if settling_time_x == -1 && max(abs(y_x(end) - y_x(ii:end))) / abs(y_x(end)) < 0.01
            settling_time_x = t(ii);
        end
        if settling_time_theta == -1 && max(abs(y_theta(ii:end))) < pi / 180 * 1
            settling_time_theta = t(ii);
        end
    end

    %% Plotting
    plot(t, y_x, "b")
    hold on
    ylabel("x")
    yyaxis right
%     ylabel("\theta (degrees)")
%     plot(t, y_theta*180/pi, "r")
    ylabel("controller voltage slew")
    plot(t, d_u, "r")
    xline(rise_time_x, "b--")
    xline(settling_time_x, "b:")
    xline(settling_time_theta, "r--")
    yline(10, "r:")
    legend(["y\_x", "controller voltage slew", strcat("rise\_time\_x: ", string(rise_time_x)), strcat("settling\_time\_x: ", string(settling_time_x)), strcat("settling\_time\_theta: ", string(settling_time_theta)), "10 degrees overshoot limit"])
    hold off
end