% matlab code to talk to temp sensor
port = 'COM3';

s = serial(port,'Terminator','LF');
timestamps = [];
temp_stamps = [];
trial = 0
while true
    trial = trial + 1;
    trial

    fopen(s)

    for i=1:5
        disp(['Acq #',num2str(i)])
        t = fgets(s);
        if i > 2
            timestamps = [timestamps, datetime];
            temp_stamps = [temp_stamps,str2num(t)];
        end
    end
    fclose(s)
    pause(30)

    figure(1)
    clf
    plot(timestamps,temp_stamps,'.-')
end
