# Written by Jelle Geerts (jellegeerts@gmail.com).
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software.
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

cmake_minimum_required(VERSION 2.6)

project(Gambit)

if(WIN32)
  set(GAMBIT_BINARY_NAME "gambit")
else(WIN32)
  # Use 'gambitchess' as the name for the binary, as 'gambit' is already used
  # by a different and unrelated project. This is done to prevent filename
  # collisions, so the 'gambitchess' binary can be installed in the same
  # directory (probably '/usr/bin') as the directory containing 'gambit'.
  set(GAMBIT_BINARY_NAME "gambitchess")
endif(WIN32)

# Description:
#   Whether this build is an official version.
#
# Remarks:
#   This option should be set to ON whenever one intends to build an official
#   release.
#
#   Note that one shouldn't have to change this option manually. If the code is
#   from an official version, this option should already be set to ON.
#
# Example values:
#   ON
#   OFF
option(CONFIG_OFFICIAL_VERSION "whether this build is an official version" OFF)
message(STATUS "Value for CONFIG_OFFICIAL_VERSION: ${CONFIG_OFFICIAL_VERSION}")

# Description:
#   Whether to enable the update checker.
#
#   This option is provided mainly for Unix platforms.
#
# Remarks:
#   On Unix platforms for which a package of this program is actively
#   maintained, it can be useful to disable the program's update checker, so
#   users get updates via the package system, and won't be informed by the
#   program itself about updates.
#
# Example values:
#   ON
#   OFF
option(CONFIG_ENABLE_UPDATE_CHECKER "whether to enable the update checker" ON)
message(STATUS "Value for CONFIG_ENABLE_UPDATE_CHECKER: ${CONFIG_ENABLE_UPDATE_CHECKER}")

# Description:
#   Absolute path of the directory containing the Gupta engine binary 'gupta'.
#   Gupta is Gambit's own engine. It's a separate binary.
#
#   This option is provided mainly for Unix platforms.
#
# Remarks:
#   Note that the value of this option, if provided, generally should _not_ end
#   with a slash.
#
#   This option is not mandatory.
#
# Example value:
#   /usr/bin
option(CONFIG_GUPTA_ENGINE_DIRECTORY "absolute path of directory containing the gupta binary" OFF)
message(STATUS "Value for CONFIG_GUPTA_ENGINE_DIRECTORY: ${CONFIG_GUPTA_ENGINE_DIRECTORY}")

# Description:
#   Prefix for resource paths.
#   Resource paths are used to find image files, translation files, etc.
#
#   This option is provided mainly for Unix platforms.
#
# Remarks:
#   Note that the value of this option, if provided, generally _should_:
#     - end with a slash, and
#     - be an absolute path.
#
#   This option is not mandatory.
#
# Example value:
#   /usr/share/games/gambit/
option(CONFIG_RESOURCE_PATH_PREFIX "prefix for resource paths" OFF)
message(STATUS "Value for CONFIG_RESOURCE_PATH_PREFIX: ${CONFIG_RESOURCE_PATH_PREFIX}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

include(cmake/functions/remove_debug_libraries_from_list.cmake)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wstrict-prototypes -Wbad-function-cast -Wformat=2 -Wundef -pedantic -Wno-long-long")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wredundant-decls -Wformat=2 -Wundef -pedantic -Wno-long-long")

set(CMAKE_C_FLAGS_DEBUG
  "-g -O -D DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG
  "-g -O -D DEBUG")
set(CMAKE_C_FLAGS_RELEASE
  "-O3 -D NDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE
  "-O3 -D NDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE
  "-s")

