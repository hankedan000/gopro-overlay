function [maxima,minima] = findPeaks(data)
%FINDPEAKS Summary of this function goes here
%   Detailed explanation goes here

data_d1 = diff(data);
nSamps_d1 = size(data_d1);

maxima = [];
minima = [];
for i=1:(nSamps_d1-1)
    if data_d1(i) > 0.0 && data_d1(i+1) <= 0.0
        maxima(end+1) = i;
    elseif data_d1(i) < 0.0 && data_d1(i+1) >= 0.0
        minima(end+1) = i;
    end
end

maxima = maxima';
minima = minima';
end
