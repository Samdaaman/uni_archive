figure(4)
data = import_enme303('zsp10p2');
% data([1:11971,14971:end], :) = [];
plot(data(:, 1), [data(:, 2), data(:, 4)]);
% axis([11.971 14.971 -0.1 0.03]);
legend (join(string([data(1, [6:8])])));