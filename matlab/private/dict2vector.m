function A = dict2vector(vectordict)
% DICT2VECTOR converts a python dict into a matlab array.
%   A = dict2vector(vectordict)
%
% Args:
%   vectordict (py.dict): A python dict where the keys are the 0-indexed indices of an array and
%       the values are the values of that array.
%
% Returns:
%   A: a matlab row vector.
%
    n_variables = py.len(vectordict);

    % create the row vector that will contain the solutions
    A = nan(1, n_variables);
    for vi = 1:n_variables
        A(vi) = vectordict{vi - 1};
    end
end

