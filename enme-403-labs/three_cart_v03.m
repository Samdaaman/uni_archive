%By Zach Preston 2020
% -> far out you menace, why'd you start on this last year? - Sam
% Just trying to get appreciation from my true idol Geof - Zach

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

u1_1 = alpha*(km*kg)/(R*r);
u1_2 = (km^2*kg^2)/(R*r^2);

% U1 = u1_1(V) - u1_2(dv1)

%% Statespace Setup
% Matrix EOM
M = [m1 0 0;
     0 m2 0;
     0 0 m3];

C_damp = [c1+u1_2 0 0;
     0      c2 0;
     0      0 c3];

K = [k12   -k12      0;
     -k12 k23+k12 -k23;
     0     -k23   k23];

b0 = [u1_1;
      0;
      0];

% Statespace (Vector form)
A = [zeros(3)       eye(3);
    -M\K         -M\C_damp];

B1 = [zeros(3,1);
     M\b0];

C = [0 0 1 0 0 0]; % only have visibility of cart3 displ


% A = [
%     zeros(3), eye(3);
%     -k12/m1, k12/m1, 0, (-c1 - (km^2*kg^2)/(R*r^2))/m1, 0, 0;
%     k12/m2, (-k12-k23)/m2, k23/m2, 0, -c2/m2, 0;
%     0, k23/m3, -k23/m3, 0, 0, -c3/m3;
% ];
% 
% B1 = [0; 0; 0; (alpha*km*kg)/(m1*R*r); 0; 0];




%% LQR System Setup
Q = 200*eye(6); % Q Penalises poor peformance
Q(1,1) = 350 % high vals penalises displ error of cart 1
Q(2,2) = 10 % penalises displ error of cart 2
Q(3,3) = 1800; % penalises displ error of cart 3
Q(4,4) = 10; % penalises rate of disp error of cart 1
Q(5,5) = 10 % penalises rate of disp error of cart 2
Q(6,6) = 40; % penalises rate of disp error of cart 3

R = 8; % R penalises controller effort

% Execute LQR method
K = lqr(A, B1, Q, R);  % determine controller gains
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




%% Plot responses
figure(1)
plot(t,v3,t,step_size*ones(size(t)))
title('Cart 3 Displacement @ 250mm step')
xlabel('time (s)')
ylabel('(m)')
ylim([0 0.5])

figure(2)
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


%% Print controller parameters
damp(sys)














