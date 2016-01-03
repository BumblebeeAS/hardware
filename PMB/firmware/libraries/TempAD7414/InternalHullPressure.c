int adc_read(unsigned int pin)
{
int fd, len, j;
char buf[MAX_BUF];
char val[3];



len = snprintf(buf, sizeof(buf), "/sys/devices/ocp.2/helper.14/AIN%d", pin);

fd = open(buf, O_RDONLY);
if (fd < 0) {
    perror("Error in getting adc/get-value");

}

read(fd, &val, 2);
close(fd);
}
printf ("value of pin ADC%d =%.4s \n", pin, val);
return atoi(&val);
}

int main(int argc, char **argv, char **envp)
{

    int InternalHullPressure;
    adc_ports_enable();         // Enable ADC pins
    make_periodic (10000, &info);// set the timing of the beagle bone to print
    while (1)
    {

    InternalHullPressure = adc_read(0);               
  

    printf("%d:%d:%d Internal Hull Pressure is: %d  \n", tm.tm_hour, tm.tm_min, tm.tm_sec, InternalHullPressure);

    wait_period (&info);
    }

    return 0;


}
