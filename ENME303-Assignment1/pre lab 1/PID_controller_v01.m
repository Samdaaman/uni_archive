%By Zach Preston 2020

clear;
clc;

% Variables
%syms x(t) s
t_array = 0:0.001:2;


%s = tf('s');
%C = pid(Kp,Ki,Kd); % same as: Kp + Ki/s + Kd*s
%controller = tf(C);

% PID Initial Controller Gains
K_p = 10;
K_d = 1;
K_I = 10;

%Tuned gains
% K_p = 30.9062;
% K_d = 20.6147;
% K_I = 10.8431;


%Method 2 for finding:
%ISSUE: Why is different respenses once feedback is inputted
M_C = 1.5; k_m = 0.017; k_g = 3.7; R = 1.5; r = 0.018; D = 7;
beta = (k_m * k_g) / (M_C * R * r); C = (D / M_C) + ((k_m^2 * k_g^2) / (M_C * R * r)); % constants expressions for TF

Cont = pid(K_p,K_I,K_d); %controller
Plant = tf(beta, [1, C, 0]); %plant
sys_total = feedback(Cont*Plant, 1)



% Transfer Function
get_TF = @func_get_tf;
sys_P = get_TF(K_p, 0, 0);
sys_PD = get_TF(K_p, K_d, 0);
sys_PID = get_TF(K_p, K_d, K_I)

figure(1);
step(0.1*sys_P, t_array)
hold on;
step(0.1*sys_PD, t_array)

[tmp_y, tmp_t] = step(0.1*sys_PID, t_array); % code to display different line width
plot(tmp_t, squeeze(tmp_y), 'LineWidth', 3) % code to display different line width

step(0.1*sys_total, t_array);
hold off;
legend("P", "PD", "PID", "sys_PID")


% figure(2)
% step(0.1*sys_PID, t_array) % the old plot function still works too

%Uncomment for GUI Tuner
%pidTuner(P, C)

%Uncomment for CLI Tuner
%opts = pidtuneOptions('CrossoverFrequency',32,'PhaseMargin',90);
%[C, info] = pidtune(P, 'pid', opts)



function result_TF = func_get_tf(K_p, K_d, K_I)
    % Constants for the physical system
    M_C = 1.5; k_m = 0.017; k_g = 3.7; R = 1.5; r = 0.018; D = 7;
    Beta = (k_m * k_g) / (M_C * R * r); C = (D / M_C) + ((k_m^2 * k_g^2) / (M_C * R * r)); % constants expressions for TF

    TF_n = [(Beta * K_d), (Beta * K_p), (Beta * K_I)];  %numerator
    TF_d = [1, (C + Beta * K_d), (Beta * K_p), (Beta * K_I)];  %denominator
    result_TF = tf(TF_n, TF_d);
end