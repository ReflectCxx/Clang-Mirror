# Source files
set(CLREFLECT_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/ASTParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ClangDriver.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ClangReflectActionFactory.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ClangReflectDiagnosticConsumer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/main.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/FindRecordDeclsVisitor.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Logger.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/MetaFunction.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ReflectableInterface.cpp"
    # Add more source files as needed
)

target_sources(${CXX_EXE_NAME} PRIVATE ${CLREFLECT_SRCS})