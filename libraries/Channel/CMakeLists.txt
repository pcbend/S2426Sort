
#this will compile the current library.
add_library(CHANNEL SHARED Channel.cxx)
#root_generate_dictionary(G__Channel Channel.h MODULE CHANNEL LINKDEF Linkdef.h)
target_link_libraries(CHANNEL ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort CHANNEL)        


