--------------------------------------------------------------------------------------------------------------
Non-basic scene graphs
--------------------------------------------------------------------------------------------------------------
Caitlin Donahue
Liv Phillips
Computer Graphics, Winter 2017

--------------------------------------------------------------------------------------------------------------
Files updated:
--------------------------------------------------------------------------------------------------------------
1000camera.c
1000matrix.c
1000scene.c
1000skybox.c

Main Files:
1000mainShadowing.c
1000mainSwitchLights.c
1100mainLOD.c
1200mainState.c

--------------------------------------------------------------------------------------------------------------
Added features:
--------------------------------------------------------------------------------------------------------------
Abstracted camera and lights into the scene graph
	-This allows the camera and lights to be stored in a more logical manner, and for viewing Camera-1 to be passed through the graph instead of the identity matrix.
	 The camera acts as the root node
	-Lights are also stored in the scene graph, As siblings of the root node
Separated Transformation and Geometry nodes
	-This way not all nodes have to include a transformation. Also, one node can be repeated multiple times with different translations. (i.e. a tree can be rendered multiple times, while only initializing its node one time).
Added LOD nodes, Switch nodes, and State nodes to the scene graph
	-LOD nodes choose their child based off of distance from the camera.
	-Switch nodes allow the user to manually change the child a node points to based off of a set of children. 
	-State nodes pass certain information onto their children, such as textures/whether to render or not
Added a skybox

--------------------------------------------------------------------------------------------------------------
1000mainSwitchLights.c
--------------------------------------------------------------------------------------------------------------
clang 1000mainSwitchLights.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation

This demonstrates how switch nodes work. Instead of having a light node as a sibling to the camera. a switch node is. The switch node cycles through six light stages (using 12 children for a smooth cycle), to simulate light and day. The switch changes every second. 

--------------------------------------------------------------------------------------------------------------
1100mainLOD.c
--------------------------------------------------------------------------------------------------------------
clang 1100mainLOD.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation

This demonstrates how LOD nodes work. There are LOD nodes on both nodes of the trees with three intervals each, showing less 
detailed meshes the further away the user gets. The scene automatically backs up to show you all three intervals, but then 
the user can control from there.

-> Note: we attempted to add the functionality you described (where objects pop to the background when far enough away)
	However, we struggled to understand how to do that, since the only way we could think of would have involved
	using a framebuffer. Since the point of LOD is to minimize uneccessary rendering, we thought that path would be fruitless.

--------------------------------------------------------------------------------------------------------------
1200mainState.c
--------------------------------------------------------------------------------------------------------------
clang 1200mainState.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation

This demonstrates how state nodes work. There are four state nodes in this scene: one controls whether the trees render or not,
one controls the texture of the trees, one controls texture of the water, and one controls texture of the land.
Trees flicker on/off 9 times, then only the textures change state.

