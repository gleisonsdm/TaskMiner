#ifndef EDGE_DEP_TYPE_ENUM_H
#define EDGE_DEP_TYPE_ENUM_H

// EdgeTypes for the dependency graphs and the Region Tree
enum EdgeDepType {RAR, 
									WAW, 
									RAW, 
									WAR, 
									CTR, 
									SCA, 
									RAWLC, 
									PARENT, 
									RECURSIVE,
									FCALL};

#endif // EDGE_DEP_TYPE_ENUM_H