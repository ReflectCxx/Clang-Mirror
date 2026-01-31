# Header files
set(CLREFLECT_INCS
    "${CMAKE_CURRENT_LIST_DIR}/ASTParser.h"
	"${CMAKE_CURRENT_LIST_DIR}/ASTParserUtils.h"
    "${CMAKE_CURRENT_LIST_DIR}/ClangDriver.h"
	"${CMAKE_CURRENT_LIST_DIR}/ClangReflectActionFactory.h"
	"${CMAKE_CURRENT_LIST_DIR}/ClangReflectDiagnosticConsumer.h"
	"${CMAKE_CURRENT_LIST_DIR}/ReflectableDeclsUtils.h"
	"${CMAKE_CURRENT_LIST_DIR}/ReflectableDeclsVisitor.h"
	"${CMAKE_CURRENT_LIST_DIR}/Logger.h"
	"${CMAKE_CURRENT_LIST_DIR}/Constants.h"
	"${CMAKE_CURRENT_LIST_DIR}/StringUtils.h"
	"${CMAKE_CURRENT_LIST_DIR}/ReflectionMeta.h"
	"${CMAKE_CURRENT_LIST_DIR}/RtlCodeManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/RtlCodeGenerator.h"
)

# Add headers to the library target
target_sources(${CXX_EXE_NAME} PRIVATE ${CLREFLECT_INCS})
