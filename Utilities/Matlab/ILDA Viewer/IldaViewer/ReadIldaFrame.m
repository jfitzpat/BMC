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

% ReadIldaFrame
% Read in point data for a specified ILDA frame from an ILDA file
% Input:
%     inputfile a full path to the file to use
%     offset to the desired frame
% Outputs:
%     Nx1 matrix of x, y, z, and status data
%     Nx1 or Nx3 matrix of color (indexed or true color)

function [x y z s c] = ReadIldaFrame (inputfile, offset)

    % If no filename specified, open a dialog
    if ~exist('inputfile', 'var')
        [f p] = uigetfile('*.ild');
        inputfile = strcat(p, f);
    end

    % Try to open the input file
    fid = fopen(inputfile, 'rt');
    if fid == -1
        return;
    end

    % If offset suppled, jump to it
    if exist('offset', 'var')
        fseek(fid, offset, 'bof');
    end

    % First 24 bytes are 'ILDA, format, and text
    header = fread(fid, 24);

    if header(1) == 73 && header(2) == 76 && header(3) == 68 && header(4) == 65
        % Next four uint16 values contain rec count and frame count, etc.
        info = fread(fid, 4, '*uint16', 0, 'ieee-be');

        % Any records?
        if info(1) ~= 0
            pos = ftell(fid);  % Save the position of the data

            if header(8) == 0 % 3D, index color
                x = fread(fid, info(1), '*int16', 6, 'ieee-be');
                fseek(fid, pos+2, 'bof');
                y = fread(fid, info(1), '*int16', 6, 'ieee-be');
                fseek(fid, pos+4, 'bof');
                z = fread(fid, info(1), '*int16', 6, 'ieee-be');
                fseek(fid, pos+6, 'bof');
                s = fread(fid, info(1), '*uint8', 7, 'ieee-be');
                fseek(fid, pos+7, 'bof');
                c = fread(fid, info(1), '*uint8', 7, 'ieee-be');

            elseif header(8) == 1 %2D, index color
                x = fread(fid, info(1), '*int16', 4, 'ieee-be');
                fseek(fid, pos+2, 'bof');
                y = fread(fid, info(1), '*int16', 4, 'ieee-be');
                fseek(fid, pos+4, 'bof');
                s = fread(fid, info(1), '*uint8', 5, 'ieee-be');
                fseek(fid, pos+5, 'bof');
                c = fread(fid, info(1), '*uint8', 5, 'ieee-be');
                z = cast(zeros(1, info(1)), 'int16');

            elseif header(8) == 4 %3D, true color
                x = fread(fid, info(1), '*int16', 8, 'ieee-be');
                fseek(fid, pos+2, 'bof');
                y = fread(fid, info(1), '*int16', 8, 'ieee-be');
                fseek(fid, pos+4, 'bof');
                z = fread(fid, info(1), '*int16', 8, 'ieee-be');
                fseek(fid, pos+6, 'bof');
                s = fread(fid, info(1), '*uint8', 9, 'ieee-be');
                fseek(fid, pos+7, 'bof');
                b = fread(fid, info(1), '*uint8', 9, 'ieee-be');
                fseek(fid, pos+8, 'bof');
                g = fread(fid, info(1), '*uint8', 9, 'ieee-be');
                fseek(fid, pos+9, 'bof');
                r = fread(fid, info(1), '*uint8', 9, 'ieee-be');
                c = [r g b];

            elseif header(8) == 5 %2D, true color
                x = fread(fid, info(1), '*int16', 6, 'ieee-be');
                fseek(fid, pos+2, 'bof');
                y = fread(fid, info(1), '*int16', 6, 'ieee-be');
                fseek(fid, pos+4, 'bof');
                s = fread(fid, info(1), '*uint8', 7, 'ieee-be');
                fseek(fid, pos+5, 'bof');
                b = fread(fid, info(1), '*uint8', 7, 'ieee-be');
                fseek(fid, pos+6, 'bof');
                g = fread(fid, info(1), '*uint8', 7, 'ieee-be');
                fseek(fid, pos+7, 'bof');
                r = fread(fid, info(1), '*uint8', 7, 'ieee-be');
                c = [r g b];
                z = cast(zeros(1, info(1)), 'int16');

            end
        end
    end

end