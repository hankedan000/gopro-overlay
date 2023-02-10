% get the path to this script
stk = dbstack;
thisScriptPath = which(stk(1).file);
[thisScriptDir,thisScriptName,ext] = fileparts(thisScriptPath);

% add script directories to the path
addpath(thisScriptDir);
addpath([thisScriptDir,'/../../extern/gopro-telem/src/matlab']);