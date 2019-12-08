close all
x = [ 1 2 4 8 ];
make_100x100 = [ 9 5 7 7 ];
plot(x, make_100x100, 'r-*', 'DisplayName', 'make_100x100')
hold on
make_50x50 = [ 2 1 2 3 ];
plot(x, make_50x50, 'r-*', 'DisplayName', 'make_50x50')
hold on
hold off
legend('Interpreter', 'none')
