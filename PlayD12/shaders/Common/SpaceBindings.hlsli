#define SPACE_BINDINGS_HLSLI
#ifndef SPACE_BINDINGS_HLSLI 

//b0, space0:  per-frame / per-scene:


//b1, space 0: per-object 


//b2 , space 0: per-material 


#define SPACE_FRAME space0
#define BINDING_FRAME_CBV b0

#define SPACE_OBJECT space0
#define BINDING_OBJECT_CBV b1

#define SPACE_MAT space0
#define BINDING_MAT_CBV b2

#endif