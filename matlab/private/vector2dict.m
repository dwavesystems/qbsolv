function vectordict = vector2dict(A)
% VECTOR2DICT Converts a matlab 1 dimensional array into a python dict.
%   vectordict = vector2dict(A)
%
% Args:
%   A: a 1 dimensional matlab array.
%
% Returns:
%   vectordict (py.dict): a python dict where the keys are the indices and the values are the
%       values of A.
%
    vectordict = py.dict();
    
    for vi = 1:length(A)
        % convert the index into a 0-indexed int for casting to python.
        vectordict{int32(vi - 1)} = A(vi);
    end
end

