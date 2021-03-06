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

cmake_minimum_required(VERSION 2.8.12)

project(Gambit)

# Use 'gambitchess' as the name for the binary, as 'gambit' is already
# used by a different and unrelated project. This is done to prevent
# filename collisions, so the 'gambitchess' binary can be installed in
# the same directory on Unix platforms (probably '/usr/bin') as the
# directory containing 'gambit'.
set(GAMBIT_BINARY_NAME "gambitchess")

set(REVISION_NUMBER_HEADER "revision_number.h")

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
option(CONFIG_OFFICIAL_VERSION "whether this build is an official version" ON)
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

# "Developer mode" switch.
# If the file '_CMakeLists-DeveloperMode' exists, it is assumed that
# one is developing code, and so wants extra compilation flags to be
# turned on, like -Werror and -pedantic-errors, such that compilation
# warnings are treated as errors.
# Non-developers such as packagers don't have to do anything, as they
# most likely won't have the file '_CMakeLists-DeveloperMode', and so
# compilation flags like -Werror won't get in their way while
# compiling.
if(EXISTS _CMakeLists-DeveloperMode)
    set(DEVELOPER_MODE ON)
else()
    set(DEVELOPER_MODE OFF)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Let CMake automatically link the executable to
# the libqtmain.a library (with which Qt provides WinMain())
# when we link to the QtCore "IMPORTED target".
if(POLICY CMP0020)
    cmake_policy(SET CMP0020 NEW)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wstrict-prototypes -Wbad-function-cast -Wformat=2 -Wundef -pedantic -Wno-long-long")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wredundant-decls -Wformat=2 -Wundef -pedantic -Wno-long-long")

if(DEVELOPER_MODE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -pedantic-errors")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic-errors")
endif()

set(CMAKE_C_FLAGS_DEBUG
    "-g -O1 -D DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG
    "-g -O1 -D DEBUG")
set(CMAKE_C_FLAGS_RELEASE
    "-O2 -D NDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE
    "-O2 -D NDEBUG")
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

# target Gambit
#******************************************************************************

add_definitions(-D CONFIG_SETTINGS_BACKEND_USE_QT)

# Make the CMake option()s have their intended effect on the source
# code.
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
        src/sdk/chess_engine_mediator/sleep/sleep_w32.c
        src/sdk/NamedLock/NamedMutex_win32.cpp
        src/sdk/procspawn/procspawn_win32.c)
else(WIN32)
    set(GAMBIT_SRCS ${GAMBIT_SRCS}
        src/sdk/chess_engine_mediator/sleep/sleep_unix.c
        src/sdk/NamedLock/LockFile_unix.cpp
        src/sdk/procspawn/procspawn_unix.c)
endif(WIN32)

# It may be that the script failed but the revision number header file exists
# anyway. This is the case for source code releases of the program, which
# contain the header file, as it cannot be generated, as source code releases
# don't contain source code control system directories (such as '.svn' or
# '.git').
if(WIN32)
    execute_process(COMMAND "update_revision_number_header.bat" RESULT_VARIABLE script_result)
    if(NOT(EXISTS ${REVISION_NUMBER_HEADER}))
        message(FATAL_ERROR "'update_revision_number_header.bat' failed to create '${REVISION_NUMBER_HEADER}'")
    endif()
else(WIN32)
    execute_process(COMMAND "sh" "update_revision_number_header.sh" RESULT_VARIABLE script_result)
    if(NOT(EXISTS ${REVISION_NUMBER_HEADER}))
        message(FATAL_ERROR "'update_revision_number_header.sh' failed to create '${REVISION_NUMBER_HEADER}'")
    endif()
endif(WIN32)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL) # OPTIONAL

if(Qt5OpenGL_FOUND)
    set(QT5_OPENGL_TARGET_LINK_LIBRARY "Qt5::OpenGL")
    add_definitions(-D CONFIG_QT_OPENGL)
endif(Qt5OpenGL_FOUND)

# This helps qt5_wrap_cpp(), otherwise you can get
# "Error: Undefined interface" when using
# the Q_INTERFACES macro.
include_directories(${Qt5Widgets_INCLUDE_DIRS})

qt5_wrap_cpp(MOC_SRCS ${MOC_HDRS})
qt5_wrap_ui(UI_HDRS ${UIS})

find_package(OpenGL) # OPTIONAL
if(OPENGL_INCLUDE_DIR)
    # Only use the OPENGL_INCLUDE_DIR variable if it's not empty, as otherwise CMake will abort
    # with an error, saying that we are using a variable that's set to NOTFOUND. And we don't want
    # that, since OpenGL is optional.
    include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
endif(OPENGL_INCLUDE_DIR)

add_executable(${GAMBIT_BINARY_NAME} ${GAMBIT_SRCS} ${MOC_SRCS} ${UI_HDRS})
target_link_libraries(${GAMBIT_BINARY_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network ${QT5_OPENGL_TARGET_LINK_LIBRARY} ${OPENGL_LIBRARIES})

if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
    set_target_properties(${GAMBIT_BINARY_NAME} PROPERTIES LINK_FLAGS_RELEASE -mwindows)
endif()
