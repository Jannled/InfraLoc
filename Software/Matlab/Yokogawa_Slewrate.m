%% Settings
fileName = 'Oscii/000.wdf'
RELEVANT_CHANNEL = 2;

hUnit = 's';
vUnit = 'V';

%% Rohdaten visualisierung
fig1 = figure;
[data, times] = readWDF(fileName);
plot1 = plot(times, data);
xlabel(strcat('Zeit [', hUnit, ']'));
ylabel(strcat('Amplitude [', vUnit, ']'));
%title("Test");

set(plot1(1),'DisplayName','CH1','Visible','off');
set(plot1(2),'DisplayName','CH2');
set(plot1(3),'DisplayName','CH3');
set(plot1(4),'DisplayName','CH4','Visible','off');

legend('show');

%% Slew Rate
fig2 = figure;

sl_auto = statelevels(data(:, RELEVANT_CHANNEL))
sl = [0.02, 2.18]
slewrate(data(:, RELEVANT_CHANNEL), times, "StateLevels", sl);
hold on;
[slew, lt, ht] = slewrate(data(:, RELEVANT_CHANNEL), times, "StateLevels", sl);
xlabel(strcat('Zeit [', hUnit, ']'));
ylabel(strcat('Amplitude [', vUnit, ']'));
t_rise = ht - lt;

% Customize plot
ax = gca;
xlim([0, inf]);
ylim([-0.2, 2.5]);
ax.XAxis.Exponent = -3;
legend('Location', 'northeast');
fig2.Position = [0, 0, 960, 540]; %pbaspect([3, 2, 1]);
plot(times, data(:, 3), Color="#7E2F8E");
yticks(sort([yticks, sl(2)]));
hold off;

fprintf("Rise & Fall [")
for i = t_rise
   fprintf("%.4fuS, ", i*1000*1000);
end
fprintf("]\n");

fprintf("arithmetische Mittel(Anstiegszeit) = %.2fuS \n", mean(t_rise(t_rise > 0))*1000*1000);
fprintf("arithmetische Mittel(Fallzeit) = %.2fuS \n", mean(t_rise(t_rise < 0))*1000*-1000);

%exportgraphics(fig2, fileName + ".pdf");
