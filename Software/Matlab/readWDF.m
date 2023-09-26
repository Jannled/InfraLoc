function [dataTable, timeSeries] = readWDF(filename)
%READWDF Read all curves from the Yokogawa
%   Modified ytmSampleWdfPlotMultiWave(filename) by Yokogawa
    
    history = 1;
    SAMPLE_COUNT = 125000;
    MAX_CHANNELS = 4;

    [traceCount, result] = ytmWdfGetTraceCount(filename);
    if result == 1
        return;
    end

    tmpStorage = zeros(SAMPLE_COUNT, MAX_CHANNELS);
    x = 1:SAMPLE_COUNT;

    for trace = 1 : traceCount
        %   Get file parameters
        [traceName, result] =  ytmWdfGetTraceName(filename, trace);
        if result == 1
            return;
        end
    
        [vScaleUpper, result] = ytmWdfGetVUpperScale(filename, trace, history);
        if result == 1
            return;
        end
    
        [vScaleLower, result] = ytmWdfGetVLowerScale(filename, trace, history);
        if result == 1
            return;
        end
    
        [hResolution, result] = ytmWdfGetHResolution(filename, trace, history);
        if result == 1
            return;
        end
    
        [hOffset, result] = ytmWdfGetTriggerOffset(filename, trace, history);
        if result == 1
            return;
        end
    
        [vUnit, result] = ytmWdfGetVUnit(filename, trace, history);
        if result == 1
            return;
        end
    
        [hUnit, result] = ytmWdfGetHUnit(filename, trace, history);
        if result == 1
            return;
        end
    
        [recordLength, result] = ytmWdfGetRecordLength(filename, trace, history);
        if result == 1
            return;
        end

        % Create x axis
        clear x;
        x(1 : recordLength) = hOffset : hResolution : hResolution * (double(recordLength) - 1) + hOffset;
        
        % Get waveform data
        %clear data;
        [data, result] = ytmWdfGetData(filename, trace, history);
        if result == 1
            return;
        end
        
        tmpStorage(:, trace) = data;
    end
    result = 0;

    dataTable = tmpStorage;
    timeSeries = x;
end
