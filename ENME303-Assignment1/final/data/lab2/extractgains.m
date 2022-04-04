figure(5)
data = import_enme303('zsp10pid3');
% data([1:11971,14971:end], :) = [];
plot(data(:, 1), [data(:, 2), data(:, 4)]);
hold on
yyaxis right
plot(data(:, 1), data(:, 5));
ylim([-13 13])
% axis([11.971 14.971 -0.1 0.03]);
legend (join(string([data(1, [6:8])])));
hold off