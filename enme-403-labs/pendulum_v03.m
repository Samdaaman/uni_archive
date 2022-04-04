clc
close all
plot_sys = @func_plot_sys;

%% Defining constants (one case only)
M_p = 0.215; % mass of pendulum
M_c = 1.608; % mass of cart
L_p = 0.314; % effective half-length of pendulum
I_0 = 7.06 * 10^-3; % pendulum intertia about C of G
R = 0.16; % motor terminal resistance
r = 0.0184; % radius of pinion
k_g = 3.71; % gearing ratio
k_m = 1.68 * 10^-2; % back EMF constant
g = 9.81; % some newton kid made this up

C = 0; % TODO recalculate but assume damping is zero initially https://learn.canterbury.ac.nz/mod/hsuforum/discuss.php?d=6666#p18220

A = [
    0, 0, 1, 0;
    0, 0, 0, 1;
    0, (-M_p^2 * L_p^2 * g) / ((M_c+M_p)*I_0 + M_c*M_p*L_p^2), ((I_0 + M_p*L_p^2) * (C*R*r^2 - k_m^2*k_g^2)) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r^2), 0;
    0, ((M_c+M_p) * M_p*L_p*g) / ((M_c+M_p)*I_0 + M_c*M_p*L_p^2), (-M_p*L_p * (C*R*r^2 - k_m^2*k_g^2)) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r^2), 0;
];

B_1 = [
    0;
    0;
    ((I_0 + M_p*L_p^2) * k_m*k_g) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r);
    (-M_p*L_p*k_m*k_g) / (((M_c+M_p)*I_0 + M_c*M_p*L_p^2) * R*r);
];

C_1 = [
    1, 0, 0, 0;
    0, 1, 0, 0;
];

C_2 = C_1;
B_2 = [0; 0; 1; 0];

R = 1;
Q = eye(4);

[K] = lqr(A, B_1, Q, R);

C_ref = [1 , 0, 0, 0]; % from the lab recommendation
N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1;

A_CL = A - B_1*K; % the closed loop system SS matrix
B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

gamma = 2;
% copy from _lmi file
% Based on the below equations
% P > 0
% [A'*P+P*A-K'*B_1'*P-P*B_1*K+C_2'*C_2,     P*B_2;     B_2'*P,     -gamma^2] <= 0
setlmis([]);                                                                          
P=lmivar(1,[4, 1]);                                                                   

lmiterm([-1 1 1 P],1,1);                        % LMI #1: P                          

lmiterm([2 1 1 P],A',1,'s');                    % LMI #2: A'*P+P*A                   
lmiterm([2 1 1 P],.5*K'*B_1',-1,'s');           % LMI #2: -K'*B_1'*P (NON SYMMETRIC?)
lmiterm([2 1 1 P],.5*1,-B_1*K,'s');             % LMI #2: -P*B_1*K (NON SYMMETRIC?)  
lmiterm([2 1 1 0],C_2'*C_2);                    % LMI #2: C_2'*C_2                   
lmiterm([2 2 1 P],B_2',1);                      % LMI #2: B_2'*P                     
lmiterm([2 2 2 0],-gamma^2);                    % LMI #2: -gamma^2                   

sys_01=getlmis;

% setlmis([]);                                                                                             
% P=lmivar(1,[4, 1]);                                                                                      
% 
% lmiterm([-1 1 1 P],1,1);                        % LMI #1: P                                              
% lmiterm([2 1 1 P],A',1,'s');                    % LMI #2: A'*P+P*A                                       
% lmiterm([2 1 1 P],.5*1,-B_1*B_1'*P,'s');        % LMI #2: -P*B_1*B_1'*P (NON SYMMETRIC?)                 
% lmiterm([2 1 1 P],.5*(1/gamma^2),B_2*B_2'*P,'s');     % LMI #2: (1/gamma^2)*P*B_2*B_2'*P (NON SYMMETRIC?)
% lmiterm([2 1 1 0],C_2'*C_2);                    % LMI #2: C_2'*C_2                                       
% sys_01=getlmis;    

function func_plot_sys(A_CL, B_hat, C_1)
    sys = ss(A_CL, B_hat, C_1, 0);
    [y, t] = step(sys, 10);  % this y matrix has one coloumn for x and one for y, 10 seconds was chosen
    y_x = y(:, 1);
    y_theta = y(:, 2);

    % Calculating performance metrics
    overshoot_theta = max(y_theta) * 180 / pi;
    steady_state_error_x = abs(0.1 - y_x(end)) / 0.1;
    steady_state_error_theta = abs(0.1 - y_theta(end)) / 0.1;
    rise_time_x = -1;
    settling_time_x = -1;
    settling_time_theta = -1;
    for i = 1:length(y_x)
        if rise_time_x == -1 && y_x(i) >= 0.09
            rise_time_x = t(i);
        end
        if settling_time_x == -1 && max(abs(y_x(end) - y_x(i:end))) / abs(y_x(end)) < 0.01
            settling_time_x = t(i);
        end
        if settling_time_theta == -1 && max(abs(y_theta(i:end))) < pi / 180 * 1
            settling_time_theta = t(i);
        end
    end

    % Plotting
    figure(1)
    plot(t, y_x, "b")
    hold on
    yyaxis right
    plot(t, y_theta, "r")
    xline(rise_time_x, "b--")
    xline(settling_time_x, "b:")
    xline(settling_time_theta, "r--")
    legend(["y\_x", "y\_theta", strcat("rise\_time\_x: ", string(rise_time_x)), strcat("settling\_time\_x: ", string(settling_time_x)), strcat("settling\_time\_theta: ", string(settling_time_theta))])
    hold off
end