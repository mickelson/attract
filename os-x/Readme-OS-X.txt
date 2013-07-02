Attract-Mode Mac OS-X 
=====================

Attract-Mode would only work for me on OS-X if linked against a version
of SFML that was built with the "-D_GLIBCXX_FULLY_DYNAMIC_STRING=1" flag 
set in CMAKE_CXX_FLAGS. 

See: http://github.com/LaurentGomila/SFML/issues/5
