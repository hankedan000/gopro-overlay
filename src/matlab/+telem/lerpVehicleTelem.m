function [outSamp] = lerpVehicleTelem(inSampA,inSampB,timePoint)
%LERPVEHICLETELEM Summary of this function goes here
%   Detailed explanation goes here
dt = inSampB.t_offset - inSampA.t_offset;
ratio = (timePoint - inSampA.t_offset) / dt;
outSamp = telem.allocVehicleTelem(1);
outSamp.t_offset = timePoint;
outSamp.engineSpeed_rpm = telem.lerp(inSampA.engineSpeed_rpm,inSampB.engineSpeed_rpm,ratio);
outSamp.tps = telem.lerp(inSampA.tps,inSampB.tps,ratio);
outSamp.boost_psi = telem.lerp(inSampA.boost_psi,inSampB.boost_psi,ratio);
end

