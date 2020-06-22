% Copyright 2020 Scrootch.me!
% 
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
% 
%    http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.

% FindIldaFrames
% Find the file offset of each frame in an ILDA file
% Optional Input:
%     inputfile a full path to the file to use, if not provided a
%     file selector dialog will appear
% Output:
%     Nx1 matrix of offsets
% Optional Output: 
%     Full path of file examined

function [offsets fpath] = FindIldaFrames (inputfile)

    % If no filename specified, open a dialog
    if ~exist('inputfile', 'var')
        [f p] = uigetfile('*.ild');
        inputfile = strcat(p, f);
    end

    fpath = inputfile;

    % Try to open the input file
    fid = fopen(inputfile, 'rt');
    if fid == -1
        return;
    end

    % Start with an empty list
    offsets = [];

    % Loop until we are finished
    while true
        % Save our position in case this is a frame
        pos = ftell(fid);

        % First 24 bytes are 'ILDA, format, and text
        header = fread(fid, 24);

        % Check if we hit the end of a file (no end record, etc.)
        if feof(fid) == 1
            break;
        end

        % Test if the header is valid
        if header(1) ~= 73 || header(2) ~= 76 || header(3) ~= 68 || header(4) ~= 65
            break;
        end

        % Next four uint16 values contain rec count and frame count, etc.
        info = fread(fid, 4, '*uint16', 0, 'ieee-be');

        % No records?
        if info(1) == 0
            break;
        end

        % If it isn't a color pallete, save the frame
        if header(8) ~= 2
            offsets = [offsets; pos];
        end

        % Figure out record size based on the 5 choices
        if header(8) == 0 % 3D, index color
            rsize = 8;
        elseif header(8) == 1 %2D, index color
            rsize = 6;
        elseif header(8) == 2 % Palette
            rsize = 3;
        elseif header(8) == 4 %3D, true color
            rsize = 10;
        elseif header(8) == 5 %2D, true color
            rsize = 8;
        else
            break;
        end

        % Jump over the records
        fseek(fid, rsize * info(1), 'cof');
end

end
