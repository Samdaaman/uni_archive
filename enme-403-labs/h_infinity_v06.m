% ENME403 - H Infinity Control
% Author: Zach Preston, Mikael Ewans, Sam Hogan
% Last Edited: 01/05/21
% Description: Find some gains with a bit of H-infinity magic

clc;
clear;
close all;

%% Calculate A_CL
% Cart masses (kg)
m1 = 1.608;

m_unloaded = 0.75;
m_loaded = 1.25;

m2 = m_unloaded;
m3 = m_unloaded;

% Spring constants (N/m)

ka = 175;
kb = 400;
kc = 800;

k12 = ka;
k23 = ka;

% Damping (Ns/m)
c1 = 0;
c2 = 3.68;
c3 = 3.68;

% Motor input
alpha = 12.45;
km = 0.00176;
kg = 3.71;
R = 1.4;
r = 0.0184;
step_size = 0.250; % can be 250mm or 500mm

u1_1 = alpha*(km*kg)/(R*r);
u1_2 = (km^2*kg^2)/(R*r^2);

% Matrix EOM
M = [m1 0 0;
     0 m2 0;
     0 0 m3];

C_damp = [c1+u1_2 0  0;
          0       c2 0;
          0       0  c3];

K_spring = [k12   -k12   0;
           -k12 k23+k12 -k23;
            0     -k23   k23];

b0 = [u1_1;
      0;
      0];

% Statespace (Vector form)
A = [zeros(3)            eye(3);
    -M\K_spring         -M\C_damp];

B1 = [zeros(3,1);
     M\b0];

C = [0 0 1 0 0 0]; % only have visibility of cart3 displ


%% Tunning Matrices
C2 = [0 0 1 0 0 0]; % set of regulated outputs desired to control
B2 = [1;1;1;0;0;0]; % minimise disturbances on this particular control input

%% Example Finding "Gamma Score" Assessment
K = [10 5 3 3 1 0];  % determine controller gains e.g. LQR produces K = [8.954,4.447,3.030,3.192,0.781,0.556]
A_CL = (A - B1*K); % closed loop system

gamma = 1; %gamma

C2 = [0 0 1 0 0 0]; % set of regulated outputs desired to control
B2 = [1;1;1;0;0;0]; % minimise disturbances on this particular control input

test1 = check_feasibility(gamma, A_CL, B2, C2) % should be true
gamma_min = find_lowest_gamma(1, A_CL, B2, C2)
test2 = check_feasibility(gamma_min, A_CL, B2, C2) % should be true as gamma_min should be feasible
test3 = check_feasibility(gamma_min * 0.9, A_CL, B2, C2) % should be false as 90% of gamma_min should be infeasible