if(WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D _WIN32_WINNT=0x0500")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mthreads")
else(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif(WIN32)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

include_directories(src src/sdk)

# temporary workaround until CMake supports RC files for the MinGW toolchain
if(MINGW)
  set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> -O coff <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
endif(MINGW)

# target Gambit
#******************************************************************************

add_definitions(-D CONFIG_SETTINGS_BACKEND_USE_QT)

# Pass on the CMake option() values to the source code.
if("${CONFIG_OFFICIAL_VERSION}" STREQUAL "ON")
  add_definitions(-D CONFIG_OFFICIAL_VERSION=${CONFIG_OFFICIAL_VERSION})
endif()
if("${CONFIG_ENABLE_UPDATE_CHECKER}" STREQUAL "ON")
  add_definitions(-D CONFIG_ENABLE_UPDATE_CHECKER=${CONFIG_ENABLE_UPDATE_CHECKER})
endif()
if(NOT("${CONFIG_GUPTA_ENGINE_DIRECTORY}" STREQUAL "OFF"))
  add_definitions(-D CONFIG_GUPTA_ENGINE_DIRECTORY=${CONFIG_GUPTA_ENGINE_DIRECTORY})
endif()
if(NOT("${CONFIG_RESOURCE_PATH_PREFIX}" STREQUAL "OFF"))
  add_definitions(-D CONFIG_RESOURCE_PATH_PREFIX=${CONFIG_RESOURCE_PATH_PREFIX})
endif()

set(GAMBIT_SRCS
  src/Core/AbnormalTerminationHandler.cc
  src/Core/debugf.c
  src/Core/enforce.cc
  src/Core/Engine.cc
  src/Core/EngineException.cc
  src/Core/EngineManager.cc
  src/Core/Event.cc
  src/Core/EventDispatcher.cc
  src/Core/GambitApplication.cc
  src/Core/GameController.cc
  src/Core/GameControllerTimer.cc
  src/Core/GeneralException.cc
  src/Core/GnuChessEngine.cc
  src/Core/GuptaEngine.cc
  src/Core/MoveEvent.cc
  src/Core/PgnDeserializer.cc
  src/Core/Preferences.cc
  src/Core/ResourcePath.cc
  src/Core/UpdateChecker.cc
  src/Core/UpdateCheckerTimestamp.cc
  src/Core/UpdateCheckResult.cc
  src/sdk/chess_engine_mediator/ce_mediator.c
  src/sdk/NamedLock/NamedLock.cpp
  src/sdk/Settings/Backends/Qt/SettingsContainer.cc
  src/sdk/Settings/Settings.cc
  src/sdk/Settings/SettingsContainerMixin.cc
  src/sdk/Settings/SettingsElement.cc
  src/sdk/Settings/SettingsGlue.cc
  src/sdk/SignalTester/SignalTester.cc
  src/Model/Board.cc
  src/Model/CaptureInfo.cc
  src/Model/CastlingFlags.cc
  src/Model/CastlingInfo.cc
  src/Model/Coord.cc
  src/Model/EnPassant.cc
  src/Model/Game.cc
  src/Model/MoveHistory.cc
  src/Model/MoveNotation.cc
  src/Model/PgnMoveList.cc
  src/Model/PgnPlayerType.cc
  src/Model/Piece.cc
  src/Model/Ply.cc
  src/Model/Result.cc
  src/Model/Rules.cc
  src/Model/Side.cc
  src/Utils/Qt/languageIdString.cc
  src/Utils/Qt/QString_find_first_not_of.cc
  src/Utils/String/ucfirst.cc
  src/View/BoardStyle.cc
  src/View/BoardStyles.cc
  src/View/BoardView.cc
  src/View/BusyIndicatorWidget.cc
  src/View/GraphicsScene.cc
  src/View/GraphicsView.cc
  src/View/LanguageListWidget.cc
  src/View/LanguageListWidgetItem.cc
  src/View/MissingFileDialog.cc
  src/View/MoveAnimation.cc
  src/View/NotificationWidget.cc
  src/View/OptionallyPaintedLabel.cc
  src/View/PieceCaptureAnimation.cc
  src/View/PieceMovementAnimation.cc
  src/View/PreferencesDialog.cc
  src/View/ProxyAuthenticationDialog.cc
  src/View/SpriteManager.cc
  src/View/ToolBar.cc
  src/View/UI.cc
  src/main.cc)

set(MOC_HDRS
  src/Core/GambitApplication.hh
  src/Core/GameController.hh
  src/Core/UpdateChecker.hh
  src/sdk/Settings/SettingsGlue.hh
  src/sdk/SignalTester/SignalTester.hh
  src/View/BoardView.hh
  src/View/BusyIndicatorWidget.hh
  src/View/GraphicsScene.hh
  src/View/MissingFileDialog.hh
  src/View/PreferencesDialog.hh
  src/View/ProxyAuthenticationDialog.hh
  src/View/ToolBar.hh
  src/View/UI.hh)

set(UIS
  src/View/PreferencesDialog.ui
  src/View/ProxyAuthenticationDialog.ui)

if(WIN32)
  set(GAMBIT_SRCS ${GAMBIT_SRCS}
    src/resource-win32/rsrc.rc
    src/sdk/NamedLock/NamedMutex_win32.cpp
    src/sdk/procspawn/procspawn_win32.c)
else(WIN32)
  set(GAMBIT_SRCS ${GAMBIT_SRCS}
    src/sdk/NamedLock/LockFile_unix.cpp
    src/sdk/procspawn/procspawn_unix.c)
endif(WIN32)

if(WIN32)
  execute_process(COMMAND "svn_update_revision_number_header.bat" RESULT_VARIABLE script_result)
  if(NOT(script_result EQUAL 0))
   message(FATAL_ERROR "Failed to execute 'svn_update_revision_number_header.bat'.")
  endif()
else(WIN32)
  execute_process(COMMAND "sh" "svn_update_revision_number_header.sh" RESULT_VARIABLE script_result)
  if(NOT(script_result EQUAL 0))
    message(FATAL_ERROR "Failed to execute 'svn_update_revision_number_header.sh'.")
  endif()
endif(WIN32)

# Important: do not add QtOpenGL as component, otherwise the GL libraries will
# be added to link against using link flags instead of filenames. This would
# fail on FreeBSD, where the OpenGL libraries are not in the standard link
# directories. To sidestep this problem, we manually link the OpenGL libraries
# by their filenames.
find_package(Qt4 REQUIRED QtCore QtGui QtNetwork)
if(QT_QTOPENGL_INCLUDE_DIR)
  # Only use the QT_QTOPENGL_INCLUDE_DIR variable if it's not empty, as otherwise CMake will
  # abort with an error, saying that we are using a variable that's set to NOTFOUND. And we don't
  # want that, since OpenGL is optional.
  include_directories(SYSTEM ${QT_QTOPENGL_INCLUDE_DIR})

  # The code will use Qt's OpenGL interface, so the QtOpenGL library must also be available.
  if(NOT QT_QTOPENGL_LIBRARY)
    message(FATAL_ERROR "System in an inconsistent state: the Qt OpenGL headers are available, but not the link libraries.")
  endif(NOT QT_QTOPENGL_LIBRARY)

  add_definitions(-D CONFIG_QT_OPENGL)
endif(QT_QTOPENGL_INCLUDE_DIR)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_SRCS ${MOC_HDRS})
qt4_wrap_ui(UI_HDRS ${UIS})

