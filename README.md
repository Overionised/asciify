

## Build
***works on linux and maybe osx***
***windows users use wsl***

### option 1 temporary install
~~~ shell
git clone https://github.com/Overionised/asciify
cd asciify
gcc main.c -lm -o asciify
~~~

### option 2 permanent install
~~~ shell
git clone https://github.com/Overionised/asciify
cd asciify
gcc main.c -lm -o asciify
sudo cp asciify /usr/bin/
~~~
## Usage:

### genral
~~~

 asciify  <options> <input_image>

        options:
			 -h   show this message 
			 -n    sets natural luminance uses the CIE standard values for percieved luminance 
			 -g <number(float)>  sets a custum gamma, 1.8 seems to be about right
			 -x set custom width value for the image 
			 -y set a custom height value for the image  
			 -f keep the original image resolution (this will result in a massive ascii wall) 
			 -c < .:-=+*#%@> set a custom character set size of 10 use _ in place of <space> 
			 -o <name_of_output_file.txt>  ( by default image is printed to terminal) 
~~~

### option 1 usage
~~~ shell
./asciify <options> <input_image>
~~~

### option 2 usage
~~~ shell
asciify <options> <input_image>
~~~
