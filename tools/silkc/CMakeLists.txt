add_executable (silkc silkc.cpp)
set_target_properties (silkc PROPERTIES LINK_FLAGS ${LLVM_LFLAGS})
target_link_libraries (silkc SilkVMCore SilkDecil SilkSupport)
target_link_libraries (silkc ${LLVM_LIBS})
target_link_libraries (silkc "-ldl -lpthread")
