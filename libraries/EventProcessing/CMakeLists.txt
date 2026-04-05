
#this will compile the current library.
add_library(EVENTBUILDER SHARED EventBuilder.cxx EventProcess.cxx)
#root_generate_dictionary(G__EventBuilder EventBuilder.h MODULE EVENTBUILDER LINKDEF Linkdef.h)
target_link_libraries(EVENTBUILDER ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort EVENTBUILDER)        


