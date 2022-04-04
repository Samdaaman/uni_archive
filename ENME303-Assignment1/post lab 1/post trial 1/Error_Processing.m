% Henry Seaton, Zach Preston, Sam Hogan 2020


clear, clc, close all

data_p_1 = import_enme303('zsp10p');    % Kp = 4.8
data_p_2 = import_enme303('zsp10p2');   % Kp = 43 -> actual overshot a bit
data_p_3 = import_enme303('zsp10p3');   % Kp = 98.2

data_pd_1 = import_enme303('zsp10pd');  % Kp = 60, Kd = 12
data_pd_2 = import_enme303('zsp10pd2'); % Kp = 60, 
data_pd_3 = import_enme303('zsp10pd3');

data_pid_1 = import_enme303('zsp10pid');
data_pid_2 = import_enme303('zsp10pid2');

%% Select Dataset to analyse
test_data = data_pd_1;

%% Unpacks data from array
Kp = test_data(1,6);
Ki = test_data(1,7);
Kd = test_data(1,8);

theoretical_tf = func_get_tf(Kp,Ki,Kd);

t_array = test_data(:,1);
pos_array = test_data(:,2);
input_array = test_data(:,4);
voltage_use = test_data(:,5);




%_____________________Z NOTES_______________________
% Kp = 43 -> actual response overshot a bit so should

%Finding constants required to convert theoretical to real-world
%Can then add/subtract these onto our updated gains
Kp_v2 = Kp - 40;
Ki_v2 = Ki + 0;
Kd_v2 = Kd + 20;

v2_tf = func_get_tf(Kp_v2,Ki_v2,Kd_v2);
v2_response = lsim(v2_tf, input_array, t_array);



%% Compares actual and theoretical results
response = lsim(theoretical_tf, input_array, t_array);

hold on;

plot(t_array,response);
plot(t_array,pos_array,'r');

diff_array = response - pos_array;
plot(t_array,diff_array);

%Zach added
plot(t_array, v2_response);

xlabel("Time (s)");
ylabel("Position (m)");
legend("Theoretical", "Actual", "Error", "v2");

hold off;







%% Voltage Analysis
total_voltage_use = sum(abs(voltage_use))

%% Comparison of actual and theoretical responses
% diff1 = compare_result(data_pd_1);
% diff2 = compare_result(data_pd_2);
% diff3 = compare_result(data_pd_3);
% 
% diff4 = compare_result(data_p_2);
% diff5 = compare_result(data_p_3);
% 
% diff6 = compare_result(data_pid_1);
% diff7 = compare_result(data_pid_2);
% close all
% 
% hold on
% plot(t_array(1:16000),diff1(1:16000));
% plot(t_array(1:16000),diff2(1:16000));
% plot(t_array(1:16000),diff3(1:16000));
% plot(t_array(1:16000),diff4(1:16000));
% plot(t_array(1:16000),diff5(1:16000));
% plot(t_array(1:16000),diff6(1:16000));
% plot(t_array(1:16000),diff7(1:16000));
% 
% 
% mean_diff = (diff1(1:16000) + diff2(1:16000) + diff3(1:16000) + diff4(1:16000) + diff5(1:16000) + diff6(1:16000) + diff7(1:16000))./7;
% plot(t_array(1:16000),mean_diff,"-x");
% 
% legend("PD1 (Kd = 12)", "PD2 (Kd = 24)", "PD3 (Kd = 5)", "P1", "P2", "PID1", "PID2", "Average")


%% Supporting/ Optional Functions
function diff_array = compare_result(test_data)
    Kp = test_data(1,6);
    Ki = test_data(1,7);
    Kd = test_data(1,8);
    
    theoretical_tf = func_get_tf(Kp,Ki,Kd);
    
    t_array = test_data(:,1);
    pos_array = test_data(:,2);
    input_array = test_data(:,4);
    
    response = lsim(theoretical_tf,input_array,t_array);

    hold on;
    
%     plot(t_array,response);
%     plot(t_array,pos_array,'r');
%     legend("Theoretical", "Actual");
    
    diff_array = response - pos_array;
    
    plot(t_array,diff_array);

end


function result_TF = func_get_tf(K_p, K_d, K_I)
    % Constants for the physical system
    M_C = 1.5; k_m = 0.017; k_g = 3.7; R = 1.5; r = 0.018; D = 7;
    Beta = (k_m * k_g) / (M_C * R * r); C = (D / M_C) + ((k_m^2 * k_g^2) / (M_C * R * r)); % constants expressions for TF

    TF_n = [(Beta * K_d), (Beta * K_p), (Beta * K_I)];  %numerator
    TF_d = [1, (C + Beta * K_d), (Beta * K_p), (Beta * K_I)];  %denominator
    result_TF = tf(TF_n, TF_d);
end