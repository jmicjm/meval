# meval
```c++
#include "meval.h"
```
## plain expression
```c++
double v = eval("-(-sin(pi/3)^2^-2+5(-e^-1))-(4-2pi)+abs(ln(e/8))!");
```

## expression with variables:
```c++
std::cout << eval("x+y;var x=11;var y=3");
```
output:```14```

### operators
```+ - * / % ^ !(factorial) < <= > >= == != !(not) && || =```
### functions
```sin cos tan cot asin acos atan acot log log10 log2 sqrt cbrt abs floor ceil round```

#### note
-implicit multiplication have higher precedence than explicit multiplication  
-due to the way floating point arithmetic works relational and logical operators can be unreliable
