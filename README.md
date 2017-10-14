# ACMI-Compiler

## What is this ?

This is an ACMI compiler more precisely a .flt file compiler.
For Falcon BMS 4.33 and it should work with pretty much any version of falcon starting from OpenFalcon (Haven't tested anything before that but the code was old AF so ...)

This program is meant to compile .flt files into .vhs file. 

.flt files are created and filled on the fly by falcon (wheter it's falcon BMS or openfalcon) and they are compiled into more readable .vhs files when exiting the 3D environment. The flt file is then deleted and you're left with a .vhs file that you can put into Falcon ACMI viewer (if you hate yourself) or Tacview


Now, you probably know that compiling big ACMI files takes a LOT of time, so I made this program to use the full potential of your computer to convert a .flt into .vhs 


## Modifications to the original code

So, what did I do ? 

- Changed the pointer lists to Vectors 
- Removed *continue*  that are both slow and make the code less readable
- Removed useless function that are only here for the ACMI Viewer in Falcon
- Added thread support for heavy task parallelization 

## Things be aware if you want to modify this code

- Building in Debug in Visual Studio will make the code slower because VS add a lot of debugging stuff to the vector (it's nearly **25 times slower** ) So use *release* if you want to evaluate your code performance