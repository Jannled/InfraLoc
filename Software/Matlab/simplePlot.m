serialportlist()
close all; clear;

%% Config
NUM_CHANNELS = 16;

BUFF_SIZE = 128;

%% Rainbow
lineColors = ones(1, 16, 3);
lineColors(1, :, 1) = 1/NUM_CHANNELS * (0:NUM_CHANNELS-1);
lineColors = hsv2rgb(lineColors);
lineColors = squeeze(lineColors);

%% Open Connection
serial_recver = serialport("COM7", 115200);

%% Gather data
data = zeros(BUFF_SIZE, NUM_CHANNELS);
for i = 1:length(data)
    data(i, :) = parseLine(serial_recver.readline());
end

%% Plot data
fig1 = figure;
fig1.Position = [0, 0, 960, 540];
plot1 = plot(data);

for i = 1:NUM_CHANNELS
    set(plot1(i), 'DisplayName', sprintf("D%02d", i));
end

ylim([0, 1000]);
legend('show');
ax = gca;
ax.ColorOrder = lineColors;

%% Function definitions
function values = parseLine(l)
% 
    l = l.strip();
    values = double(extractBetween(l, 2, strlength(l)-1).split(','));
end
