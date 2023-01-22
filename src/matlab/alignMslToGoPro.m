function [outputArg1,outputArg2] = alignMslToGoPro(mslFilePath,goProFilePath)
%ALIGNMSLTOGOPRO Summary of this function goes here
%   Detailed explanation goes here
msqLogData = msq.readlog(mslFilePath);
vehicleTelem = msq.logToVehicleTelem(msqLogData);
goProTelem = gpmf.readcsv(goProFilePath);

resampledVehicleTelem = telem.resampVehicleTelem(vehicleTelem,50.0);

rpm = resampledVehicleTelem.engineSpeed_rpm;
gpsSpeed = goProTelem.gps.speed2D;

figure();
plot(rpm);
title('Engine Speed');
xlabel('samples');
ylabel('engine speed (rpm)');

figure();
plot(gpsSpeed);
title('GPS Speed');
xlabel('samples');
ylabel('speed 2D (m/s)');

end

