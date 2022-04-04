clear;
clc;
% 
% figure(1)
% data = import_enme303('zsp10pid');
% data([1:11971,14971:end], :) = [];
% plot(data(:, 1), [data(:, 2), data(:, 4)]);
% axis([11.971 14.971 -0.1 0.03]);
% legend (join(string([data(1, [6:8])])));
% 
% figure (2)
% data = import_enme303('zsp10pid2');
% data([1:15421,19421:end], :) = [];
% plot(data(:, 1), 0 - [data(:, 2), data(:, 4)]);
% axis([15.421 19.421 0 0.13])
% legend (join(string([data(1, [6:8])])));
% 
% clc

plot_gains = @func_plot_gains;
plot_gains(100, 200, 10, 3);
plot_gains(98.2, 116, 21, 4);
plot_gains(200, 300, 30, 5);
plot_gains(200, 200, 30, 6);
plot_gains(200, 300, 25, 7);
plot_gains(200, 200, 25, 8);



function result_plot_gains = func_plot_gains(K_p, K_I, K_d, i)
    get_TF = @func_get_tf;
    get_TF_fudged = @func_get_tf_fudged;
    sys_PID = get_TF(K_p, K_I, K_d);
    sys_PID_fudged = get_TF_fudged(K_p, K_I, K_d);
    t_array = 0:0.001:4;
    figure(i)
    step(0.1*sys_PID, t_array)
    hold on
    step(0.1*sys_PID_fudged, t_array);
    legend(join(string([K_p, K_I, K_d])), "fudged")
    title(join([join(string(pole(sys_PID)), ", ") join(string(pole(sys_PID_fudged)), ", ")], " -- "))
    hold off
    result_plot_gains = 1;
end

function result_TF = func_get_tf(K_p, K_I, K_d)
    % Constants for the physical system
    M_C = 1.5; k_m = 0.017; k_g = 3.7; R = 1.5; r = 0.018; D = 7;
    Beta = (k_m * k_g) / (M_C * R * r); C = (D / M_C) + ((k_m^2 * k_g^2) / (M_C * R * r)); % constants expressions for TF

    TF_n = [(Beta * K_d), (Beta * K_p), (Beta * K_I)];  %numerator
    TF_d = [1, (C + Beta * K_d), (Beta * K_p), (Beta * K_I)];  %denominator
    result_TF = tf(TF_n, TF_d);
end

function result_TF = func_get_tf_fudged(K_p, K_I, K_d)
    % Fudge factors
    B_F = 0.15;
    C_F = 1.1;
    
    M_C = 1.5; k_m = 0.017; k_g = 3.7; R = 1.5; r = 0.018; D = 7;
    Beta = (k_m * k_g) / (M_C * R * r);
    C = (D / M_C) + ((k_m^2 * k_g^2) / (M_C * R * r)); % constants expressions for TF
    
    Beta = Beta * B_F;
    C = C * C_F;
    
    TF_n = [(Beta * K_d), (Beta * K_p), (Beta * K_I)];  %numerator
    TF_d = [1, (C + Beta * K_d), (Beta * K_p), (Beta * K_I)];  %denominator
    result_TF = tf(TF_n, TF_d);
end
