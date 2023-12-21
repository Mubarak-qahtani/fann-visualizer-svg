#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include "fann.h"

#include "svg.c"

/*
	//~gcc -Wall visualize.c -o visualize -lgd -lfann
	gcc -Wall visualize.c -o visualize-lfann
*/

/***
TODO: change connection color to represent weight

*/

/// constants
int neuron_size = 50; // height & width in px
//~int neuron_size = 25; // height & width in px

//~int tiles_between_neurons = 2;
int tiles_between_neurons = 1;
//~int tiles_between_neurons = 3;
//~int tiles_between_neurons = 6;

//~int tiles_between_layers = 3;
int tiles_between_layers = 5;
//~int tiles_between_layers = 15;

int weight_size = 3;


unsigned int num_inputs;
unsigned int num_outputs;
unsigned int total_neurons;
unsigned int num_layers;
unsigned int *layers_array;
unsigned int *bias_array;
unsigned int total_connections;
struct fann_connection *connections_array;

unsigned int largest_layer;

enum type_of_neuron { input, output, hidden, bias};

struct neuron
{
	unsigned int index;
	unsigned int layer;
	enum type_of_neuron type;
	float x,y;
};

struct neuron *my_neurons;


unsigned int array_sum(unsigned int const *array, int array_size)
{
	unsigned int sum = 0;
	for (int i=0; i<array_size; i++)
	{
		sum += array[i];
	}
	return sum;
}

unsigned int max_in_array(unsigned int const *array, unsigned int const *array2, int array_size)
{
	unsigned int max = 0;
	for(int i=0; i<array_size; i++)
	{
		if ((array[i] + array2[i]) > max)
			max = array[i] + array2[i];
	}
	return max;
}

