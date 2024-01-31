The Events Subsystem handles events and bindings via a TMap in which for each tag there are 
several associated functions stored as a struct that works as a pair of UObject (the object to call the 
function on) and the UFunction (like a function pointer) on that specified object. To properly use the 
subsystem there are three simple steps:

1) First, a channel needs to be created with a tag associated to it. As listed in the nodes below 
it is necessary to create a channel via tag and if the channel already exists, none will be 
created, and the user will be notified.

3) To bind this object and the function that will be triggered, “Try bind function by channel” 
must be called. We need to pass a couple of parameters to it: the channel tag (basically says 
to which channel this function and this object will be bound), the function name which must 
be identical to the function name specified on the object bound and last the object itself.

4) Last, to broadcast an event to that channel “Broadcast channel event by tag params” must 
be called. There are two different functions that can be called to broadcast an event: the 
one listed above and “Broadcast channel event by tag no params”. The two differ from the 
fact that the first one has parameters (as wildcards, so everything can be potentially passed 
–multiple parameters must be wrapped in a custom struct-) that can be passed to the node. 
The second doesn’t need any parameter. Whenever “Broadcast channel event by tag 
params” is called, some parameters will be broadcasted to the functions bound to that 
channel although only the ones that accepts that specific type of parameter will be called. 
This prevents the wrong functions to be called and allows functions with different 
parameters to be bound to the same channel without causing unnecessary mistakes.
