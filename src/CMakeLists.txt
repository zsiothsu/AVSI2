#model name
project(Interpreter)

#add all source file to variable HELLOWORLD_SRC
aux_source_directory(. INTERPRETER_SRC)

#create static library
add_library(${PROJECT_NAME} STATIC ${INTERPRETER_SRC})