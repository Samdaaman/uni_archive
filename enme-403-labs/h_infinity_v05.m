% ENME403 - H Infinity Control
% Author: Mikael Ewans
% Last Edited: 01/05/21
% Description: Calculates the H infinity norm of a system using chosen
% gains of K.

clc
clear

%% Calculate A_CL
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


%% Example Finding "Gamma Score" Assessment
K = [10 5 3 3 1 0];  % determine controller gains e.g. LQR produces K = [8.954,4.447,3.030,3.192,0.781,0.556]
A_CL = (A - B1*K); % closed loop system

gamma = 1; %gamma

C2 = [0 0 1 0 0 0]; % set of regulated outputs desired to control
B2 = [1;1;1;0;0;0]; % minimise disturbances on this particular control input

test1 = check_feasibility(gamma, A_CL, B2, C2) % should be true 
gamma_min = find_lowest_gamma(1, A_CL, B2, C2)
test2 = check_feasibility(gamma_min, A_CL, B2, C2) % should be true
test3 = check_feasibility(gamma_min * 0.99, A_CL, B2, C2) % should be false


%% Functions
function is_it_feasible = check_feasibility(gamma, A_CL, B2, C2)
    hinfinity = [];
    setlmis(hinfinity);

    % matrix variables
    P  = lmivar(1,[6 1]); % P full symmetric

    lmiterm([1 1 1 P],A_CL',1,'s');                 % LMI #1: A_CL'*P+P*A_CL
    lmiterm([1 1 1 0],C2'*C2);                      % LMI #1: C2'*C2
    lmiterm([1 2 1 P],B2',1);                       % LMI #1: B2'*P
    lmiterm([1 2 2 0],-gamma^2);                    % LMI #1: -gamma^2

    hinfinity=getlmis;
    
    options = [0,0,10,0,1]; % options(3) bounds the Frobenius norm of P by 'X', options(5) turns of console text
    [tmin,xfeas] = feasp(hinfinity,options);

%     P_sol = dec2mat(hinfinity,xfeas,P); % Calculates P that solves the LMI
%     eig(P_sol) % Check P_sol = P_sol' > 0, therefore eig(P) > 0
    is_it_feasible = tmin < 0;
end

function gamma_min = find_lowest_gamma(gamma_start, A_CL, B2, C2)
    gamma_guess = gamma_start;
    
    while check_feasibility(gamma_guess, A_CL, B2, C2)
        gamma_guess = gamma_guess / 2;

        if gamma_guess < 10^-6  % prevent instabilty of bad things happening if gamma goes really low
            return
        end
    end
    
    gamma_min = gamma_guess;
    if gamma_guess ~= gamma_start
        for gamma_better_guess = flip(logspace(log10(gamma_guess), log10(2*gamma_guess), 100))
            if check_feasibility(gamma_better_guess, A_CL, B2, C2)
                gamma_min = gamma_better_guess;
            else
                return
            end
        end
    end
end