int save_output_image(char *filename, bool draw_grid, bool random_connection_color, 
									bool draw_connection_weights, bool horizental_view, bool output_stats_image)
{
	
	if(horizental_view == true)	// horizental_view
	{
		;/// XXX: ?!!!!
	}
	
	// Create a Blank Image
	int image_width = (neuron_size * ((signed int)largest_layer + (tiles_between_neurons / 2))) * tiles_between_neurons - (neuron_size) + 1;
	int image_height = (neuron_size * num_layers) * tiles_between_layers - (neuron_size * (tiles_between_layers - 1)) + 1;
	//~printf("image_width = %d\n", image_width);
	//~printf("image_height = %d\n", image_height);
	
	svg* neural_network_image;
	//~neural_network_image = svg_create(image_width, image_height);
	neural_network_image = svg_create(image_width, image_height + 100);
	
	if (!neural_network_image)
	{
		printf("Error in svg_create\n");
		exit(EXIT_FAILURE);
	}
	
	// Colors Array
	char background_color[] = "#999999";
	char grid_color[] = "#808080";
	char neuron_stroke_color[] = "#5A5A5A";
	char input_neuron_color[] =  "#AAFFAA";
	char hidden_neuron_color[] =  "#E9AFAE";
	char output_neuron_color[] = "#ABCCFF";
	char bias_neuron_color[] = "#FFFF00";
	char positive_connection_weight_color[] = "#00FF00";
	char negitive_connection_weight_color[] = "#FF0000";
	char dead_connection_weight_color[] = "#000000";
	
	// Paint Background
	svg_fill(neural_network_image, background_color);
	
	printf("1\n");
	// Draw Grid if draw_grid is set to true 
	if(draw_grid == true)
	{
		for (int y=0; y<image_height; y+=neuron_size)
		{
			svg_line(neural_network_image, grid_color, 1, 0, y, image_width, y);
		}
		for (int x=0; x<image_width; x+=neuron_size)
		{
			svg_line(neural_network_image, grid_color, 1, x, 0, x, image_height);
		}
	}
	
	printf("2\n");
	// Draw Connections
	/// TODO: fix it, very slow, when we have alot of connections
	for (unsigned int i=0; i<total_connections ;i++)
	{
		float weight = connections_array[i].weight;
		int from_neuron = connections_array[i].from_neuron;
		int to_neuron = connections_array[i].to_neuron;
		
		char *color = neuron_stroke_color;			// What color is the connection
		if(random_connection_color == false)
		{
			if(weight > 0.0)
			{
				color = positive_connection_weight_color;
			}
			else if(weight < 0.0)
			{
				color = negitive_connection_weight_color;
			}
			else
			{
				color = dead_connection_weight_color;
			}
		}
		else
		{
			color = malloc(sizeof(char)*8);
			sprintf(color, "#%06x", (int)(drand48()*(16777215.0)));
		}
		
		int thickness;
		// Set connection thickness
		if(draw_connection_weights == true)
		{
			thickness = (int)ceil(fabs(weight));
			//~thickness = (int)(drand48()*(double)(weight_size));
			if(thickness > weight_size)
			{
				thickness = 5;
			}
		}
		else
		{
			thickness = 1;
		}
		
		// Draw connection
		svg_line(neural_network_image, color, thickness, my_neurons[from_neuron].x, my_neurons[from_neuron].y, my_neurons[to_neuron].x, my_neurons[to_neuron].y);
		//~svg_bezier(neural_network_image, color, thickness, my_neurons[from_neuron].x, my_neurons[from_neuron].y, my_neurons[to_neuron].x, my_neurons[to_neuron].y);
		if (random_connection_color == true)
			free(color);
	}

	printf("3\n");
	
	// Draw Neurons
	for (unsigned int key=0; key<total_neurons ; key++)
	{
		char *color;
		if(my_neurons[key].type == input)
		{
			color = input_neuron_color;
		}
		else if(my_neurons[key].type == hidden)
		{
			color = hidden_neuron_color;
		}
		else if(my_neurons[key].type == output)
		{
			color = output_neuron_color;
		}
		else if(my_neurons[key].type == bias)
		{
			color = bias_neuron_color;
		}
		else
		{
			printf("error in color \n");
			color = 0;
		}
		
		svg_circle (neural_network_image, neuron_stroke_color, 1, color, neuron_size / 2, my_neurons[key].x, my_neurons[key].y);
	}
	
	printf("4\n");
	
	if(output_stats_image == 1)
	{
		// Colors Array
		//char stats_background_color[] = "#999999";
		char inputs_text_color[] =  "#AAFFAA";
		char hidden_text_color[] =  "#E9AFAE";
		char outputs_text_color[] = "#ABCCFF";
		char bias_text_color[] = "#FFFF00";
		char layers_text_color[] = "#008080";
		char connections_text_color[] = "#804000";
		
		int x = 25;
		int y = image_height;
		int font_size = 15;
		int increment = font_size + 3;
		
		char tmp_text[64];
		
		sprintf(tmp_text, "%u Inputs", num_inputs);
		svg_text(neural_network_image, x, y, "sans-serif", font_size, inputs_text_color, inputs_text_color, tmp_text);
		y += increment;
		
		sprintf(tmp_text, "%u Hidden", total_neurons - (num_inputs + num_outputs + array_sum(bias_array, num_layers)));
		svg_text(neural_network_image, x, y, "sans-serif", font_size, hidden_text_color, hidden_text_color, tmp_text);
		y += increment;
		
		sprintf(tmp_text, "%u Outputs", num_outputs);
		svg_text(neural_network_image, x, y, "sans-serif", font_size, outputs_text_color, outputs_text_color, tmp_text);
		y += increment;
		
		sprintf(tmp_text, "%u Bias", array_sum(bias_array, num_layers));
		svg_text(neural_network_image, x, y, "sans-serif", font_size, bias_text_color, bias_text_color, tmp_text);
		y += increment;
		
		sprintf(tmp_text, "%u Layers", num_layers);
		svg_text(neural_network_image, x, y, "sans-serif", font_size, layers_text_color, layers_text_color, tmp_text);
		y += increment;
		
		sprintf(tmp_text, "%u Connections", total_connections);
		svg_text(neural_network_image, x, y, "sans-serif", font_size, connections_text_color, connections_text_color, tmp_text);
	}
	
	svg_finalize(neural_network_image);
	svg_save(neural_network_image, filename);
	svg_free(neural_network_image);
	
	printf("5\n");
	
	return 0;
}

void neuron_positions()
{
	// Determine Neuron X,Y Position
	int row = 1; // y
	/// TODO: fix 'col' issue
	int col = 1 + (int)(ceilf((float)largest_layer / 2.0) - ceilf((float)(layers_array[0] + bias_array[0]) / 2.0)) * tiles_between_neurons; // x
	//~int col = (1 + (int)(ceilf((float)largest_layer / 2.0f - (float)(layers_array[0] + bias_array[0]) / 2.0f))) * tiles_between_neurons;
	//~printf("col = %d\n", col);
	for(unsigned int index=0; index < total_neurons; index++)
	{
		if(index > 0 && my_neurons[index].layer > my_neurons[index - 1].layer)
		{
			row += tiles_between_layers;
			
			/// TODO: fix 'col' issue
			//~col = 2 + (int)(ceilf((float)largest_layer / 2.0f)) * tiles_between_neurons
								//~- ceilf((float)(layers_array[my_neurons[index].layer] + bias_array[my_neurons[index].layer]) / 2.0f) * tiles_between_neurons;
			//~col = 2 + (int)(ceilf((float)largest_layer / 2.0f)
								//~- ceilf((float)(layers_array[my_neurons[index].layer] + bias_array[my_neurons[index].layer]) / 2.0f)) * tiles_between_neurons;
			//~col = 2 + (int)(ceilf((float)largest_layer / 2.0f - (float)(layers_array[my_neurons[index].layer] + bias_array[my_neurons[index].layer]) / 2.0f)) * tiles_between_neurons;
			col = (1 + (int)(ceilf((float)largest_layer / 2.0f - (float)(layers_array[my_neurons[index].layer] + bias_array[my_neurons[index].layer]) / 2.0f))) * tiles_between_neurons;
			//~printf("col = %d\n", col);
		}
		else
		{
			col += tiles_between_neurons;
		}
		
		my_neurons[index].x = (neuron_size * col) - (neuron_size / 2);
		my_neurons[index].y = (neuron_size * row) - (neuron_size / 2);
	}
}


