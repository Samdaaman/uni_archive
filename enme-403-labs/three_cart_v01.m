%By Zach Preston 2020 

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
v = 10; % applied voltage (max 10v)

u1_1 = alpha*(km*kg)/(R*r);
u1_2 = (km^2*kg^2)/(R*r^2);

% U1 = u1_1(V) - u1_2(dv1)

%% Statespace Setup
% Matrix EOM
M = [m1 0 0;
     0 m2 0;
     0 0 m3];

C = [c1+u1_2 0 0;
     0      c2 0;
     0      0 c3];

% K = [-k12   k12      0;
%      k12 -k23-k12 k23;
%      0     k23   -k23];

K = [k12   -k12      0;
     -k12 k23+k12 -k23;
     0     -k23   +k23];

b0 = [u1_1;
      0;
      0];

% Statespace (Vector form)
A = [zeros(3) eye(3);
    -M\C         M\K];

B1 = [zeros(3,1);
     M\b0];

C1 = [0 0 1 0 0 0]; % only have visibility of cart3 displ


%% LQR System Setup
Q = 10*eye(6); % Q Penalises poor peformance
%Q(1,1) = 1 % high vals penalises displ error of cart 1
%Q(2,2) = 1 % penalises displ error of cart 2
%Q(3,3) = 1 % penalises displ error of cart 3
%Q(4,4) = 1 % penalises rate of disp error of cart 1
%Q(5,5) = 1 % penalises rate of disp error of cart 2
%Q(6,6) = 1 % penalises rate of disp error of cart 3


R = 5.0; % R penalises controller effort
K = lqr(A, B1, Q, R);
%N = inv(C*(inv(A-B1*K))*B1)
r = 0.250 % 250 mm reference input
sys = ss((A - B1*K), (r*B1), C1, 0)


%% System simulation using step
t = 0:0.01:50;
[y,t,x] = step(sys, t);

%% System simulation setup using Lsim
%t = 0:0.01:50;
%ref_input = ones(1, numel(t));
%x0 = zeros(6,1); %0.1.*ones(6,1); 
%[y,t,x] = lsim(sys, ref_input, t, x0);


%% Plot responses
figure(1)
plot(t,y,t,r*ones(size(t)))
title('Response to Non-Zero Initial Conditions')
xlabel('time (s)')
ylabel('(m)')
ylim([0 0.5])

%% Print controller parameters
damp(sys)














