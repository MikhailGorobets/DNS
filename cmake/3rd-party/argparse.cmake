include_directories(PUBLIC "${PROJECT_DIRECTORY}/3rd-party/argparse/include")
add_subdirectory(${PROJECT_DIRECTORY}/3rd-party/argparse)
set_target_properties(argparse PROPERTIES FOLDER "3rd-party")