function [telem] = readlog(mslFilePath)
%READ reads in Megasquirt Log (*.msl) file to a matrix
%   Detailed explanation goes here

% start with auto detected import options, then override a few to support
opts = detectImportOptions(mslFilePath,'FileType','delimitedtext');
opts.VariableNamingRule = 'modify';
opts.VariableNamesLine = 3;
opts.VariableUnitsLine = 4;
opts.DataLines = 5;

telem = readtable(mslFilePath,opts);
end

