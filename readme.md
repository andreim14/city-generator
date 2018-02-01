# City-Gnerator
This poject is based on [Yocto/GL](https://github.com/xelatihy/yocto-gl) Library, introducing a random city generator. Feature include:

- Random building and street generation
- Possibility to choose:
    - City size
    - Maximum number of floor of the buildings
    - How much the streets has to be forked
    - The chance to place a building
    - The chance to place trees
    - The urbanization (explain later)
    - Normal or Sunset mode
- Saving the scene to Obj file

This generator creates a bidimentional matrix and populates it as follows. 
At the beginning roads are created using a sort of random walk algorithm. The algorithm "walks" in a direction and at each step has some probaility to fork in some perpendicular direction (or double fork). If it forks, it will "walk" in the new direction and the fork probability will be Halved. When it reaches the border of the matrix or some other critical case (such as a street parallel to this one near it) it will stop and the algorithm will continue where it forked. 
The result is a random road network that contains crossroads and can be more or less complex it depends on the input fork-probability. 
After that we will have a matrix that contains the roads and we need to place buildings and trees. The algorithm will then place buildings near the roads with the input building-creation-probability, otherwise will place trees with the input tree-creation-probability and will check the crossroads or the end of road placing the right model.

The buildings are recursively generated.  At each step a random floor, base or roof will be choose. Managing the compatibility between the models and the position of them.

The urbanization let the user choose how the city will look like so will it have only big buildings or small houses? Starting from the center of the city to the borders there will be more or less high buildings and trees it depends on the value of the input urbanization.

The sunset mode is a special mode that lets will create a sunset atmosphere making your generated city look really cool.

## Outputs and generation parameters:
![Image](images/example_2048px_size_50-sc_50-bc_40-u_80-sunset-mode.png)
    
    `./bin/city_generator -size 50 -sc 50 -bc 40 -u 80 -s`
    
![Image](images/example_1024px_size_50-sc_50-bc_35-u_70.png)
    
    `./bin/city_generator -size 50 -sc 50 -bc 35 -u 70 `

![Image](images/example_1024px_size_30-sc_60-bc_50-u_70.png)
    
    `./bin/city_generator -size 30 -sc 60 -bc 50 -u 70 `
    
![Image](images/example_1024px_size_30-sc_60-bc_50-u_0.png)
    
    `./bin/city_generator -size 30 -sc 60 -bc 50 -u 0 `

![Image](images/example_1024px_size_30-sc_60-bc_50-u_100.png)
    
    `./bin/city_generator -size 30 -sc 60 -bc 50 -u 100 `
    
You can find more examples in images folder.

## Building and Running:

Build:

    ./build.sh

Run:

    ./bin/city_generator
For the prerequisites look at [Yocto/GL](https://github.com/xelatihy/yocto-gl#compilation)
 
### Parameters:

- `-size int` to specify the size of the city `default = 30`

- `-m int` to specify the minimum number of floors of the buildings of the city `default = 1`

- `-M int` to specify the maximum number of floors of the buildings of the city `default = 5`

- `-sc int` to specify the probability for the roads to fork `default = 30`

- `-bc int` to specify the probability for a building to be placed `default = 50`

- `-tc int` to specify the probability for a tree to be placed `default = 30`

- `-u int` to specify the urbanization level of the city `default = 75`

- `-s` to activate the sunset mode

- `-o string` to specify the output filename where the scene  `default = "scene_out.obj"`

Example:

    ./bin/city_generator -size 40 -tc 30 -sc 60 -bc 50 -u 70 -o generated_city.obj

### Street gneration example:

The generation of the streets can be controlled by adjusting the `-sc` parameter those are some outputs:

![Image](images/possible_street_generation.png)

    `with high split chance`
    
![Image](images/possible_street_generation_3.png)
    
    `with 0 as split chance`
    
![Image](images/possible_street_generation_2.png)
    
    `with default as split chance`
    
### Authors

[Andrei Stefan Bejgu](https://github.com/andreim14)
    
### Credits
[Fabio Pellacini](https://github.com/xelatihy) for the [Yocto/GL](https://github.com/xelatihy/yocto-gl) library.