# Uncomment/comment the following lines to enable/disable linking only against
# Qt's release libraries, which may be useful when creating a debug build of
# the program for distribution (since the release libraries are smaller).
message(WARNING "Unconditionally linking against Qt release libraries.")
remove_debug_libraries_from_list(QT_LIBRARIES)
list(REMOVE_ITEM QT_LIBRARIES "optimized")
if(QT_QTOPENGL_LIBRARY)
  remove_debug_libraries_from_list(QT_QTOPENGL_LIBRARY)
  list(REMOVE_ITEM QT_QTOPENGL_LIBRARY "optimized")
endif(QT_QTOPENGL_LIBRARY)

add_executable(${GAMBIT_BINARY_NAME} ${GAMBIT_SRCS} ${MOC_SRCS} ${UI_HDRS})

find_package(OpenGL) # OPTIONAL
if(OPENGL_INCLUDE_DIR)
  # Only use the OPENGL_INCLUDE_DIR variable if it's not empty, as otherwise CMake will abort
  # with an error, saying that we are using a variable that's set to NOTFOUND. And we don't want
  # that, since OpenGL is optional.
  include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
endif(OPENGL_INCLUDE_DIR)
target_link_libraries(${GAMBIT_BINARY_NAME} ${QT_LIBRARIES} ${QT_QTOPENGL_LIBRARY} ${OPENGL_LIBRARIES})

if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
  set_target_properties(${GAMBIT_BINARY_NAME} PROPERTIES LINK_FLAGS_RELEASE -mwindows)
endif()
