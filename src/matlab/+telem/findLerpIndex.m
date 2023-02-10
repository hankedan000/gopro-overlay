function [found,index] = findLerpIndex(initIndex,samps,searchTime)
%FINDLERPINDEX Summary of this function goes here
%   Detailed explanation goes here
    found = false;
    nSamps = size(samps,1);
    for index=initIndex:nSamps
	    nextIndex = index + 1;
        if nextIndex > nSamps
            break
        end
    
        if samps(index,:).t_offset <= searchTime && searchTime <= samps(nextIndex,:).t_offset
            found = true;
            break
        end
    end
end
