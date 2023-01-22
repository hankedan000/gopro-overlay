function [vehicleTelem] = allocVehicleTelem(nSamps)
%ALLOCVEHICLETELEM Summary of this function goes here
%   Detailed explanation goes here
VariableNames = {'t_offset','engineSpeed_rpm','tps','boost_psi'};
VariableTypes = {'double','double','double','double'};
vehicleTelem = table('Size',[nSamps 4],'VariableTypes',VariableTypes,'VariableNames',VariableNames);
end

