# Code for arduino-based temperature sensor and for PC interface to use with arduino

`logger.py` and `logger.m` are scripts for Python and matlab to connect to Arduino board running `temp_controller`

Example MATLAB code to talk to Arduino termomether on `COM3` port. Code takes 5 measurements every 30 seconds:

    % matlab code to talk to temp sensor
    port = 'COM3';

    s = serial(port,'Terminator','LF');
    if ~exist('time_stamps')
        time_stamps = [];
        temp_stamps = [];
    end

    trial = 0
    while true
        trial = trial + 1;
        trial

        fopen(s)

        for i=1:5 % 5 measurements
            disp(['Acq #',num2str(i)])
            t = fgets(s);
            if i > 2
                time_stamps = [time_stamps, datetime];
                temp_stamps = [temp_stamps, str2num(t)];
            end
        end
        fclose(s)
        pause(30) % sleep for 30 seconds
        % update the figure:
        figure(1)
        clf
        plot(time_stamps,temp_stamps,'.-')
    end


# Code to connect with micromanager

Upload firmware for arduino from `umanager_temperature`

# Hardware wiring

Replicated from [Konstantin Dimitrov](https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806)

# Components:

- [Arduino Uno R3](https://www.digikey.com/products/en?keywords=1050-1024-ND)
- [Proto Shield for Arduino Kit Stackable Version R3](https://www.digikey.com/products/en?keywords=1528-1207-ND)
- [JST-SM Pigtail connector (3-pin)](https://www.digikey.com/product-detail/en/sparkfun-electronics/CAB-14575/1568-1831-ND/8543395)
- [Waterproof DS18B20 Digital Temperature Sensor](https://www.digikey.com/products/en?keywords=1528-1592-ND)
