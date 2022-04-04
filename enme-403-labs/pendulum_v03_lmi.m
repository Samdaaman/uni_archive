setlmis([]);
P=lmivar(1,[6, 1]);

lmiterm([-1 1 1 P],1,1);                        % LMI #1: P

lmiterm([2 1 1 P],A',1,'s');                    % LMI #2: A'*P+P*A
lmiterm([2 1 1 P],.5*1,-B_1*B_1'*P,'s');        % LMI #2: -P*B_1*B_1'*P (NON SYMMETRIC?)
lmiterm([2 1 1 P],.5*(1/gamma^2),B_2*B_2'*P,'s');     % LMI #2: (1/gamma^2)*P*B_2*B_2'*P (NON SYMMETRIC?)
lmiterm([2 1 1 0],C_2'*C_2);                    % LMI #2: C_2'*C_2
sys_01=getlmis;