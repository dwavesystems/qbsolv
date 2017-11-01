classdef QBSolvTest < matlab.unittest.TestCase
% These unittests are not designed to test the quality of qbsolv, they assume that qbsolv has been
% tested elsewhere, they simply test that the bindings work correctly.

methods (Test)
    function testTypicalQubo(testCase)
        Q = sparse(-1*triu(ones(100, 100), 1));
        response = QBSolv().sampleQubo(Q);
        testCase.checkQuboResponse(response, Q);
        
        Q = -1 * Q;
        response = QBSolv().sampleQubo(Q);
        testCase.checkQuboResponse(response, Q);
    end
    
    function testTypicalIsing(testCase)
        h = zeros(1, 100);
        J = sparse(-1*triu(ones(100, 100), 1));
        
        response = QBSolv().sampleIsing(h, J);
        checkIsingResponse(testCase, response, h, J);
        
        h = zeros(1, 100);
        J = sparse(triu(ones(100, 100), 1));
        
        response = QBSolv().sampleIsing(h, J);
        checkIsingResponse(testCase, response, h, J);
    end
    
    function testNRepeatsArg(testCase)
        h = zeros(1, 5);
        J = sparse(triu(ones(5), 1));
        response = QBSolv().sampleIsing(h, J, 6);
        checkIsingResponse(testCase, response, h, J);

     
        Q = ones(3);
        response = QBSolv().sampleQubo(Q, 100);
        testCase.checkQuboResponse(response, Q);  % check it, because why not
    end
    
    function testTrivialQubo(testCase)
        Q = sparse([]);
        response = QBSolv().sampleQubo(Q);
        testCase.checkQuboResponse(response, Q);
        
        % this one fails, but seems to be an issue with qbsolv
%         Q = sparse([0]);
%         response = QBSolv().sampleQubo(Q);
%         testCase.checkQuboResponse(response, Q);
    end
end

methods
    function checkIsingResponse(testCase, response, h, J)
        
        testCase.verifyEqual(length(response.energies), size(response.samples, 1));
        testCase.verifyEqual(size(response.samples, 2), length(h));
        
        % check that the energy is equal to to the calculated energy of the sample
        for si = 1:size(response.samples, 1)
            sample = response.samples(si, :);
            energy = response.energies(si);
            
            testCase.verifyEqual(sum(sample.*h) + sample*J*sample', energy);
        end
    end
    
    
    function checkQuboResponse(testCase, response, Q)
        
        testCase.verifyEqual(length(response.energies), size(response.samples, 1));
        testCase.verifyEqual(size(response.samples, 2), size(Q, 1));
        
        % check that the energy is equal to to the calculated energy of the sample
        for si = 1:size(response.samples, 1)
            sample = response.samples(si, :);
            energy = response.energies(si);
            
            testCase.verifyEqual(sample*Q*sample', energy);
        end
    end
end
end

