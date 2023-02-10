function smoothedData = smoothMovingAvg(data, windowSize)
% Smooth data using a moving average filter
%
% Parameters:
%   data: NxM matrix of data, where N is the number of samples and M is the
%     number of components. Data is smoothed along the samples.
%   windowSize: size of the moving average window (must be odd)
%
% Returns:
%   smoothedData: NxM matrix of smoothed data

% Ensure window size is odd
if mod(windowSize, 2) == 0
    windowSize = windowSize + 1;
end

% Preallocate output array
smoothedData = zeros(size(data));

% Half of the window size
halfWindow = (windowSize - 1) / 2;

% Loop through each sample and apply the moving average filter
for i = 1:size(data, 1)
    % Calculate the start and end indices for the moving average window
    startIdx = max(i - halfWindow, 1);
    endIdx = min(i + halfWindow, size(data, 1));

    % Extract the window of data and compute the mean
    window = data(startIdx:endIdx, :);
    smoothedData(i, :) = mean(window, 1);
end
end
