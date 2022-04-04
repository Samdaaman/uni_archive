 
clc;
clear;
close all;

%Dataset notes:
%lab3_files(1) -> unstable and bad
%lab3_files(3) -> unstable and bad
%lab3_files(4) -> unstable as they gave us a 0.9sec period?
%lab3_files(5) ->

%Import lab names
lab1_files = ["zsp10g2", "zsp10g3", "zsp10g4"];
lab2_files = ["zsp10g1", "zsp10g2", "zsp10g3", "zsp10g3weights"];
lab3_files = ["zsp10g1", "zsp10g2", "zsp10g2pulse", "zsp10g3", "zsp10g3weights"];
set(0,'DefaultFigureWindowStyle','docked') 

% Import lab 1 datasets
for ii=1:length(lab1_files)
    curr_file_name = lab1_files(ii);
    curr_file_data = load(append(curr_file_name, '.mat'));
    data = curr_file_data.(curr_file_name);
    
    curr_test_data.identifier = curr_file_name;
    curr_test_data.t = data.X(1).Data;
    curr_test_data.v3 = data.Y(9).Data; % Cart 3 position
    curr_test_data.CMV = data.Y(13).Data; % Cart motor voltage
    curr_test_data.PC = data.Y(14).Data; % Cart position commands
    curr_test_data.RMV = data.Y(15).Data; % Cart raw motor voltage (no clipping)
    lab1_datasets(ii) = curr_test_data;
end

