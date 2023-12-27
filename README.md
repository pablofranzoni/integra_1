# Configurable DS3231 RTC Precision Timer with DC Driver suitable for use with peristaltic pump or small pump for fish tanks or irrigation uses.
 
**With 6 modes, where the first parameter indicates the action to perform:**

1: Mode

2: Year //not used

3: Month

4: Day

5: Hour

6: Minute

7: Second

**Get Interval Data:**

1||||||;

**Set Interval Data:**

2|0|0|0|0|5|0;  //every 5 minutes

**Set Date/Time:**

3|23|11|3|19|0|0;

**Get Date/Time:**

4||||||;

1: Mode

2: Minute

3: Second

**Set activation time:**

5|2|30|0|0|0|0;

**Get activation time:**

6||||||;

Note that the parameters must be separated by | and end with ;
The character string is sent through the UART port with a USB/Serial adapter CH340 or similar. The speed is 9600 baud.
