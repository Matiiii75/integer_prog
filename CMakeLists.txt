# L'outil CMake permet de générér automatiquement le fichier Makefile d'un projet.
cmake_minimum_required(VERSION 3.14)
project(projet_gopp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O2 -pg")
 
#------------------- CREATION DES VARIABLES POUR LA BIBLIOTHEQUE GUROBI -----------------------
message(STATUS "CMAKE_SYSTEM_NAME : ${CMAKE_SYSTEM_NAME}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux") # Pour Linux
    # CORRECTION ICI : Chemin mis à jour vers Gurobi 12.0.3
    set(GUROBI_ROOT "/opt/local/stow/gurobi1203/linux64") 
endif()    
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows") # Pour Windows
    set(GUROBI_ROOT "C:/gurobi952/win64") 
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # Pour MacOS
    set(GUROBI_ROOT "/Library/gurobi952/mac64") 
endif()
    
message(STATUS "GUROBI_ROOT : ${GUROBI_ROOT}")

# On créé une variable stockant le dossier où se situent les fichiers en-tête du solveur GUROBI
set(GUROBI_INCLUDE_DIR "${GUROBI_ROOT}/include")
message(STATUS "GUROBI_INCLUDE_DIR : ${GUROBI_INCLUDE_DIR}") 

# On créé des variables stockant l'emplacement des biblothèques/librairies du solveur GUROBI
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux") # Pour Linux
    set(GUROBI_LIBRARY_1 "${GUROBI_ROOT}/lib/libgurobi_c++.a")
    # CORRECTION ICI : libgurobi91.so remplacé par libgurobi120.so (pour version 12.0)
    set(GUROBI_LIBRARY_2 "${GUROBI_ROOT}/lib/libgurobi120.so")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows") # Pour Windows
    set(GUROBI_LIBRARY_1 "${GUROBI_ROOT}/lib/gurobi95.lib")
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(GUROBI_LIBRARY_2 "${GUROBI_ROOT}/lib/gurobi_c++mdd2019.lib")
    else()
        set(GUROBI_LIBRARY_2 "${GUROBI_ROOT}/lib/gurobi_c++md2019.lib")
    endif()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # Pour MacOS
    set(GUROBI_LIBRARY_1 "${GUROBI_ROOT}/lib/libgurobi_c++.a")
    set(GUROBI_LIBRARY_2 "${GUROBI_ROOT}/lib/libgurobi91.dylib")
endif()

set(GUROBI_LIBRARIES ${GUROBI_LIBRARY_1} ${GUROBI_LIBRARY_2})
message(STATUS "GUROBI_LIBRARIES : ${GUROBI_LIBRARIES}") 

#------------------- PARTIE PRINCIPALE -----------------------
# On indique où trouver les définitions des fonctions du solveur GUROBI
include_directories(${GUROBI_INCLUDE_DIR})
include_directories(include)

# Recherche tous les fichiers .cpp dans le dossier src (stocke tout cela dans la variable SRCS)
file(GLOB_RECURSE SRCS src/*.cpp)
message(STATUS "SRCS : ${SRCS}") 

# On indique que l'on veut un exécutable "projet_gopp" compilé à partir des fichiers stockés dans SRCS
add_executable(projet_ip ${SRCS})

# On indique où trouver les biblothèques du solveur GUROBI
target_link_libraries(projet_ip ${GUROBI_LIBRARIES})

# On ajoute un lien symbolique vers le dossier data
if (UNIX)  # Pour MacOS et Linux
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/ ${CMAKE_CURRENT_BINARY_DIR}/data)
else()  # Pour Windows
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/data" _dstDir)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/" _srcDir)
    execute_process(COMMAND cmd.exe /c mklink /J "${_dstDir}" "${_srcDir}")
endif()