% Import lab 2 datasets (as uses different naming for import
for ii=1:length(lab2_files)
    curr_file_name = lab2_files(ii);
    curr_file_data = load(append(curr_file_name, '-l2', '.mat'));
    data = curr_file_data.(curr_file_name);
    
    curr_test_data.identifier = append(curr_file_name, " - lab 2");
    curr_test_data.t = data.X(1).Data;
    curr_test_data.v3 = data.Y(9).Data; % Cart 3 position
    curr_test_data.CMV = data.Y(13).Data; % Cart motor voltage
    curr_test_data.PC = data.Y(14).Data; % Cart position commands
    curr_test_data.RMV = data.Y(15).Data; % Cart raw motor voltage (no clipping)
    lab2_datasets(ii) = curr_test_data;
end

% Import lab 3 datasets (as uses different naming for import
for ii=1:length(lab3_files)
    curr_file_name = lab3_files(ii);
    curr_file_data = load(append(curr_file_name, '-l3', '.mat'));
    data = curr_file_data.(curr_file_name);
    
    curr_test_data.identifier = append(curr_file_name, " - lab 3");
    curr_test_data.t = data.X(1).Data;
    curr_test_data.v3 = data.Y(9).Data; % Cart 3 position
    curr_test_data.CMV = data.Y(13).Data; % Cart motor voltage
    curr_test_data.PC = data.Y(14).Data; % Cart position commands
    curr_test_data.RMV = data.Y(15).Data; % Cart raw motor voltage (no clipping)
    lab3_datasets(ii) = curr_test_data;
end

lab_total_datasets = [lab1_datasets lab3_datasets];




%% Plot responses
%% Plot actual behaviour
figure(1);
target_datasets = lab2_datasets;

for ii=1:length(target_datasets)
    subplot(2, 3, ii)
    
    % Plot meta data
    title(target_datasets(ii).identifier);
    xlabel("Time (s)");
    ylabel("Cart 3 Position (m)");
    
%     yyaxis left
    grid('on')
    hold on
    plot(target_datasets(ii).t, target_datasets(ii).v3, "-");
    plot(target_datasets(ii).t, target_datasets(ii).PC, "-");
    hold off

end
    
    
figure(2);
% Plot meta data
target_dataset = lab1_datasets(2);
title(target_dataset.identifier);
xlabel("Time (s)");
ylabel("Cart 3 Position (m)");
grid('on')
hold on
plot(target_dataset.t,target_dataset.v3, "-");
plot(target_dataset.t,target_dataset.PC, "-",  "Color", "#ff0000");
plot(target_dataset.t,target_dataset.PC+0.01, "--", "Color", "#ff0000");
plot(target_dataset.t,target_dataset.PC-0.01, "--", "Color", "#ff0000");
hold off



%%_________________________Comparison Plots____________________%%
mark_colours = string({'#8854d0', '#a55eea', '#3867d6', '#4b7bec' ,'#45aaf2' ,'#0fb9b1', '#2bcbba', '#20bf6b', '#26de81', '#f7b731', '#fed330' ,'#fa8231', '#fd9644', '#eb3b5a', '#fc5c65'});

% Mild spicy Hinf
K_mild = [20.1127 -0.856658 -0.599751 2.534 0.656867 0.184933];
% Hot Hinf
K_spicy = [38.9194 -1.47209 0.0867516 0.991952 2.34518 0.648148];

% Define constants
step_size =  0.25*4; % No idea why but we gotta multiply by 4 for lsim to track
% Spring constants (N/m)
ka = 175;
kb = 400;
kc = 800;
% Cart masses (kg)
m1 = 1.608;
m_unloaded = 0.75;
m_loaded = 1.25;

% Setup systems
K = K_mild;
sys_ul_ka = get_cl_sys(ka, m_unloaded, K, step_size);
sys_ul_kc = get_cl_sys(kc, m_unloaded, K, step_size);
sys_l_ka = get_cl_sys(ka, m_loaded, K, step_size);
sys_l_kc = get_cl_sys(kc, m_loaded, K, step_size);
cart_systems = [sys_ul_ka sys_ul_kc sys_l_ka sys_l_kc];

% Simulate system
target_sys = cart_systems(1).ss;
T = 3; %period in seconds
num_commands = 3;
t_step = 0:0.01:T;
t_3step = 0:0.01:num_commands*T;
ref_input = [ones(1, numel(t_step)) zeros(1, numel(t_step)-1) ones(1, numel(t_step)-1)];
x0 = zeros(6,1); %0.1.*ones(6,1); 

ref_input = lab1_datasets(2).PC;
t_3step = lab1_datasets(2).t;
 
[curr_v3, curr_t, curr_x] = lsim(target_sys, ref_input, t_3step, x0);
[curr_v3, curr_t, curr_x] = lsim(target_sys, ref_input, t_3step, x0);



% Plot comparison
figure(3);
PC_colour = "#880000";

% Plot meta data
target_dataset = lab1_datasets(2);
%target_dataset = lab3_datasets(2);
title(target_dataset.identifier);
xlabel("Time (s)");
ylabel("Cart 3 Position (m)");

grid('on')
hold on
plot(target_dataset.t,target_dataset.v3, "-", "LineWidth", 1, 'Color', mark_colours(2));
plot(curr_t, curr_v3, "-", "LineWidth", 1, 'Color', mark_colours(4));


plot(target_dataset.t,target_dataset.PC, "-",  "Color", PC_colour);
plot(target_dataset.t,target_dataset.PC+0.01, "--", "Color", PC_colour);
plot(target_dataset.t,target_dataset.PC-0.01, "--", "Color", PC_colour);
hold off

xlim([35 45])
legend(["Actual Response", "Simulated Response", "Program Command"])













%% State Space Generation Functions
function sys_out = get_cl_sys(k_spring, m_cart, K, step_size)
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

    u1_1 = alpha*(km*kg)/(R*r);
    u1_2 = (km^2*kg^2)/(R*r^2);
    
    m1 = 1.608;
    m2 = m_cart;
    m3 = m_cart;
    
    k12 = k_spring;
    k23 = k_spring;
    
    M = [m1 0 0;
         0 m2 0;
         0 0 m3];
    
    C_damp = [c1+u1_2 0 0;
         0      c2 0;
         0      0 c3];
    
    K_spring = [k12   -k12      0;
         -k12 k23+k12 -k23;
         0     -k23   k23];
    
    % Statespace (Vector form)
    A = [zeros(3)       eye(3);
        -M\K_spring         -M\C_damp];
    
    b0 = [u1_1;
          0;
          0];
    
    B1 = [zeros(3,1);
         M\b0];

    A_CL = (A - B1*K); % closed loop system
    
    C = [0 0 1 0 0 0]; % only have visibility of cart3 displ

    % Reference tracking
    N = -(C * A_CL^-1 * B1)^-1; % reference tracking
    B_hat = B1*N*step_size; % input matrix adjusted for step size with reference tracking
    
    sys_out.ss = ss(A_CL, B_hat, C, 0);
    sys_out.k_spring = k_spring;
    sys_out.m_cart = m_cart;
    sys_out.B1 = B1;
    sys_out.A = A;
    sys_out.A_CL = A_CL;
    sys_out.B_hat = B_hat;
    sys_out.K = K;
    sys_out.N = N;
    sys_out.poles = pole(sys_out.ss);
    sys_out.description = strcat("k=", string(k_spring)," m=", string(m_cart));
    
end


