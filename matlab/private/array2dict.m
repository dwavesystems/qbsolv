function arraydict = array2dict(A)
% ARRAY2DICT Converts a matlab array into a python dict.
%   arraydict = array2dict(A)
%
% Args:
%   A: a matlab array
%
% Returns:
%   arraydict (py.dict): A python dict where the keys are 2-tuples of the form (row, col) and the
%       values are the values in A.
%

    % get the row, col, val triplets from A
    [row, col, val] = find(A);
    
    arraydict = py.dict();
    
    for ri = 1:length(row)
        % in the python dict, the edges are of the form (row, col) where row and col are python
        % ints. To cast a matlab array to ints, we need to convert it into int32.
        edge = py.tuple(int32([row(ri), col(ri)]) - 1);
        
        % populate the dict
        arraydict{edge} = val(ri);
    end

end
