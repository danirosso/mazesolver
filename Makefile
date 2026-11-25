main:main.cpp
	c++ main.cpp -Iinclude -lX11 -lm ./lib/libraylib.a -o main
	./main lab
