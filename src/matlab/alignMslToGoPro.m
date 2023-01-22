function [outputArg1,outputArg2] = alignMslToGoPro(mslFilePath,goProFilePath)
%ALIGNMSLTOGOPRO Summary of this function goes here
%   Detailed explanation goes here
mslTelem = msq.readlog(mslFilePath);
goProTelem = gpmf.readcsv(goProFilePath);

rpm = mslTelem.RPM;
gpsSpeed = goProTelem.gps.speed2D;

figure(1);
plot(rpm);
title('Engine Speed');
xlabel('samples');
ylabel('engine speed (rpm)');

figure(2);
plot(gpsSpeed);
title('GPS Speed');
xlabel('samples');
ylabel('speed 2D (m/s)');

end

