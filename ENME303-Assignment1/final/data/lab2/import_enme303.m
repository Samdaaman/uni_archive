function data = import_enme303(file)
% IMPORT_ENME303 Import ENME303 laboratory data.
%   DATA = IMPORT_ENME303(FILE) imports data from a MATLAB file saved
%   during a ENME303 laboratory class. FILE must be a character string
%   that excludes the file extension, and which contains neither spaces
%   or dashes.
%
%   DATA is a N-by-8 matrix, where N is the number of rows of data that
%   were acquired during the laboratory class. Note that column one of
%   DATA will always contain time values (s), but that the order of the
%   remaining columns varies depending on which apparatus (A or B) was
%   used. This function will print the names of the variables stored in
%   columns 2-8 just before exiting. These variables are:
%
%   Variable Name | Desciption
%   ------------- | --------
%   RMV           | Raw Motor Voltage [V]
%   CP            | Cart Position [m]
%   CV            | Cart Velocity [m/s]
%   PC            | Cart Position Command [m]
%   P             | Proportional Gain
%   I             | Integral Gain
%   D             | Derivative Gain
%
%   Note that the RMV column contains the motor voltage prior to 12V
%   clipping.
%
%   Example:
%       data = IMPORT_ENME303('g1_pid');
%       plot(data(:, 1), data(:, 2:4));

%% File and license information.
%**************************************************************************
%
%   File:           import_enme303.m
%   Module:         N/A
%   Project:        N/A
%   Workspace:      N/A
%
%   Author:         Rodney Elliott
%   Date:           20 March 2018
%
%**************************************************************************
%
%   Copyright:      (c) 2018 Rodney Elliott
%
%   This program is free software: you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation, either version 3 of the License, or
%   (at your option) any later version.
%
%   This program is distributed in the hope that it will be useful,
%   but WITHOUT ANY WARRANTY; without even the implied warranty of
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
%   GNU General Public License for more details.
%
%   You should have received a copy of the GNU General Public License
%   along with this program. If not, see <http://www.gnu.org/licenses/>.
%
%**************************************************************************
file_name = [file '.mat'];
structure = load(file_name);

data = zeros(length(structure.(file).X(1).Data), 8);

data(:, 1) = structure.(file).X(1).Data';
data(:, 2) = structure.(file).Y(1).Data';
data(:, 3) = structure.(file).Y(2).Data';
data(:, 4) = structure.(file).Y(3).Data';
data(:, 5) = structure.(file).Y(4).Data';
data(:, 8) = structure.(file).Y(5).Data';
data(:, 7) = structure.(file).Y(6).Data';
data(:, 6) = structure.(file).Y(7).Data';

disp('Columns 2-8 contain the following:')
structure.(file).Y.Name
