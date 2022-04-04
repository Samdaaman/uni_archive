clear;
clc;

% figure(1)
% data = load("exported_data\1p_43_0_0.mat").data;
% % data([1:11971,14971:end], :) = [];
% plot(data(:, 1), [data(:, 2), data(:, 4)]);
% % axis([11.971 14.971 -0.1 0.03]);
% legend (join(string([data(1, [6:8])])));

plot_from_file = @func_plot_from_file;
% plot_from_file("exported_data\1p_43_0_0.mat", 0, 0, 0, 0, 3);

compare_with_actual = @func_compare_with_actual;
compare_with_actual("exported_data\1p_43_0_0.mat", 6911, 2000, 0, 1, 2);
compare_with_actual("exported_data\2p_43_0_0.mat", 10780, 2000, 0, 0, 4);




% clc
% plot_gains = @func_plot_gains;
% plot_gains(43, 0, 0, 5);

function result_compare_with_actual = func_compare_with_actual(filepath, dstart, length, yoff, flip, i)
    data = load(filepath).data;
    if (dstart > 1)
        data([1:dstart,dstart + length:end], :) = [];
    end
    figure(i)
    actual_time = data(:, 1) - (dstart - 1)/1000;
    actual_CP = data(:, 2) - yoff;
    actual_PC = data(:, 4) - yoff;
    if flip == 1
        actual_CP = -actual_CP;
        actual_PC = -actual_PC;
    end
    K_p = data(1, 6);
    K_I = data(1, 7);
    K_d = data(1, 8);
    get_TF = @func_get_tf;
    get_TF_fudged = @func_get_tf_fudged;
    sys_PID = get_TF(K_p, K_I, K_d);
    sys_PID_fudged = get_TF_fudged(K_p, K_I, K_d);
    t_array = 0:0.001:length/1000;
    figure(i)
    step(0.1*sys_PID, t_array)
    hold on
    step(0.1*sys_PID_fudged, t_array);
    plot(actual_time, actual_CP);
    plot(actual_time, actual_PC);
    title("Step response (0.1) for gains " + join(string([K_p, K_I, K_d])));
    legend("theroretical-unfudged", "theroretical-fudged", "actual-CP", "actual-PC");
    hold off
    result_compare_with_actual = 1;
end

function result_plot_from_file = func_plot_from_file(filepath, dstart, dend, lower, upper, i)
    data = load(filepath).data;
    if (dend > 1)
        data([1:dstart,dend:end], :) = [];
    end
    figure(i)
    plot(data(:, 1), 0 - [data(:, 2), data(:, 4)]);
    if (dend > 1)
        axis([dstart/1000 dend/1000 lower upper]);
    end
    legend (join(string([data(1, [6:8])])));
    result_plot_from_file = "yeet";
end

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
    title("Step response (0.1) for gains " + join(string([K_p, K_I, K_d])));
    legend("theroretical-unfudged", "theroretical-fudged");
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