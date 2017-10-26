classdef QBSolv
%UNTITLED2 Summary of this class goes here
%   Detailed explanation goes here

properties
end

methods (Static)
    function response = sampleQubo(Q, n_repeats)
    % SAMPLEQUBO Uses QBsolv to determine low energy samples for the given QUBO.
    
        if isempty(Q)
            response.samples = [];
            response.energies = [];
            return
        end
    
        if nargin < 2
            n_repeats = int32(50);
        end
        
        % we need to convert our Q array into a python dict
        Q_dict = array2dict(Q);
        
        % now let's pass it off the dwave_qbsolv
        pyresponse = py.dwave_qbsolv.QBSolv().sample_qubo(Q_dict, pyargs('n_repeats', n_repeats));
        
        % extract the samples and the energies from the response
        data = py.list(pyresponse.items());
        
        n_samples = py.len(data);
        
        samples = nan(n_samples, length(Q));
        energies = nan(n_samples, 1);
        for ii = 1:length(data)
            % we need to convert the python dict into a vector
            sample = dict2vector(data{ii}{1});
            samples(ii, :) = sample;
            
            % energy gets converted to a matlab double automatically
            energies(ii) = data{ii}{2};
        end
        
        response.samples = samples;
        response.energies = energies;
    end
    
    function response = sampleIsing(h, J, n_repeats)
    % TODO
    
        if isempty(h)
            response.samples = [];
            response.energies = [];
            return
        end
        
        if nargin < 3
            n_repeats = int32(50);
        end
        
        % convert h and J
        h_dict = vector2dict(h);
        J_dict = array2dict(J);
        
        % now let's pass it off the dwave_qbsolv
        pyresponse = py.dwave_qbsolv.QBSolv().sample_ising(h_dict, J_dict, pyargs('n_repeats', n_repeats));
        
        % extract the samples and the energies from the response
        data = py.list(pyresponse.items());
        
        n_samples = py.len(data);
        
        samples = nan(n_samples, length(h));
        energies = nan(n_samples, 1);
        for ii = 1:length(data)
            % we need to convert the python dict into a vector
            sample = dict2vector(data{ii}{1});
            samples(ii, :) = sample;
            
            % energy gets converted to a matlab double automatically
            energies(ii) = data{ii}{2};
        end
        
        response.samples = samples;
        response.energies = energies;
    end
end
    
end

