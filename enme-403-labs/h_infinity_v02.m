% ENME403 - H Infinity Control
% Author: Mikael Ewans
% Last Edited: 01/05/21
% Description: Calculates the H infinity norm of a system. Code adapted for
% the three carts with Schurs Complement.

clc
clear

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

%% LMI system setup
% trying to describe: [A'*P + P*A - P*B1*B1'*P + (1/Y^2)*P*B2*B2'*P + C2'*C2] <= 0

Y = 10; %gamma
eig(A) % useful for determining gamma based on the open loop analysis

C2 = [0 0 1 0 0 0]; % set of regulated outputs desired to control

B2 = [0;
      0;
      0;
      1;
      0;
      0]; % minimise disturbances on this particular control input

n = size(A,1);     % number of states
ncon = size(B1,2); % number of control inputs

% initialise
hinfinity = [];
setlmis(hinfinity);

% matrix variables
P  = lmivar(1,[n 1]); % P full symmetric

lmiterm([1 1 1 P],A',1,'s');                    % LMI #1: A'*P+P*A
lmiterm([1 1 1 0],C2'*C2);                      % LMI #1: C2'*C2 %
lmiterm([1 2 1 P],B1',1);                       % LMI #1: B1'*P % %
lmiterm([1 2 2 0],1);                           % LMI #1: 1

hinfinity=getlmis;

%% Solve
[tmin,xfeas] = feasp(hinfinity);

P_sol = dec2mat(hinfinity,xfeas,P); % Calculates P that solves the LMI

K_gains = (1/2)*B1'*P_sol; % Calculates gains based on h infinity analysis
