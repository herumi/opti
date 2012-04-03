
experiment of optimization
=============

Test Code
-------------

* strlen_sse2.cpp ; fast strlen/memchr with SSE2 sample
* strlen_sse42.cpp ; fast strlen with SSE4.2 sample
* str_util.hpp(str_util_test.cpp) ; fast strstr, findStr and other functions with SSE4.2 sample

>Note: These sample code requires Xbyak(https://github.com/herumi/xbyak) and "-fno-operator-names" option is required on gcc to avoid analyzing "and", "or", etc. as operators.

Reference
-------------

* http://slideshare.com/herumi/x86opti3
** http://www.ustream.tv/recorded/21484472 (Japanese)

License
-------------

modified new BSD License
http://opensource.org/licenses/BSD-3-Clause

Author
-------------

MITSUNARI Shigeo(herumi at nifty dot com)

