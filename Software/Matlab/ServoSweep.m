serialportlist()
close all; clear;

%% Config
servo_limits = [500, 1833];
servo_angles = [90, 270];

NUM_CHANNELS = 16;

STEP_SIZE = 10; % uS

expectedAngles = (0 : 360/NUM_CHANNELS : 360 - 360/NUM_CHANNELS);

%% Rainbow
lineColors = ones(1, 16, 3);
lineColors(1, :, 1) = 1/NUM_CHANNELS * (0:NUM_CHANNELS-1);
lineColors = hsv2rgb(lineColors);
lineColors = squeeze(lineColors);

%% Start Serial
serial_sender = serialport("COM4", 115200);
serial_recver = serialport("COM7", 115200);
fprintf("[Serial] %s", serial_sender.readline());

steps = servo_limits(1):STEP_SIZE:servo_limits(2);
steps(end+1) = steps(end) + STEP_SIZE;
steps = flip(steps);
degrees = map(steps, servo_limits(1), servo_limits(2), servo_angles(1), servo_angles(2));

%% Wait for the servo while making sure RX buffer does not overflow
writeAngle(serial_sender, steps(1));
for i = 1:10
    serial_recver.flush();
    pause(0.2);
end

%% Data aquisition loop
data = zeros(length(steps), NUM_CHANNELS);
for i = 1:length(steps)
    pulse = steps(i);
    writeAngle(serial_sender, pulse);
    pause(0.2);
    serial_recver.flush();
    serial_recver.readline(); % Discard a line that might be cut in half due to flushing
    data(i, :) = parseLine(serial_recver.readline());
end

%%
data = flip(data);

%% Plot data
fig1 = figure;
fig1.Position = [0, 0, 960, 540];
plot1 = plot(degrees, data);

for i = 1:NUM_CHANNELS
    set(plot1(i), 'DisplayName', sprintf("D%02d", i));
    % Draw dashed X lines for the expected peaks
    if i >= 5 && i <= 13
        xline(expectedAngles(i), ':', 'Color', '[0.5, 0.5, 0.5]','HandleVisibility','off');
    end
end

xlabel("Winkel [deg]");
legend('show');
ax = gca;
ax.ColorOrder = lineColors;

%% Calculate maximum angles
[maxVals, maxIdx] = max(data);
maxAngles = degrees(maxIdx);

fig2 = figure;
subplot(2, 2, 1);
bar(maxAngles);
title("Max Angles");

subplot(2, 2, 2);
bar(maxVals);
title("Max Values");

subplot(2, 2, 3);
plot(maxAngles, maxVals, '-o');
title("Angle over Value");

subplot(2, 2, 4);
angleDiff = expectedAngles - maxAngles;
bar(expectedAngles-maxAngles);
xlabel("Kanal")
ylabel("Abweichung [deg]")
%title("Angle difference");
xlim([5 - 0.5, 13 + 0.5]);

fprintf("D06 - D12 Min: %.4f°, Max: %.4f°, Mean: %.4f°, Median: %.4f° Variance: %.4f°\n", ...
    min(angleDiff(6:12)), ...
    max(angleDiff(6:12)), ...
    mean(angleDiff(6:12)), ...
    median(angleDiff(6:12)), ...
    var(angleDiff(6:12)) ...
);

%% Calculate the angles InfraLoc would spit out
infralocAngles = zeros(1, length(data));            
for i = 1:length(data)
    transformedSignals = data(i, :);
    [~, highestIndex] = max(transformedSignals);
    angle = highestIndex * (360/NUM_CHANNELS);
    leftChannel = mod((highestIndex - 2), NUM_CHANNELS) + 1;
    rightChannel = mod((highestIndex + 0), NUM_CHANNELS) + 1;

	leftValue = transformedSignals(leftChannel);
	currentValue = transformedSignals(highestIndex);
	rightValue = transformedSignals(rightChannel);

    if(leftValue > rightValue)
		angle = angle - (leftValue/currentValue) * (360/NUM_CHANNELS);
	else
		angle = angle + (rightValue/currentValue) * (360/NUM_CHANNELS);
    end
    infralocAngles(i) = angle;
end

%% Plot the Infraloc Angles over the expected angles
fig3 = figure;
plt = plot(flip([infralocAngles; degrees]'));
ylabel("Winkel [deg]");
legend("Gemessener Winkel", "Erwarteter Winkel");
legend('Location', 'southeast');
legend('show');
set(plt(2),'DisplayName','Erwarteter Winkel','LineStyle','--');


%% Wait 2s and then close Serial Ports
pause(2);
serial_sender.delete();
serial_recver.delete();


function values = parseLine(l)
% 
    l = l.strip();
    values = double(extractBetween(l, 2, strlength(l)-1).split(','));
end

function writeAngle(serport, time_micros)
% 
    serport.writeline("" + time_micros);
    serport.readline();
    serport.flush();
end

function out = map(x, inMin, inMax, outMin, outMax)
    out = (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
end
