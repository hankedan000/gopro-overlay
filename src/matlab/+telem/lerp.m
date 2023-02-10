function [out] = lerp(a,b,ratio)
%LERP Summary of this function goes here
%   Detailed explanation goes here
slope = b - a;
if slope ~= 0
    out = a + (slope * ratio);
else
    out = a;
end
end

