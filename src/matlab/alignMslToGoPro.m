mslFilePath = '/home/daniel/Downloads/Autocross/20230115_MSCC/msq_logs/2021-05-12_09.37.04.msl';
goProFilePath = '/home/daniel/Downloads/Autocross/20230115_MSCC/GH010181_telemetry_combined.csv';

% function [outputArg1,outputArg2] = alignMslToGoPro(mslFilePath,goProFilePath)
%ALIGNMSLTOGOPRO Summary of this function goes here
%   Detailed explanation goes here
msqLogData = msq.readlog(mslFilePath);
vehicleTelem = msq.logToVehicleTelem(msqLogData);
goProTelem = gpmf.readcsv(goProFilePath);

%%
gpsRate_hz = goProTelem.gps.rate_hz;
smoothWindow_sec = 0.5;
smoothWindowSize = round(gpsRate_hz * smoothWindow_sec);

resampledVehicleTelem = telem.resampVehicleTelem(vehicleTelem,gpsRate_hz);

rpm = resampledVehicleTelem.engineSpeed_rpm;
rpmSmooth = dsp.smoothMovingAvg(rpm,smoothWindowSize);
gpsSpeed = goProTelem.gps.speed2D;
gpsSpeedSmooth = dsp.smoothMovingAvg(gpsSpeed,smoothWindowSize);

figure();
plot(rpm);
hold on;
plot(rpmSmooth);
hold off;
legend({'rpm','rpmSmooth'});
title('Engine Speed');
xlabel('samples');
ylabel('engine speed (rpm)');

figure();
plot(gpsSpeed);
hold on;
plot(gpsSpeedSmooth);
hold off;
legend({'gpsSpeed','gpsSpeedSmooth'});
title('GPS Speed');
xlabel('samples');
ylabel('speed 2D (m/s)');

%%
[rpmMaximaIdx,rpmMinimaIdx] = dsp.findPeaks(rpmSmooth);

rpmMaximaValues = rpmSmooth(rpmMaximaIdx);
figure();
plot(rpmSmooth);
hold on;
plot(rpmMaximaIdx,rpmMaximaValues,'ok');
hold off;
title('Engine Speed With Maxima');
xlabel('samples');
ylabel('engine speed (RPM)');

[speedMaximaIdx,speedMinimaIdx] = dsp.findPeaks(gpsSpeedSmooth);

speedMaximaValues = gpsSpeedSmooth(speedMaximaIdx);
figure();
plot(gpsSpeedSmooth);
hold on;
plot(speedMaximaIdx,speedMaximaValues,'ok');
hold off;
title('Vehicle Speed With Maxima');
xlabel('samples');
ylabel('speed (m/s)');

% end

