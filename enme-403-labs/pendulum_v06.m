% Uses LQR to get optimised gains for each lab setup. Then plots the
% responses of all the lab setups for the optimised gains
% The objective was minimal controller effort

clc
close all

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

% for ii = 1:4
%     [A, B_1] = get_state_space(setups(ii));
%     
%     Q = [
%         30, 0, 0, 0;
%         0, 10^-8, 0, 0;
%         0, 0, 10^-8, 0;
%         0, 0, 0, 10^-8;
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

%% Setting up state space
[A, B_1, C_1] = get_state_space(setup_1);
A_CL = (A - B_1*K); % closed loop system

% 1cm disturbance in x and 5deg disturbance in theta
B_2 = [0.01; 0.1; 0; 0];
C_2 = [1, 0, 0, 0; 0, 1, 0, 0];

%% Example Finding "Gamma Score" Assessment
K = [-5.48, -24.5, -9.55, -6.00];

gamma = 10;
test1 = check_feasibility(gamma, A_CL, B_2, C_2) % should be true
gamma_min = find_lowest_gamma(1, A_CL, B_2, C_2)
test2 = check_feasibility(gamma_min, A_CL, B_2, C_2) % should be true as gamma_min should be feasible
test3 = check_feasibility(gamma_min * 0.9, A_CL, B_2, C_2) % should be false as 90% of gamma_min should be infeasible


%% Grid Search Time
% Do a 6 dimensional grid search but refine the grid at every iteration
% (set limit so the grid doesn't wander to instability)
K_start = [-5.48, -24.5, -9.55, -6.00]; % low performance gains from LQR
% K_start = [-70.7107 -109.1248  -43.0149  -26.8460]; % high performance gains from LQR

grid_widths = abs(K_start) / 4;
K_guesses = [K_start];
lowest_gammas = [NaN];

dummy_figure = figure();
for ii = 1:10
    % HOT H-inf spice
    %K_guess_min = K_guesses(ii, :) - grid_widths / 2;
    %K_guess_max = K_guesses(ii, :) + grid_widths / 2;
    
    % a mild spicy H-inf
    % K_guess_min = max(K_guesses(ii, :) - grid_widths / 2, K_start - abs(K_start) / 3);
    % K_guess_max = min(K_guesses(ii, :) + grid_widths / 2, K_start + abs(K_start) / 2);

    % a pretty mild spicy H-inf
    K_guess_min = max(K_guesses(ii, :) - grid_widths / 2, K_start - abs(K_start) / 3);
    K_guess_max = min(K_guesses(ii, :) + grid_widths / 2, K_start + abs(K_start) / 3);

    % a EXTREMLY mild spicy H-inf
    % K_guess_min = max(K_guesses(ii, :) - grid_widths / 2, K_start - abs(K_start) / 5);
    % K_guess_max = min(K_guesses(ii, :) + grid_widths / 2, K_start + abs(K_start) / 5);

%     % unused
%     K_guess_min = max(K_guesses(ii, :) - grid_widths / 2, K_start*0.75);  % prevent the grid wandering into instability
%     K_guess_max = K_guesses(ii, :) + grid_widths / 2;
    
    K_limits = {};
    fprintf("Iteration number is %d\n", ii)
    for jj = 1:4
        K_limits{jj} = linspace(K_guess_min(jj), K_guess_max(jj), 5);
    end

    [lowest_gamma, lowest_gamma_location, lowest_gamma_gains] = grid_search(K_limits, A, B_1, B_2, C_2); % do the grid search and find the lowest gamma in the grid
    
    K_guesses(ii + 1, :) = lowest_gamma_gains;
    lowest_gammas(ii + 1) = lowest_gamma;
    fprintf("Gamma=%s at location [%s] for K_guess is [%s]\n", string(lowest_gamma), strjoin(string(lowest_gamma_location)), strjoin(string(lowest_gamma_gains)));
    grid_widths = grid_widths * 0.95; % reduce the grid size
    
    % some magic so you can cancel this loop by pressing a key down
    drawnow()
    is_key_pressed = ~isempty(get(dummy_figure,'CurrentCharacter'));
    if is_key_pressed
        break
    end
end

figure()
x = 1:length(lowest_gammas);
semilogy(x, lowest_gammas)
hold on
yyaxis right
for ii = 1:4
    plot(x, K_guesses(:, ii) ./ K_guesses(1, ii))
end
legend(["lowest gamma", "K1", "K2", "K3", "K4"])
title("Relative wandering of gains (scaled)")
hold off

plot_results(A, B_1, C_1, K_guesses(end, :))


%% Functions
function [A, B_1, C_1] = get_state_space(setup)
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

function plot_results(A, B_1, C_1, K)
    C_ref = [1 , 0, 0, 0]; % from the lab recommendation
    N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1

    A_CL = A - B_1*K; % the closed loop system SS matrix
    B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

    sys = ss(A_CL, B_hat, C_1, 0);
    [y, t] = step(sys, 7);  % this y matrix has one coloumn for x and one for y, 5 seconds was chosen
    y_x = y(:, 1);
    y_theta = y(:, 2);

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
    figure()
    plot(t, y_x, "b")
    hold on
    ylabel("x")
    yyaxis right
    ylabel("\theta (degrees)")
    plot(t, y_theta*180/pi, "r")
    xline(rise_time_x, "b--")
    xline(settling_time_x, "b:")
    % xline(settling_time_theta, "r--")
    yline(10, "r:")
    yline(-10, "r:")
    legend(["y\_x", "y\_theta", strcat("rise\_time\_x: ", string(rise_time_x)), strcat("settling\_time\_x: ", string(settling_time_x)), "10 degrees overshoot limit"])
    hold off
end

% Determine whether or not a particular A_CL and gamma are feasible
% (uses LMI inequality)
function is_it_feasible = check_feasibility(gamma, A_CL, B2, C2)
    n = size(A_CL,1);     % number of states    

    hinfinity = [];
    setlmis(hinfinity);
    
    % matrix variables
    P  = lmivar(1,[n 1]); % P full symmetric

    lmiterm([1 1 1 P],A_CL',1,'s');                 % LMI #1: A_CL'*P+P*A_CL
    lmiterm([1 1 1 0],C2'*C2);                      % LMI #1: C2'*C2
    lmiterm([1 2 1 P],B2',1);                       % LMI #1: B2'*P
    lmiterm([1 2 2 0],-gamma^2);                    % LMI #1: -gamma^2

    hinfinity=getlmis;
    
    options = [0,0,10,0,1]; % options(3) bounds the Frobenius norm of P by 'X', options(5) turns of console text
    [tmin,xfeas] = feasp(hinfinity,options);

%     P_sol = dec2mat(hinfinity,xfeas,P); % Calculates P that solves the LMI
%     eig(P_sol) % Check P_sol = P_sol' > 0, therefore eig(P) > 0
    is_it_feasible = tmin < 0;
end

% Find an approximate minimum feasible gamma by searching downwards from a
% start point
function gamma_min = find_lowest_gamma(gamma_start, A_CL, B2, C2)
    gamma_guess = gamma_start;
    jump_size = gamma_guess / 2;
    
    while jump_size > 2^-10
        if check_feasibility(gamma_guess, A_CL, B2, C2)
            gamma_guess = gamma_guess - jump_size;
        else
            gamma_guess = gamma_guess + jump_size;
        end
        jump_size = jump_size / 2;
    end
    
    if check_feasibility(gamma_guess, A_CL, B2, C2) ~= 1
        gamma_guess = gamma_guess + jump_size * 2;
    end
    gamma_min = gamma_guess;
end

% Do a 4-dimensional grid search to find the lowest gamma in a grid given
function [lowest_gamma, lowest_gamma_location, lowest_gamma_gains] = grid_search(K_limits, A, B1, B2, C2)
    gamma_max = 1;
    lowest_gamma = inf;
    lowest_gamma_location = [];
        
    % I am sorry for the RAM I have claimed
    for ii_1 = 1:length(K_limits{1})
        K1 = K_limits{1}(ii_1);
        for ii_2 = 1:length(K_limits{2})
            K2 = K_limits{2}(ii_2);
            % fprintf("Progess: [%s]\n", strjoin(string([ii_1, ii_2])))
            for ii_3 = 1:length(K_limits{3})
                K3 = K_limits{3}(ii_3);
                for ii_4 = 1:length(K_limits{4})
                    K4 = K_limits{4}(ii_4);
                    K = [K1, K2, K3, K4];
                    A_CL = (A - B1*K); % closed loop system
                    gamma_test = find_lowest_gamma(min(lowest_gamma, gamma_max), A_CL, B2, C2);
                    if gamma_test < lowest_gamma
                        lowest_gamma = gamma_test;
                        lowest_gamma_location = [ii_1, ii_2, ii_3, ii_4];
                        lowest_gamma_gains = K;
                    end                     
                end
            end
        end
    end
end

