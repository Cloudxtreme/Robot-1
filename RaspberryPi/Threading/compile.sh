#
#!/bin/bash
#
gcc -g -o rgb_soft_pwm rgb_soft_pwm.c -I /usr/local/include -L /usr/local/lib -l pigpio -l rt -l pthread

