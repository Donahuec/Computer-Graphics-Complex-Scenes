--------------------------------------------------------------------------------------------------------------
Non-basic scene graphs
--------------------------------------------------------------------------------------------------------------
Caitlin Donahue
Live Phillips
Computer Graphics, Winter 2017

--------------------------------------------------------------------------------------------------------------
Files updated:
--------------------------------------------------------------------------------------------------------------
1000camera.c
1000matrix.c
1003scene.c
1000skybox.c

Main Files:
1000mainShadowing.c
1000mainSwitchLights.c
...?

--------------------------------------------------------------------------------------------------------------
Added features:
--------------------------------------------------------------------------------------------------------------
Abstracted camera and lights into the scene graph
	-This allows the camera and lights to be stored in a more logical manner, and for viewing Camera-1 to be passed through the graph instead of the identity matrix.
	 The camera acts as the root node
	-Lights are also stored in the scene graph, As siblings of the root node
Separated Transformation and Geometry nodes
	-This way not all nodes have to include a transformation. Also, one node can be repeated multiple times with different translations. (i.e. a tree can be rendered multiple times, while only initializing its node one time).
Added LOD nodes and Switch nodes to the scene graph
	-LOD nodes choose their child based off of distance from the camera.
	-Switch nodes allow the user to manually change the child a node points to based off of a set of children. 
Added a skybox

--------------------------------------------------------------------------------------------------------------
1000mainSwitchLights.c
--------------------------------------------------------------------------------------------------------------
clang 1000mainSwitchLights.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation

This demonstrates how switch nodes work. Instead of having a light node as a sibling to the camera. a switch node is. The switch node cycles through six light stages (using 12 children for a smooth cycle), to simulate light and day. The switch changes every second. 