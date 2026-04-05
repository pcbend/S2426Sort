
#this will compile the current library.
add_library(HISTOGRAMER SHARED Histogramer.cxx)
root_generate_dictionary(G__Histogramer Histogramer.h MODULE HISTOGRAMER LINKDEF Linkdef.h)
target_link_libraries(HISTOGRAMER ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort HISTOGRAMER)        


