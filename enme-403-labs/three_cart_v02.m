% By Zach Preston 2020
% -> far out you menace, why'd you start on this last year? - Sam

clear;
clc;


%% Defining constants

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

%% Statespace Setup
% Matrix EOM

% Statespace
A = [
    zeros(3), eye(3);
    -k12/m1, k12/m1, 0, (-c1 - (km^2*kg^2)/(R*r^2))/m1, 0, 0;
    k12/m2, (-k12-k23)/m2, k23/m2, 0, -c2/m2, 0;
    0, k23/m3, -k23/m3, 0, 0, -c3/m3;
];

B1 = [0; 0; 0; (alpha*km*kg)/(m1*R*r); 0; 0];

C = [0 0 1 0 0 0]; % only have visibility of cart3 displ

%% LQR System Setup
Q = 100*eye(6); % Q Penalises poor peformance
%Q(1,1) = 1 % high vals penalises displ error of cart 1
%Q(2,2) = 1 % penalises displ error of cart 2
%Q(3,3) = 1 % penalises displ error of cart 3
%Q(4,4) = 1 % penalises rate of disp error of cart 1
%Q(5,5) = 1 % penalises rate of disp error of cart 2
%Q(6,6) = 1 % penalises rate of disp error of cart 3

R = 1; % R penalises controller effort
K = lqr(A, B1, Q, R); % determine controller gains

A_CL = (A - B1*K); % closed loop system

N = -(C * (A-B1*K)^-1 * B1)^-1; % reference tracking
B_hat = B1*N*step_size; % input matrix adjusted for step size with reference tracking

sys = ss(A_CL, B_hat, C, 0);


%% System simulation using step
[v3, t, x] = step(sys, 10); % q_d is the derivative of q

%% System simulation setup using Lsim
%t = 0:0.01:50;
%ref_input = ones(1, numel(t));
%x0 = zeros(6,1); %0.1.*ones(6,1); 
%[y,t,x] = lsim(sys, ref_input, t, x0);


%% Calculating performance metrics
steady_state_error = abs(step_size - v3(end)) / step_size;
rise_time = -1;
settling_time = -1;
V = zeros(length(t), 1); % preallocate
for ii = 1:length(t)
    V(ii) = -K*x(ii, :)' + N*step_size;
end

for ii = 1:length(v3)
    if rise_time == -1 && v3(ii) >= step_size * 0.9
        rise_time = t(ii);
    end
    if settling_time == -1 && max(abs(v3(end) - v3(ii:end))) / abs(v3(end)) < 0.05
        settling_time = t(ii);
    end
end

%% Plot responses and performance
figure(1)
plot(t, v3)
title('Response to Non-Zero Initial Conditions')
xlabel('time (s)')
ylabel('(m)')

figure(2)
plot(t, v3, "b")
hold on
xline(rise_time, "b--")
xline(settling_time, "b:")
yyaxis right
plot(t, V, "r")
yline(10, "r--")
legend(["y\_x", strcat("rise\_time: ", string(rise_time)), strcat("settling\_time: ", string(settling_time)), "Voltage"])
hold off

%% Print controller parameters
damp(sys)














