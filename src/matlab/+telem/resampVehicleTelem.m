function [outTelem] = resampVehicleTelem(inTelem,outRate_hz)
%RESAMPVEHICLETELEM Summary of this function goes here
%   Detailed explanation goes here
duration_sec = inTelem(end,:).t_offset;
nSampsOut = round(outRate_hz * duration_sec);
outTelem = telem.allocVehicleTelem(nSampsOut);
outDt_sec = 1.0 / outRate_hz;

takeIdx = 1;
outTime_sec = 0.0;
for outIdx=1:nSampsOut
    [found,takeIdx] = telem.findLerpIndex(takeIdx,inTelem,outTime_sec);

    % perform interpolation
    if found
        sampA = inTelem(takeIdx,:);
        sampB = inTelem(takeIdx+1,:);
        outTelem(outIdx,:) = telem.lerpVehicleTelem(sampA,sampB,outTime_sec);
    elseif takeIdx == 0
        outTelem(outIdx,:) = inTelem(takeIdx,:);
    else
        outTelem(outIdx,:) = inTelem(end,:);
    end
    outTime_sec = outTime_sec + outDt_sec;
end
end

