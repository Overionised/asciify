#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define malpic(p, i, j, c) (p[((j) * (x+1) + (i)) * 3 + (c)])

#include "libs/stb_image.h"
#include "libs/stb_image_resize2.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <stdio.h>

float makelin(float c) {
    if (c <= 0.04045f) {
        return c / 12.92f;
    } else {
        return powf((c + 0.055f) / 1.055f, 2.4f);
    }
}

float g_makeperlumi(float Y, float gamma) {
    // Simple gamma correction (gamma = 2.2)
	Y = Y / 100.0f; //normalise values back
    return powf(Y, 1.0f/gamma) * 100.0f;
}

float n_makeperlumi(float Y) {
	Y= Y/100;
    if (Y <= (216.0f / 24389.0f)) { // The CIE standard states 0.008856 but 216/24389 is the intent for 0.008856451679036
        return Y * (24389.0f / 27.0f); 		// The CIE standard states 903.3, but 24389/27 is the intent, making 903.296296296296296
    } else {
        return powf(Y, (1.0f / 3.0f)) * 116.0f - 16.0f;
    }
}

void outterm( int x, int y, float **lumi, char *charset){


    for (int j = 1; j <= y; j++) {
        for (int i = 1; i <= x; i++) {
            int index = (int)(lumi[i][j] / 10.0f);
            if (index > 9) index = 9;  // Cap at 9 (valid indices: 0-9)
            if (index < 0) index = 0;

            // Use %c for single character, not %s
            printf( "%c", charset[index]);
        }
        printf("\n");
    }
}


void outfile(char *filename, int x, int y, float **lumi, char *charset){
    FILE *fptr;
    fptr = fopen(filename, "w");
    if (!fptr) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return;
    }

    for (int j = 1; j <= y; j++) {
        for (int i = 1; i <= x; i++) {
            int index = (int)(lumi[i][j] / 10.0f);
            if (index > 9) index = 9;  // Cap at 9 (valid indices: 0-9)
            if (index < 0) index = 0;

            // Use %c for single character, not %s
            fprintf(fptr, "%c", charset[index]);
        }
        fprintf(fptr, "\n");
    }

    fclose(fptr);
}

void help(char *argv[]) {
    fprintf(stderr, "\n \n Usage: %s  <options> <input_image> \n \n", argv[0]);
	fprintf( stderr,
			" options: \n \n"
			" -h   show this message \n"
			" -n    sets natural luminance uses the CIE standard values for percieved luminance \n"
			" -g <number(float)>  sets a custum gamma, 1.8 seems to be about right\n"
			" -x set custom width value for the image \n"
			" -y set a custom height value for the image \n "
			" -f keep the original image resolution (this will result in a massive ascii wall) \n"
			" -c < .:-=+*#%@> set a custom character set size of 10 use _ in place of <space> \n"
			" -o <name_of_output_file.txt>  ( by default image is printed to terminal) \n>"

	);
}

