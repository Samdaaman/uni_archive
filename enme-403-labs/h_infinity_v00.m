% ENME403 - H Infinity Control
% Author: Mikael Ewans
% Last Edited: 01/05/21
% Description: Calculates the H infinity norm of a system. Attempting
% smaller scaler problem first to prove it works. See page 106 of notes.

clc
clear

% state space equations
A  = [0    1; % Acl
    -100 -25];

B1 = [0;
      1];
  
K  = [100 15];

% disturbances - will arbitrary decide
B2 = [1; 
      1];
  
C1 = [1 0];

C2 = [1 0]; % set of regulated outputs desired to control

% gamma
Y = 10; % arbitrary

% setup LMI system
% trying to describe: [A'*P + P*A - P*B1*B1'*P + (1/Y^2)*P*B2*B2'*P + C2'*C2] <= 0
n = size(A,1);     % number of states
ncon = size(B1,2); % number of control inputs

% initialise
hinfinity = [];
setlmis(hinfinity);

% matrix variables
P  = lmivar(1,[n 1]); % P full symmetric

lmiterm([1 1 1 P],A',1,'s');                    % LMI #1: A'*P+P*A
lmiterm([1 1 1 P],.5*1,-B1*B1'*P,'s');          % LMI #1: -P*B1*B1'*P (NON SYMMETRIC?)
lmiterm([1 1 1 P],.5*(1/Y^2),B2*B2'*P,'s');     % LMI #1: (1/Y^2)*P*B2*B2'*P (NON SYMMETRIC?)
lmiterm([1 1 1 0],C2'*C2);                      % LMI #1: C2'*C2

hinfinity=getlmis;

[tmin,xfeas] = feasp(hinfinity);

P_sol = dec2mat(hinfinity,xfeas,P);

K_gains = (1/2)*B1'*P_sol; % Calculates gains based on h infinity analysis