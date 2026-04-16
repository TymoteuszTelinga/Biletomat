# CMakeList.txt: projekt CMake dla elementu Biletomat, dołącz źródło i zdefiniuj
# tutaj logikę specyficzną dla projektu.
#
cmake_minimum_required (VERSION 3.8)

# Włącz przeładowywanie kodu XAML na gorąco dla kompilatorów MSVC, jeśli jest to obsługiwane.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Biletomat")

find_package(spdlog REQUIRED)
find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(raylib REQUIRED)

# ── TCP Client ───────────────────────────────────────────────────────────────
add_executable (Biletomat "Client/Biletomat.cpp" "Client/TextField.h" "Client/TextField.cpp" "Client/CashRegister.h" "Client/CashRegister.cpp" "Common/StructDefiicions.h" "Client/Biletomat.h" "Client/main.cpp" "Client/BiletomatGUI.h" "Client/BiletomatGUI.cpp")

target_compile_definitions(Biletomat PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    NOGDI
    NOUSER
)

target_link_libraries(Biletomat spdlog::spdlog)
target_link_libraries(Biletomat httplib::httplib)
target_link_libraries(Biletomat nlohmann_json::nlohmann_json)
target_link_libraries(Biletomat raylib)

# ── TCP Server ───────────────────────────────────────────────────────────────
add_executable (server "Server/server.cpp" "Common/StructDefiicions.h" "Server/BroadcastHub.h" "Server/BroadcastHub.cpp" "Server/TimeoutManager.h" "Server/TimeoutManager.cpp" "Server/Server.h")
target_link_libraries(server spdlog::spdlog)
target_link_libraries(server httplib::httplib)
target_link_libraries(server nlohmann_json::nlohmann_json)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Biletomat PROPERTY CXX_STANDARD 20)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
endif()

# TODO: Dodaj testy i zainstaluj elementy docelowe w razie potrzeby.
