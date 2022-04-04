
file = 'zsp10g3';
file_name = [file '.mat'];
structure = load(file_name);
data = structure.(file);

t = data.X(1).Data;
v3 = data.Y(9).Data; % Cart 3 position
CMV = data.Y(13).Data; % Cart motor voltage
PC = data.Y(14).Data; % Cart position commands
RMV = data.Y(15).Data; % Cart raw motor voltage (no clipping)




%% Plot responses
figure(1);
step_size = 0.250;
title("lab data import")
grid('on')
yyaxis left
hold on
plot(t, v3, "-");
yline(step_size, "b--")

yyaxis right
plot(t, PC, "-");
plot(t, CMV);
hold off

%ylim([0 0.3+step_size])