int main(int argc, char **argv)
{
	srand48(time(0));
	
	/// NOTE: drawing connection's weights, it slows the program if you have alot of connections
	//~bool draw_connection_weights = false;
	bool draw_connection_weights = true;
	
	// In densely connected networks it can be difficult to see all the connections so randomizing the 
	// connection color can improve the visibility some.
	// true = random color
	// false = colors: red negitive, green positive
	bool random_connection_color = true;
	
	bool draw_grid = true;
	// Output a second image with the ANN stats?
	bool output_stats_image = true;
	
	// true  : left to right
	// false : top to bottom
	bool horizental_view = true;
	
	struct fann *ann;
	
	if(argc < 2)
	{
		//~printf("Usage: %s NETWORK_FILENAME\n", argv[0]);
		//~exit(EXIT_SUCCESS);
		
		ann = fann_create_from_file("ocr_float.net"); //Load ANN
	}
	else
	{
		ann = fann_create_from_file(argv[1]); //Load ANN
	}
	
	if(ann == NULL)
	{
		printf("Error in reading NET file\n");
		exit(EXIT_FAILURE);
	}
	
	num_inputs = fann_get_num_input(ann);
	num_outputs = fann_get_num_output(ann);
	total_neurons = fann_get_total_neurons(ann);
	//~printf("total_neurons = %d\n", total_neurons);
	
	num_layers = fann_get_num_layers(ann);
	layers_array = calloc(num_layers, sizeof(unsigned int));
	fann_get_layer_array(ann, layers_array);
	
	bias_array = calloc(num_layers, sizeof(unsigned int));
	fann_get_bias_array(ann, bias_array);
	
	for (unsigned int i=0; i < num_layers; i++)
	{
		printf("layer[%d] = %d neurons, %d biases\n", i, layers_array[i], bias_array[i]);
	}
	
	total_connections = fann_get_total_connections(ann);
	connections_array = malloc(sizeof(struct fann_connection) * total_connections);
	fann_get_connection_array(ann, connections_array);
	
	int type_of_network = fann_get_network_type(ann);
	//~printf("type of network: %s \n", FANN_NETTYPE_NAMES[fann_get_network_type(ann)]);
	if (type_of_network == FANN_NETTYPE_LAYER)
	{	/* Each layer only has connections to the next layer */
		;//
	}
	else if (type_of_network == FANN_NETTYPE_SHORTCUT)
	{	/* Each layer has connections to all following layers */
		;//
	}
	else
	{	// error: there are no third type of network in this version of FANN
		printf("Error in Network type\n");
		fann_destroy(ann);
		exit(EXIT_FAILURE);
	}
	
	fann_destroy(ann);
	
	my_neurons = (struct neuron *)malloc(sizeof(struct neuron) * total_neurons);
	//~my_neurons = (struct neuron *)calloc(total_neurons, sizeof(struct neuron));
	for (unsigned int i=0; i<total_neurons ;i++)
	{
		my_neurons[i].index = i;
	}
	
	// Figure out which neuron belongs on which layer, Assign it a type: ['input', 'output', 'hidden', 'bias']
	unsigned int current_neuron = 0;
	for (unsigned int layer=0; layer < num_layers; layer++)
	{
		// Detect Input, Output & Hidden Neurons 
		unsigned int i;
		for(i = current_neuron; i < (current_neuron + layers_array[layer]); i++)
		{
			my_neurons[i].layer = layer;
			if(layer == 0)
			{
				my_neurons[i].type = input;
			}
			else if(layer == (num_layers - 1))
			{
				my_neurons[i].type = output;    
			}
			else
			{
				my_neurons[i].type = hidden;
			}
		}
		
		current_neuron = i;
		// Detect Bias Neurons 
		for(i = current_neuron; i < (current_neuron + bias_array[layer]); i++)
		{
			my_neurons[i].layer = layer;
			my_neurons[i].type = bias;
		}
		current_neuron = i;
		
		//~printf("layer = %d, i = %d \n", layer, i);
		assert (i <= total_neurons);
	}
	
	// Get largest_layer (neurons + bias neurons)
	largest_layer = max_in_array(layers_array, bias_array, num_layers);
	
	// Determine Neuron X,Y Position
	neuron_positions();
	
	save_output_image("ann_name.svg", draw_grid, random_connection_color, draw_connection_weights, horizental_view
								, output_stats_image);
	
	free(my_neurons);
	free(layers_array);
	free(bias_array);
	free(connections_array);
	
}
