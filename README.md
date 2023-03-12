This is a software 3D rendering engine that outputs the rendered result in the terminal window using ncurses. It's very much unfinished at the moment.  
  
To build this project, use the included GNU Makefile. You must have ncurses installed; there are no additional dependencies.
  
Currently, the rendered model is hard-coded. If you want to change it, it's defined in the ```main()``` function (in rasterizer.c). The vertex pipeline and the fragment pipeline are also both fixed-function at the moment; these are implemented in ```main()``` and ```draw_polygon()``` respectively.  
The only character used for rendering is currently '#'. Support for other characters (and a shader model for specifying output characters) will be added in the future.  
  
This renderer additionally supports two different color modes. The first mode ignores the first 16 system colors when initializing the palatte, as there's not an easy way to restore the defaults when the program exits. There is also a 256-color mode. However, 256-color mode changes the default terminal colors and they will not be restored when the program exits.  
  
The controls are as follows:  
	- To rotate the model, press or hold the right arrow key. (The program will exit after two full revolutions).  
	- To close the program, press q.  
	- WASD can be used to move a cursor over the rendered frame. The depth-buffer value of the hovered-over fragment will be shown in the top-right of the screen.  
  
This is just a proof-of-concept project that I thought would be neat. It's probably been done before by someone else, but oh well.  
This rendering engine was just made for fun. Feel free to use it however you like.  
