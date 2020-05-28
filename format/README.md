## Format String Exploitation

In this directory we will see Protostar challenges related to Format String Vulnerability.

### Compiling
ASRL must be disabled
```
pi@raspberrypi:~ $ echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```
I suggest to compile with the parameter `-Tdata=0x01010100` to avoid null bytes into the addresses of the variables


### Resources
[Format String Attack - OWASP](https://owasp.org/www-community/attacks/Format_string_attack)
[Testing Format String - OWASP](https://owasp.org/www-project-web-security-testing-guide/latest/4-Web_Application_Security_Testing/07-Input_Validation_Testing/13.3-Testing_for_Format_String)
[Format String Vulnerability](https://crypto.stanford.edu/cs155old/cs155-spring08/papers/formatstring-1.2.pdf)
