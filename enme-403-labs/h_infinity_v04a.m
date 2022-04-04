% ENME403 - H Infinity Control
% Author: Mikael Ewans
% Last Edited: 01/05/21
% Description: Calculates the H infinity norm of a system using chosen
% gains of K.

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

C_damp = [c1+u1_2 0  0;
          0       c2 0;
          0       0  c3];

K_spring = [k12   -k12   0;
           -k12 k23+k12 -k23;
            0     -k23   k23];

b0 = [u1_1;
      0;
      0];

% Statespace (Vector form)
A = [zeros(3)            eye(3);
    -M\K_spring         -M\C_damp];

B1 = [zeros(3,1);
     M\b0];

C = [0 0 1 0 0 0]; % only have visibility of cart3 displ

%% Choosing Gains

K = [10 5 3 3 1 0];  % determine controller gains e.g. LQR produces K = [8.954,4.447,3.030,3.192,0.781,0.556]

%% LMI system setup
B2 = [1; 0; 0; 0; 0; 0]; % minimise disturbances on this particular control input
C2 = [0 0 1 0 0 0]; % set of regulated outputs desired to control

Y = 10; %gamma
A_CL = (A - B1*K); % closed loop system, so first entry in LMI matrix becomes A_CL'*P + P*A_CL + C_2'*C_2

setlmis([]);                                                         
P=lmivar(1,[6,1]);                                                   

lmiterm([1 1 1 P],A_CL',1,'s');                 % LMI #1: A_CL'*P+P*A_CL
lmiterm([1 1 1 0],C2'*C2);                      % LMI #1: C2'*C2
lmiterm([1 2 1 P],B2',1);                       % LMI #1: B2'*P
lmiterm([1 2 2 0],-Y^2);                        % LMI #1: -Y^2

lmi_mclmi_face=getlmis;

% %% LMI system setup (old)
% n = size(A_CL,1);     % number of states
% ncon = size(B1,2);    % number of control inputs
% 
% % initialise
% hinfinity = [];
% setlmis(hinfinity);
% 
% % matrix variables
% P  = lmivar(1,[n 1]); % P full symmetric
% 
% lmiterm([1 1 1 P],.5*(A_CL'-K'*B1'),1,'s');        % LMI #1: (A'-K'*B1')*P (NON SYMMETRIC?)
% lmiterm([1 1 1 P],.5*1,(A_CL-B1'*K'),'s');         % LMI #1: P*(A-B1'*K') (NON SYMMETRIC?)
% lmiterm([1 1 1 0],C2'*C2);                      % LMI #1: C2'*C2
% lmiterm([1 2 1 P],B2',1);                       % LMI #1: B2'*P
% lmiterm([1 2 2 0],-1*Y^2);                      % LMI #1: -1*Y^2
% 
% hinfinity=getlmis;

%% Solve
options = [0,0,0,0,0]; % options(3) bounds the Frobenius norm of P by 'X' 
[tmin,xfeas] = feasp(lmi_mclmi_face,options,-1); % '-1' asks tmin to be less than or equal to –1.

P_sol = dec2mat(lmi_mclmi_face,xfeas,P); % Calculates P that solves the LMI
eig(P_sol) % Check P_sol = P_sol' > 0, therefore eig(P) > 0