int main(int argc, char *argv[]) {

	int x,y;
	static int n;
	int output_h, output_w;
	bool natural = false;
	bool custom_gamma = false;
	bool output_file = false;
	bool resize = true;
	bool custom_resize =false;
	float gamma = 2.2f;
	char *charset = " .:-=+*#%@";
	char *filename = NULL;
	int opt;


while ((opt = getopt(argc, argv, "hng:x:y:fc:o:")) != -1) {
	switch (opt) {
			case 'h':
				help(argv);
				return 1;
            case 'n':
                natural = true;
                break;
            case 'g':
				custom_gamma = true;
                gamma = atof(optarg);
                break;
            case 'o':
				output_file = true;
                filename = optarg;
                break;

			case 'x':
				custom_resize = true;
				output_w = atoi(optarg);  // convert string to int
				if (output_w <= 0) {
					fprintf(stderr, "Error: custom width must be a positive integer.\n");
					return 1;
				}
				break;
			case 'y':
				custom_resize = true;
				output_h = atoi(optarg);
				if (output_h <= 0) {
					fprintf(stderr, "Error: custom height must be a positive integer.\n");
					return 1;
				}
				break;
			case 'f':
				resize=false;
				break;

			case 'c':
				if (strlen(optarg) == 10) {  // Validate length
                    charset = optarg;
					for (int i = 0; i < 10; i++) {
						if (charset[i] == '_') {
							charset[i] = ' ';
						}
					}
                } else {
                    fprintf(stderr, "Error: character set must be exactly 10 characters, use _ instead of space\n");
                    return 1;
                }
                break;
            default: /* '?' */
                help(argv);
                return 1;
        }
    }
	if (optind >= argc) {
        fprintf(stderr, "Error: input image not specified.\n");
        help(argv);
        return 1;
    }

	//load image
	// After getopt and checking optind
	char *input_image = argv[optind];  // correct image path
	unsigned char *data = stbi_load(input_image, &x, &y, &n, 3);

	// error check
	if (!data) {
		fprintf(stderr, "Failed to load image: %s\n", input_image);
		return 1;
	}

	if (!custom_resize) {
    output_w = 100;
    output_h = (int)ceilf((float)output_w * (float)y / (float)x);  // preserve aspect ratio
}
		//lets check for resizing first

		unsigned char *resized_data = malloc((size_t)output_w * (size_t)output_h * 3u);

	if (resize){
		stbir_resize_uint8_srgb(
			data, x, y, x * 3,           // input
			resized_data, output_w, output_h, output_w * 3, 3  // output
		);
		stbi_image_free(data);
		data = resized_data;
		x = output_w; y = output_h;

	}

	//declare rest
	int idx;
	int *pixel = malloc((x+1) * (y+1) * 3 * sizeof(int));		//we use malloc to avoid stack overflow
	float *linpixel = malloc((x+1) * (y+1) * 3 * sizeof(float)); //TT yes this is a retroactive change
	float **lumi = malloc((x+1) * sizeof(float *));   // allocate x+1
	for (int i = 0; i <= x; i++) {
		lumi[i] = malloc((y+1) * sizeof(float));  // each pointer gets a row of y+1 floats
	}




	//convert data into something usable we start indexing from 1 because i am not insane
	//we put debug values into [0][0][0] specically the image size in total so pretty much the [x] [y] [3]
	malpic(pixel, 0, 1, 0) = x;
	malpic(pixel, 1, 0, 0) = y;
	malpic(pixel, 0, 0, 1) = 3;
	for (int j = 1; j <= y; j++) {
    		for (int i = 1; i <= x; i++) {
        		idx = ((j-1) * x + (i-1)) * 3;
			    malpic(pixel, i, j, 0) = data[idx + 0]; // R
        		malpic(pixel, i, j, 1)  = data[idx + 1]; // G
        		malpic(pixel, i, j, 2)  = data[idx + 2]; // B
    		}
	}

	//now i convert it into an array of luminance values lumi[x][x] = luminance.
	//for that we use a linear function to first conver into linear values (i dont know either but stack overflow said so)

	//first lets get the debug
	malpic(linpixel, 0, 1, 0) = x;
	malpic(linpixel, 1, 0, 0) = y;
	malpic(linpixel, 0, 0, 1) = 3;

	for (int j = 1; j <= y; j++) {
    		for (int i = 1; i <= x; i++) {
				malpic(linpixel, i, j, 0) = makelin( malpic(pixel, i, j, 0) / 255.0f);
        		malpic(linpixel, i, j, 1) = makelin(malpic(pixel, i, j, 1) / 255.0f);
        		malpic(linpixel, i, j, 2) = makelin(malpic(pixel, i, j, 2) / 255.0f);
			}
		}
		//so basically this is the linear values for each pixel. now we need to convert that into luminescense
		// Y = (0.2126 * sRGBtoLin(vR) + 0.7152 * sRGBtoLin(vG) + 0.0722 * sRGBtoLin(vB)) <- this is the equation for that in pseudo

			for (int j = 1; j <= y; j++) {
				for (int i = 1; i <= x; i++) {
					lumi[i][j] = ((0.2126 * malpic(linpixel, i, j, 0) + 0.7152 * malpic(linpixel, i, j, 1) + 0.0722 * malpic(linpixel, i, j, 2)) * 100);
					}
			}

	//now we try and get the perceptual values. our eyes do not percieve stuff in a linear way this is optional
			if (natural){
				for (int j = 1; j <= y; j++) {
					for (int i = 1; i <= x; i++) {
						lumi[i][j] = n_makeperlumi(lumi[i][j]);
					}
				}
			}
			else if (custom_gamma){
				for (int j = 1; j <= y; j++) {
					for (int i = 1; i <= x; i++) {
						lumi[i][j] = g_makeperlumi(lumi[i][j], gamma);
					}
				}
			}

		if(output_file){
			outfile( filename ,x, y, lumi, charset);
		}
		else{
			outterm( x, y, lumi, charset);
		}

		stbi_image_free(data);
		free(pixel);
		free(linpixel);
		for (int i = 0; i <= x; i++) free(lumi[i]);
		free(lumi);


	return 0;
}
