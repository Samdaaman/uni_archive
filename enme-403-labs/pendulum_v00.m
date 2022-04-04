% Just random poles with no optimisation but I think this is the right
% method

M_p = 0.215; % mass of pendulum
M_c = 1.608; % mass of cart
L_p = 0.314; % effective half-length of pendulum
I_0 = 7.06 * 10^-3; % pendulum intertia about C of G
R = 0.16; % motor terminal resistance
r = 0.0184; % radius of pinion
k_g = 3.71; % gearing ratio
k_m = 1.68 * 10^-2; % back EMF constant
g = 9.81; % some newton kid made this up
C = 0; % assume damping is zero initially https://learn.canterbury.ac.nz/mod/hsuforum/discuss.php?d=6666#p18220

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

p = [-0.1, -0.2, -0.3, -0.4]; % these are random - we need to optimise

K = place(A, B_1, p);

C_ref = [1 , 0, 0, 0]; % from the lab recommendation
N = -(C_ref * (A-B_1*K)^-1 * B_1)^-1;

A_CL = A - B_1*K; % the closed loop system SS matrix
B_hat = B_1*N*0.1; % input matrix with reference tracking and step size (0.1) compensation

sys = ss(A_CL, B_hat, C_1, 0);
step(sys);