%% Grid Search Time
% Do a 6 dimensional grid search but refine the grid at every iteration
% (set limit so the grid doesn't wander to instability)
K_start = [8.954,4.447,3.030,3.192,0.781,0.556]; % gains from LQR
grid_widths = abs(K_start) / 2;
K_guesses = [K_start];
lowest_gammas = [NaN];

dummy_figure = figure();
for ii = 1:50
%     K_guess_min = K_guesses(ii, :) - grid_widths / 2;
%     K_guess_max = K_guesses(ii, :) + grid_widths / 2;

    K_guess_min = max(K_guesses(ii, :) - grid_widths / 2, K_start*0.75);  % prevent the grid wandering into instability
    K_guess_max = K_guesses(ii, :) + grid_widths / 2;
    
    K_limits = {};
    fprintf("Iteration number is %d\n", ii)
    for jj = 1:6
        K_limits{jj} = linspace(K_guess_min(jj), K_guess_max(jj), 3);
    end

    [lowest_gamma, lowest_gamma_location, lowest_gamma_gains] = grid_search(K_limits, A, B1, B2, C2); % do the grid search and find the lowest gamma in the grid
    
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
for ii = 1:6
    plot(x, K_guesses(:, ii) ./ K_guesses(1, ii))
end
legend(["lowest gamma", "K1", "K2", "K3", "K4", "K5", "K6"])
title("Relative wandering of gains (scaled)")
hold off

plot_results(A, B1, C, K_guesses(end, :))


%% Functions
function plot_results(A, B1, C, K)
    step_size = 0.25;
    A_CL = (A - B1*K); % closed loop system

    % Reference tracking
    N = -(C * A_CL^-1 * B1)^-1; % reference tracking
    B_hat = B1*N*step_size; % input matrix adjusted for step size with reference tracking

    sys = ss(A_CL, B_hat, C, 0);


    %% System simulation using step
    t = 0:0.01:10;
    [v3,t,x] = step(sys, 3);


    %% Calculating performance metrics
    steady_state_error = abs(step_size - v3(end)) / step_size;

    % Determine controller effort
    V = zeros(length(t), 1); % initialise as zero array
    for ii = 1:length(t)
        V(ii) = -K*x(ii, :)' + N*step_size;
    end

    % initially populate peformance metrics 
    rise_time = -1;
    settling_time = -1; 
    for ii = 1:length(v3)
        if rise_time == -1 && v3(ii) >= step_size * 0.9
            rise_time = t(ii);
        end
        if settling_time == -1 && max(abs(v3(end) - v3(ii:end))) / abs(v3(end)) < 0.05
            settling_time = t(ii);
        end
    end
    
    figure()
    plot(t,v3,t,step_size*ones(size(t)))
    title('Cart 3 Displacement @ 250mm step')
    xlabel('time (s)')
    ylabel('(m)')
    ylim([0 0.5])

    figure()
    title('Cart 3 Displacement @ 250mm step')
    yyaxis left
    plot(t, v3, "b")
    hold on
    xline(rise_time, "b--")
    xline(settling_time, "b:")
    hold off
    ylim([0 0.5])

    yyaxis right
    plot(t, V, "r")
    ylim([0 10])

    legend('v_3', strcat("t_{rise}: ", string(rise_time)), strcat("t_{settle}: ", string(settling_time)), 'Voltage');
end

% Determine whether or not a particular A_CL and gamma are feasible
% (uses LMI inequality)
function is_it_feasible = check_feasibility(gamma, A_CL, B2, C2)
    hinfinity = [];
    setlmis(hinfinity);

    % matrix variables
    P  = lmivar(1,[6 1]); % P full symmetric

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
    
    while check_feasibility(gamma_guess, A_CL, B2, C2)
        gamma_guess = gamma_guess / 2;

        if gamma_guess < 10^-6  % prevent instabilty of bad things happening if gamma goes really low
            return
        end
    end
    
    gamma_min = gamma_guess;
    if gamma_guess ~= gamma_start
%         for gamma_better_guess = flip(logspace(log10(gamma_guess), log10(2*gamma_guess), 10))
        for gamma_better_guess = linspace(2*gamma_guess, gamma_guess, 20)
            if check_feasibility(gamma_better_guess, A_CL, B2, C2)
                gamma_min = gamma_better_guess;
            else
                return
            end
        end
    end
end

% Do a 6-dimensional grid search to find the lowest gamma in a grid given
% limits (3x3x3x3x3x3 size grid is reckommended for speed)
function [lowest_gamma, lowest_gamma_location, lowest_gamma_gains] = grid_search(K_limits, A, B1, B2, C2)
    gamma_max = 1;
    lowest_gamma = inf;
    lowest_gamma_location = [];
        
    % I am sorry for the RAM I have claimed
    for ii_1 = 1:length(K_limits{1})
        K1 = K_limits{1}(ii_1);
        for ii_2 = 1:length(K_limits{2})
            K2 = K_limits{2}(ii_2);
            fprintf("Progess: [%s]\n", strjoin(string([ii_1, ii_2])))
            for ii_3 = 1:length(K_limits{3})
                K3 = K_limits{3}(ii_3);
                for ii_4 = 1:length(K_limits{4})
                    K4 = K_limits{4}(ii_4);
                    for ii_5 = 1:length(K_limits{5})
                        K5 = K_limits{5}(ii_5);
                        for ii_6 = 1:length(K_limits{6})
                            K6 = K_limits{6}(ii_6);
                            K = [K1, K2, K3, K4, K5, K6];
                            A_CL = (A - B1*K); % closed loop system
                            gamma_test = find_lowest_gamma(min(gamma_max, 1.01 * lowest_gamma), A_CL, B2, C2);
                            if gamma_test < lowest_gamma
                                lowest_gamma = gamma_test;
                                lowest_gamma_location = [ii_1, ii_2, ii_3, ii_4, ii_5, ii_6];
                                lowest_gamma_gains = K;
                            end
                        end
                    end
                end
            end
        end
    end
end

