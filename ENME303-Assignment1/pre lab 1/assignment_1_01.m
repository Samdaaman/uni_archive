% Variables
syms x(t) s
t_array = 0:0.001:2;

% Forcing function
f = @(t) 0.1 * heaviside(t);
f_array = f(t_array);
F = @(s) 0.1 / s;

% PID Controller Gains
K_p = 10;
K_d = 1;
K_I = 10;

% Transfer Function
get_TF = @func_get_tf;
sys_P = get_TF(K_p, 0, 0);
sys_PD = get_TF(K_p, K_d, 0);
sys_PID = get_TF(K_p, K_d, K_I);
lsim(sys_P, sys_PD, sys_PID, f_array, t_array);
legend("P", "PD", "PID")


function result_TF = func_get_tf(K_p, K_d, K_I)
    % Constants for the physical system
    M_C = 1.5; k_m = 0.017; k_g = 3.7; R = 1.5; r = 0.018; D = 7;
    Beta = (k_m * k_g) / (M_C * R * r); C = (D / M_C) + ((k_m^2 * k_g^2) / (M_C * R * r)); % constants expressions for TF

    TF_n = [(Beta * K_d), (Beta * K_p), (Beta * K_I)];
    TF_d = [1, (C + Beta * K_d), (Beta * K_p), (Beta * K_I)];
    result_TF = tf(TF_n, TF_d);
end
